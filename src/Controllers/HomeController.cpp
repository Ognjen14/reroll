#include "Controllers/HomeController.h"

#include "Domain/CandidateScorer.h"
#include "Domain/MediaTypeFilter.h"

#include "Infrastructure/JsonStore.h"
#include "Infrastructure/TmdbMapper.h"

#include "RRLog.h"

#include <QDesktopServices>
#include <QRandomGenerator>
#include <QUrl>

#include <algorithm>
#include <utility>
#include <variant>

namespace Reroll::Controllers
{
namespace
{

constexpr quint32 RandomStartPageMaximum = 20;
constexpr Domain::CandidatePool::SizeType ReplenishmentThreshold = 5;

Infrastructure::TmdbGenreMatchMode toTmdbGenreMatchMode(Domain::GenreMatchMode mode)
{
    return mode == Domain::GenreMatchMode::And
        ? Infrastructure::TmdbGenreMatchMode::And
        : Infrastructure::TmdbGenreMatchMode::Or;
}

QString errorTextFor(const Infrastructure::TmdbError &error)
{
    return QString::fromStdString(error.userSafeContext());
}

}

HomeController::HomeController(Infrastructure::TmdbClient &tmdbClient,
                               Infrastructure::JsonStore &jsonStore,
                               Infrastructure::PosterUrlResolver &posterUrlResolver,
                               QObject *parent)
    : QObject(parent)
    , m_tmdbClient(tmdbClient)
    , m_jsonStore(jsonStore)
    , m_posterUrlResolver(posterUrlResolver)
    , m_appliedFilters(jsonStore.filters())
    , m_editableFilters(jsonStore.filters())
    , m_picker(static_cast<Domain::WeightedPicker::Seed>(
          QRandomGenerator::global()->generate()))
{
    connect(&m_posterUrlResolver,
            &Infrastructure::PosterUrlResolver::configurationLoaded,
            this,
            [this]() {
                if (hasSuggestion())
                {
                    emit suggestionChanged();
                }
            });

    connect(this, &HomeController::suggestionChanged,
            this, &HomeController::fetchStreamingProviders);

    fetchGenreLists();
}

void HomeController::setEligibilityFilter(EligibilityFilter filter)
{
    m_eligibilityFilter = std::move(filter);
}

HomeController::State HomeController::state() const noexcept
{
    return m_state;
}

QString HomeController::errorText() const noexcept
{
    return m_errorText;
}

bool HomeController::canReroll() const noexcept
{
    return m_state == State::Ready && !m_awaitingReplenishmentForPick;
}

bool HomeController::replenishing() const noexcept
{
    return m_moviePendingFetch || m_tvPendingFetch;
}

bool HomeController::trailerLoading() const noexcept
{
    return m_trailerLoading;
}

bool HomeController::hasSuggestion() const noexcept
{
    return m_session.currentSelection().has_value();
}

QString HomeController::title() const
{
    const auto &selection = m_session.currentSelection();
    return selection.has_value() ? QString::fromStdString(selection->title()) : QString();
}

int HomeController::releaseYear() const noexcept
{
    const auto &selection = m_session.currentSelection();
    return selection.has_value() ? selection->releaseYear() : 0;
}

bool HomeController::isTv() const noexcept
{
    const auto &selection = m_session.currentSelection();
    return selection.has_value() && selection->mediaType() == Domain::MediaType::Tv;
}

double HomeController::rating() const noexcept
{
    const auto &selection = m_session.currentSelection();
    return selection.has_value() ? selection->rating() : 0.0;
}

qint64 HomeController::voteCount() const noexcept
{
    const auto &selection = m_session.currentSelection();
    return selection.has_value() ? selection->voteCount() : 0;
}

QString HomeController::overview() const
{
    const auto &selection = m_session.currentSelection();
    return selection.has_value() ? QString::fromStdString(selection->overview()) : QString();
}

QString HomeController::posterUrl() const
{
    const auto &selection = m_session.currentSelection();
    if (!selection.has_value())
    {
        return {};
    }

    const QUrl url = m_posterUrlResolver.resolve(selection->posterPath(), 342);
    return url.isValid() ? url.toString() : QString();
}

qint64 HomeController::currentTmdbId() const noexcept
{
    const auto &selection = m_session.currentSelection();
    return selection.has_value() ? selection->tmdbId() : 0;
}

QString HomeController::currentPosterPath() const
{
    const auto &selection = m_session.currentSelection();
    return selection.has_value() ? QString::fromStdString(selection->posterPath()) : QString();
}

QVariantList HomeController::currentGenreIds() const
{
    QVariantList genreIds;
    const auto &selection = m_session.currentSelection();
    if (!selection.has_value())
    {
        return genreIds;
    }
    genreIds.reserve(static_cast<qsizetype>(selection->genreIds().size()));
    for (const auto genreId : selection->genreIds())
    {
        genreIds.push_back(genreId);
    }
    return genreIds;
}

QStringList HomeController::currentGenreNames() const
{
    const auto &selection = m_session.currentSelection();
    if (!selection.has_value())
    {
        return {};
    }
    return genreNamesFor(selection->mediaType(), selection->genreIds());
}

QVariantList HomeController::currentStreamingProviders() const noexcept
{
    return m_currentStreamingProviders;
}

int HomeController::rerollCount() const noexcept
{
    return static_cast<int>(m_session.rerollCount());
}

bool HomeController::recycled() const noexcept
{
    return m_recycled;
}

bool HomeController::hasUnappliedChanges() const noexcept
{
    return m_editableFilters != m_appliedFilters;
}

ViewModels::Models::GenreListModel *HomeController::genreModel() noexcept
{
    return &m_genreModel;
}

int HomeController::editableMediaType() const noexcept
{
    return static_cast<int>(m_editableFilters.mediaType());
}

int HomeController::editableMinimumYear() const noexcept
{
    return m_editableFilters.minimumYear().value_or(0);
}

int HomeController::editableMaximumYear() const noexcept
{
    return m_editableFilters.maximumYear().value_or(0);
}

double HomeController::editableMinimumRating() const noexcept
{
    return m_editableFilters.minimumRating();
}

int HomeController::editableGenreMatchMode() const noexcept
{
    return static_cast<int>(m_editableFilters.genreMatchMode());
}

bool HomeController::editableExcludeWatched() const noexcept
{
    return m_editableFilters.excludeWatched();
}

int HomeController::appliedMediaType() const noexcept
{
    return static_cast<int>(m_appliedFilters.mediaType());
}

int HomeController::appliedMinimumYear() const noexcept
{
    return m_appliedFilters.minimumYear().value_or(0);
}

int HomeController::appliedMaximumYear() const noexcept
{
    return m_appliedFilters.maximumYear().value_or(0);
}

double HomeController::appliedMinimumRating() const noexcept
{
    return m_appliedFilters.minimumRating();
}

int HomeController::appliedGenreCount() const noexcept
{
    return static_cast<int>(m_appliedFilters.genreIds().size());
}

bool HomeController::appliedExcludeWatched() const noexcept
{
    return m_appliedFilters.excludeWatched();
}

void HomeController::start()
{
    if (m_state != State::Idle)
    {
        return;
    }

    RR_LOG_I() << "HomeController starting first session";
    beginSessionLoad();
}

void HomeController::reroll()
{
    if (!canReroll())
    {
        RR_LOG_W() << "Reroll ignored, not in a rerollable state";
        return;
    }

    RR_LOG_D() << "Reroll requested" << "currentRerollCount" << m_session.rerollCount();
    resolvePickResult(pickAndSelect(true));
}

void HomeController::retry()
{
    if (m_state != State::NetworkError && m_state != State::RateLimited)
    {
        return;
    }

    RR_LOG_I() << "Retrying suggestion load after failure";
    m_state = State::Idle;
    beginSessionLoad();
}

bool HomeController::setEditableFilters(Domain::FilterCriteria updated)
{
    if (updated == m_editableFilters)
    {
        return false;
    }

    const bool previouslyHadUnappliedChanges = hasUnappliedChanges();
    const bool genresRelevant =
        updated.mediaType() != m_editableFilters.mediaType()
        || updated.genreIds() != m_editableFilters.genreIds();
    m_editableFilters = std::move(updated);
    if (genresRelevant)
    {
        refreshGenreModel();
    }
    emit editableFiltersChanged();
    if (hasUnappliedChanges() != previouslyHadUnappliedChanges)
    {
        emit hasUnappliedChangesChanged();
    }
    return true;
}

bool HomeController::setEditableMediaType(int mediaType)
{
    if (mediaType < 0 || mediaType > 2)
    {
        return false;
    }

    return setEditableFilters(Domain::FilterCriteria(
        static_cast<Domain::MediaTypeFilter>(mediaType),
        m_editableFilters.minimumYear(),
        m_editableFilters.maximumYear(),
        m_editableFilters.minimumRating(),
        m_editableFilters.genreIds(),
        m_editableFilters.genreMatchMode(),
        m_editableFilters.excludeWatched(),
        m_editableFilters.originalLanguage()));
}

bool HomeController::setEditableYearRange(int minimumYear, int maximumYear)
{
    return setEditableFilters(Domain::FilterCriteria(
        m_editableFilters.mediaType(),
        minimumYear > 0 ? std::optional<int>(minimumYear) : std::nullopt,
        maximumYear > 0 ? std::optional<int>(maximumYear) : std::nullopt,
        m_editableFilters.minimumRating(),
        m_editableFilters.genreIds(),
        m_editableFilters.genreMatchMode(),
        m_editableFilters.excludeWatched(),
        m_editableFilters.originalLanguage()));
}

bool HomeController::setEditableMinimumRating(double minimumRating)
{
    return setEditableFilters(Domain::FilterCriteria(
        m_editableFilters.mediaType(),
        m_editableFilters.minimumYear(),
        m_editableFilters.maximumYear(),
        minimumRating,
        m_editableFilters.genreIds(),
        m_editableFilters.genreMatchMode(),
        m_editableFilters.excludeWatched(),
        m_editableFilters.originalLanguage()));
}

bool HomeController::setEditableGenreMatchMode(int genreMatchMode)
{
    if (genreMatchMode < 0 || genreMatchMode > 1)
    {
        return false;
    }

    return setEditableFilters(Domain::FilterCriteria(
        m_editableFilters.mediaType(),
        m_editableFilters.minimumYear(),
        m_editableFilters.maximumYear(),
        m_editableFilters.minimumRating(),
        m_editableFilters.genreIds(),
        static_cast<Domain::GenreMatchMode>(genreMatchMode),
        m_editableFilters.excludeWatched(),
        m_editableFilters.originalLanguage()));
}

bool HomeController::setEditableExcludeWatched(bool excludeWatched)
{
    return setEditableFilters(Domain::FilterCriteria(
        m_editableFilters.mediaType(),
        m_editableFilters.minimumYear(),
        m_editableFilters.maximumYear(),
        m_editableFilters.minimumRating(),
        m_editableFilters.genreIds(),
        m_editableFilters.genreMatchMode(),
        excludeWatched,
        m_editableFilters.originalLanguage()));
}

bool HomeController::setGenreSelected(int genreId, bool selected)
{
    Domain::FilterCriteria::GenreIds genreIds = m_editableFilters.genreIds();
    const auto position = std::find(genreIds.begin(), genreIds.end(), genreId);
    if ((position != genreIds.end()) == selected)
    {
        return false;
    }

    if (selected)
    {
        genreIds.push_back(genreId);
    }
    else
    {
        genreIds.erase(position);
    }

    return setEditableFilters(Domain::FilterCriteria(
        m_editableFilters.mediaType(),
        m_editableFilters.minimumYear(),
        m_editableFilters.maximumYear(),
        m_editableFilters.minimumRating(),
        std::move(genreIds),
        m_editableFilters.genreMatchMode(),
        m_editableFilters.excludeWatched(),
        m_editableFilters.originalLanguage()));
}

bool HomeController::apply()
{
    if (m_editableFilters == m_appliedFilters)
    {
        return false;
    }

    applyFilters(m_editableFilters);
    return true;
}

bool HomeController::reset()
{
    return setEditableFilters(Domain::FilterCriteria{});
}

void HomeController::playTrailer()
{
    if (!hasSuggestion() || m_trailerLoading)
    {
        return;
    }

    const qint64 tmdbId = currentTmdbId();
    const bool requestedIsTv = isTv();
    const Infrastructure::TmdbMediaType mediaType = requestedIsTv
        ? Infrastructure::TmdbMediaType::Tv
        : Infrastructure::TmdbMediaType::Movie;

    RR_LOG_D() << "Trailer requested" << tmdbId << (requestedIsTv ? "tv" : "movie");

    m_trailerLoading = true;
    emit trailerLoadingChanged();

    m_tmdbClient.videos(
        mediaType,
        tmdbId,
        [this, tmdbId, requestedIsTv](Infrastructure::TmdbClient::VideosResult result) {
            m_trailerLoading = false;
            emit trailerLoadingChanged();

            if (currentTmdbId() != tmdbId || isTv() != requestedIsTv)
            {
                RR_LOG_D() << "Trailer result discarded, suggestion changed" << tmdbId;
                return;
            }

            if (auto *error = std::get_if<Infrastructure::TmdbError>(&result))
            {
                RR_LOG_W() << "Trailer lookup failed" << tmdbId
                           << "category" << static_cast<int>(error->category());
                return;
            }

            const auto &videos = std::get<Infrastructure::TmdbVideosResponseDto>(result);
            const auto isOfficialYoutubeTrailer =
                [](const Infrastructure::TmdbVideoDto &video) {
                    return video.site == "YouTube" && video.type == "Trailer"
                           && video.official;
                };
            const auto isYoutubeTrailer =
                [](const Infrastructure::TmdbVideoDto &video) {
                    return video.site == "YouTube" && video.type == "Trailer";
                };

            auto trailer = std::find_if(videos.results.cbegin(),
                                        videos.results.cend(),
                                        isOfficialYoutubeTrailer);
            if (trailer == videos.results.cend())
            {
                trailer = std::find_if(videos.results.cbegin(),
                                       videos.results.cend(),
                                       isYoutubeTrailer);
            }

            if (trailer == videos.results.cend())
            {
                RR_LOG_W() << "No YouTube trailer available" << tmdbId;
                return;
            }

            const QString url = QStringLiteral("https://www.youtube.com/watch?v=%1")
                .arg(QString::fromStdString(trailer->key));
            RR_LOG_I() << "Opening trailer" << tmdbId << url;
            QDesktopServices::openUrl(QUrl(url));
        });
}

void HomeController::fetchStreamingProviders()
{
    if (!hasSuggestion())
    {
        m_streamingProvidersLoaded = false;
        if (!m_currentStreamingProviders.isEmpty())
        {
            m_currentStreamingProviders.clear();
            emit streamingProvidersChanged();
        }
        return;
    }

    const qint64 tmdbId = currentTmdbId();
    const bool requestedIsTv = isTv();

    if (m_streamingProvidersLoaded
        && tmdbId == m_streamingProvidersTmdbId
        && requestedIsTv == m_streamingProvidersIsTv)
    {
        return;
    }

    m_streamingProvidersLoaded = true;
    m_streamingProvidersTmdbId = tmdbId;
    m_streamingProvidersIsTv = requestedIsTv;
    m_currentStreamingProviders.clear();
    emit streamingProvidersChanged();

    const Infrastructure::TmdbMediaType mediaType = requestedIsTv
        ? Infrastructure::TmdbMediaType::Tv
        : Infrastructure::TmdbMediaType::Movie;

    RR_LOG_D() << "Streaming providers requested" << tmdbId
               << (requestedIsTv ? "tv" : "movie");

    m_tmdbClient.watchProviders(
        mediaType,
        tmdbId,
        [this, tmdbId, requestedIsTv](
            Infrastructure::TmdbClient::WatchProvidersResult result) {
            if (currentTmdbId() != tmdbId || isTv() != requestedIsTv)
            {
                RR_LOG_D() << "Streaming providers result discarded, suggestion changed"
                           << tmdbId;
                return;
            }

            if (auto *error = std::get_if<Infrastructure::TmdbError>(&result))
            {
                RR_LOG_W() << "Streaming providers lookup failed" << tmdbId
                           << "category" << static_cast<int>(error->category());
                return;
            }

            const auto &providers =
                std::get<Infrastructure::TmdbWatchProvidersResponseDto>(result);
            QVariantList list;
            list.reserve(static_cast<qsizetype>(providers.providers.size()));
            for (const auto &provider : providers.providers)
            {
                QVariantMap entry;
                entry.insert(QStringLiteral("name"),
                             QString::fromStdString(provider.providerName));
                entry.insert(
                    QStringLiteral("logoUrl"),
                    m_posterUrlResolver.resolveUrl(
                        QString::fromStdString(provider.logoPath), 92));
                list.push_back(entry);
            }

            m_currentStreamingProviders = std::move(list);
            emit streamingProvidersChanged();
        });
}

void HomeController::applyFilters(Domain::FilterCriteria updated)
{
    if (updated == m_appliedFilters)
    {
        return;
    }

    const bool previouslyHadUnappliedChanges = hasUnappliedChanges();
    RR_LOG_I() << "Applied filters changed, starting a new session";
    m_appliedFilters = std::move(updated);
    persistFilters();
    emit filtersChanged();
    if (hasUnappliedChanges() != previouslyHadUnappliedChanges)
    {
        emit hasUnappliedChangesChanged();
    }

    m_state = State::Idle;
    beginSessionLoad();
}

void HomeController::persistFilters()
{
    m_jsonStore.setFilters(m_appliedFilters);
}

void HomeController::beginSessionLoad()
{
    if (!transitionTo(State::Loading))
    {
        RR_LOG_W() << "Could not start a session load from the current state";
        return;
    }

    ++m_generation;
    if (m_generation == 0)
    {
        ++m_generation;
    }
    const auto generation = m_generation;

    m_moviePool = {};
    m_tvPool = {};
    m_session.reset();
    m_recycled = false;
    m_awaitingReplenishmentForPick = false;
    m_initialLoadFailed = false;
    m_errorText.clear();
    emit sessionChanged();
    emit suggestionChanged();

    const auto mediaTypeFilterValue = m_appliedFilters.mediaType();
    m_movieActive = mediaTypeFilterValue == Domain::MediaTypeFilter::Movie
        || mediaTypeFilterValue == Domain::MediaTypeFilter::Both;
    m_tvActive = mediaTypeFilterValue == Domain::MediaTypeFilter::Tv
        || mediaTypeFilterValue == Domain::MediaTypeFilter::Both;

    const quint32 initialMoviePage = m_movieActive ? randomStartPage() : 0;
    const quint32 initialTvPage = m_tvActive ? randomStartPage() : 0;
    m_pendingInitialFetches = (m_movieActive ? 1 : 0) + (m_tvActive ? 1 : 0);

    RR_LOG_I() << "Session started" << generation
               << "movieActive" << m_movieActive << "startPage" << initialMoviePage
               << "tvActive" << m_tvActive << "startPage" << initialTvPage;

    if (m_movieActive)
    {
        fetchPage(Domain::MediaType::Movie, initialMoviePage, true);
    }
    if (m_tvActive)
    {
        fetchPage(Domain::MediaType::Tv, initialTvPage, true);
    }
}

quint32 HomeController::randomStartPage() const
{
    return QRandomGenerator::global()->bounded(1, static_cast<int>(RandomStartPageMaximum) + 1);
}

Domain::CandidatePool &HomeController::poolFor(Domain::MediaType mediaType) noexcept
{
    return mediaType == Domain::MediaType::Movie ? m_moviePool : m_tvPool;
}

const Domain::CandidatePool &HomeController::poolFor(
    Domain::MediaType mediaType) const noexcept
{
    return mediaType == Domain::MediaType::Movie ? m_moviePool : m_tvPool;
}

Infrastructure::TmdbDiscoverRequestDto HomeController::buildRequest(
    const Domain::FilterCriteria &filters,
    quint32 page)
{
    Infrastructure::TmdbDiscoverRequestDto request;
    request.page = page;
    request.minimumYear = filters.minimumYear();
    request.maximumYear = filters.maximumYear();
    request.minimumRating = filters.minimumRating();
    request.genreIds.assign(filters.genreIds().cbegin(), filters.genreIds().cend());
    request.genreMatchMode = toTmdbGenreMatchMode(filters.genreMatchMode());
    request.originalLanguage = filters.originalLanguage();
    return request;
}

void HomeController::fetchPage(Domain::MediaType mediaType, quint32 page, bool isInitial)
{
    const auto generation = m_generation;
    const Infrastructure::TmdbDiscoverRequestDto request = buildRequest(m_appliedFilters, page);

    if (mediaType == Domain::MediaType::Movie)
    {
        m_moviePendingFetch = true;
        m_tmdbClient.discoverMovie(
            request,
            [this, generation, page, isInitial](Infrastructure::TmdbClient::DiscoverResult result) {
                handlePageResult(generation, Domain::MediaType::Movie, page, isInitial, std::move(result));
            });
    }
    else
    {
        m_tvPendingFetch = true;
        m_tmdbClient.discoverTv(
            request,
            [this, generation, page, isInitial](Infrastructure::TmdbClient::DiscoverResult result) {
                handlePageResult(generation, Domain::MediaType::Tv, page, isInitial, std::move(result));
            });
    }
    emit replenishingChanged();
}

void HomeController::handlePageResult(std::uint64_t generation,
                                      Domain::MediaType mediaType,
                                      quint32 page,
                                      bool isInitial,
                                      Infrastructure::TmdbClient::DiscoverResult result)
{
    if (mediaType == Domain::MediaType::Movie)
    {
        m_moviePendingFetch = false;
    }
    else
    {
        m_tvPendingFetch = false;
    }
    emit replenishingChanged();

    if (generation != m_generation)
    {
        RR_LOG_D() << "Discarding stale discover response" << generation
                   << "current" << m_generation;
        return;
    }

    if (auto *error = std::get_if<Infrastructure::TmdbError>(&result))
    {
        RR_LOG_W() << "Discover page failed" << (mediaType == Domain::MediaType::Movie ? "movie" : "tv")
                   << page << "category" << static_cast<int>(error->category());

        if (isInitial)
        {
            m_initialLoadFailed = true;
            m_errorText = errorTextFor(*error);
            --m_pendingInitialFetches;
            finishInitialLoadIfReady(generation);
        }
        return;
    }

    Infrastructure::TmdbDiscoverPageDto &page_ = std::get<Infrastructure::TmdbDiscoverPageDto>(result);

    if (isInitial
        && page_.results.empty()
        && page_.totalPages > 0
        && page > page_.totalPages)
    {
        const auto clampedPage = static_cast<quint32>(
            QRandomGenerator::global()->bounded(1, static_cast<int>(page_.totalPages) + 1));
        RR_LOG_I() << "Random start page overshot totalPages, retrying"
                   << (mediaType == Domain::MediaType::Movie ? "movie" : "tv")
                   << "requested" << page << "totalPages" << page_.totalPages
                   << "retryPage" << clampedPage;
        fetchPage(mediaType, clampedPage, true);
        return;
    }

    Domain::CandidatePool::Collection candidates;
    candidates.reserve(page_.results.size());
    int skipped = 0;
    for (const Infrastructure::TmdbTitleResultDto &dto : page_.results)
    {
        auto mapped = Infrastructure::TmdbMapper::toCandidate(dto);
        if (auto *candidate = std::get_if<Domain::Candidate>(&mapped))
        {
            candidates.push_back(std::move(*candidate));
        }
        else
        {
            ++skipped;
        }
    }
    if (skipped > 0)
    {
        RR_LOG_W() << "Skipped unusable TMDB results" << skipped;
    }

    Domain::CandidatePool &pool = poolFor(mediaType);
    const auto insertedCount = pool.insertAll(std::move(candidates));
    static_cast<void>(pool.recordFetchedPage(page_.page, page_.totalPages));

    RR_LOG_I() << "Discover page loaded" << (mediaType == Domain::MediaType::Movie ? "movie" : "tv")
               << page_.page << "of" << page_.totalPages
               << "inserted" << insertedCount;

    if (isInitial)
    {
        --m_pendingInitialFetches;
        finishInitialLoadIfReady(generation);
        return;
    }

    if (m_awaitingReplenishmentForPick)
    {
        m_awaitingReplenishmentForPick = false;
        emit stateChanged();
        resolvePickResult(pickAndSelect(m_lastPickWasReroll));
        return;
    }

    requestReplenishmentIfNeeded();
}

void HomeController::finishInitialLoadIfReady(std::uint64_t generation)
{
    if (m_pendingInitialFetches > 0)
    {
        return;
    }

    if (m_initialLoadFailed)
    {
        static_cast<void>(transitionTo(State::NetworkError));
        return;
    }

    const bool anyCandidates = !m_moviePool.empty() || !m_tvPool.empty();
    if (!anyCandidates)
    {
        RR_LOG_I() << "No candidates matched the applied filters";
        static_cast<void>(transitionTo(State::Empty));
        emit suggestionChanged();
        return;
    }

    resolvePickResult(pickAndSelect(false));

    static_cast<void>(generation);
}

void HomeController::resolvePickResult(PickResult result)
{
    switch (result)
    {
    case PickResult::Selected:
        static_cast<void>(transitionTo(State::Ready));
        requestReplenishmentIfNeeded();
        break;
    case PickResult::Empty:
        static_cast<void>(transitionTo(State::Empty));
        break;
    case PickResult::Pending:
        break;
    }
}

bool HomeController::isEligible(const Domain::Candidate &candidate) const
{
    if (m_session.hasBeenShown(candidate.identity()))
    {
        return false;
    }

    return !m_eligibilityFilter || m_eligibilityFilter(candidate.identity());
}

HomeController::PickOutcome HomeController::pickEligibleCandidate()
{
    std::vector<const Domain::Candidate *> eligibleCandidates;
    Domain::WeightedPicker::Weights weights;

    const auto collect = [&](const Domain::CandidatePool &pool, bool active) {
        if (!active)
        {
            return;
        }
        for (const Domain::Candidate &candidate : pool.candidates())
        {
            if (isEligible(candidate))
            {
                eligibleCandidates.push_back(&candidate);
                weights.push_back(
                    Domain::CandidateScorer::bayesianQualityScore(candidate, m_scoringConfig));
            }
        }
    };
    collect(m_moviePool, m_movieActive);
    collect(m_tvPool, m_tvActive);

    if (eligibleCandidates.empty())
    {
        return {};
    }

    const auto selectedIndex = m_picker.pickIndex(weights);
    if (!selectedIndex.has_value())
    {
        return PickOutcome{nullptr, true};
    }

    return PickOutcome{eligibleCandidates[*selectedIndex], true};
}

bool HomeController::canRequestMorePages() const noexcept
{
    return (m_movieActive && m_moviePool.canRequestAnotherPage())
        || (m_tvActive && m_tvPool.canRequestAnotherPage());
}

HomeController::PickResult HomeController::pickAndSelect(bool isReroll)
{
    const PickOutcome outcome = pickEligibleCandidate();
    if (outcome.candidate != nullptr)
    {
        const bool selected = isReroll
            ? m_session.select(*outcome.candidate)
            : m_session.selectInitial(*outcome.candidate);
        if (selected)
        {
            RR_LOG_D() << "Suggestion selected" << QString::fromStdString(outcome.candidate->title())
                       << "reroll" << isReroll << "rerollCount" << m_session.rerollCount();
            emit suggestionChanged();
            emit sessionChanged();
            return PickResult::Selected;
        }
        return PickResult::Empty;
    }

    if (outcome.hadEligibleCandidate)
    {
        return PickResult::Empty;
    }

    if (canRequestMorePages())
    {
        RR_LOG_I() << "Pool exhausted for now, requesting more pages before picking";
        m_awaitingReplenishmentForPick = true;
        m_lastPickWasReroll = isReroll;
        emit stateChanged();
        requestReplenishmentIfNeeded();
        return PickResult::Pending;
    }

    const bool poolHasAnything = !m_moviePool.empty() || !m_tvPool.empty();
    if (poolHasAnything)
    {
        RR_LOG_I() << "All matching titles have been shown, recycling the pool";
        m_session.reset();
        m_recycled = true;
        emit recyclingStarted();
        emit sessionChanged();
        return pickAndSelect(isReroll);
    }

    return PickResult::Empty;
}

void HomeController::requestReplenishmentIfNeeded()
{
    if (m_state != State::Loading && m_state != State::Ready)
    {
        return;
    }

    Domain::CandidatePool::SizeType remaining = 0;
    const auto countRemaining = [&](const Domain::CandidatePool &pool, bool active) {
        if (!active)
        {
            return;
        }
        for (const Domain::Candidate &candidate : pool.candidates())
        {
            if (isEligible(candidate))
            {
                ++remaining;
            }
        }
    };
    countRemaining(m_moviePool, m_movieActive);
    countRemaining(m_tvPool, m_tvActive);

    if (remaining > ReplenishmentThreshold)
    {
        return;
    }

    if (m_movieActive && !m_moviePendingFetch)
    {
        const auto nextPage = m_moviePool.nextUnfetchedPage();
        if (nextPage.has_value())
        {
            fetchPage(Domain::MediaType::Movie, *nextPage, false);
        }
    }
    if (m_tvActive && !m_tvPendingFetch)
    {
        const auto nextPage = m_tvPool.nextUnfetchedPage();
        if (nextPage.has_value())
        {
            fetchPage(Domain::MediaType::Tv, *nextPage, false);
        }
    }
}

bool HomeController::isTransitionAllowed(State current, State next) noexcept
{
    if (current == next)
    {
        return true;
    }

    switch (current)
    {
    case State::Idle:
        return next == State::Loading;
    case State::Loading:
        return next == State::Ready
            || next == State::Empty
            || next == State::NetworkError
            || next == State::RateLimited;
    case State::Ready:
    case State::Empty:
    case State::NetworkError:
    case State::RateLimited:
        return next == State::Loading;
    }

    return false;
}

bool HomeController::transitionTo(State next) noexcept
{
    if (!isTransitionAllowed(m_state, next))
    {
        RR_LOG_W() << "Illegal state transition" << static_cast<int>(m_state)
                   << "->" << static_cast<int>(next);
        return false;
    }

    if (m_state != next)
    {
        RR_LOG_D() << "State transition" << static_cast<int>(m_state)
                   << "->" << static_cast<int>(next);
        m_state = next;
        emit stateChanged();
    }
    return true;
}

void HomeController::fetchGenreLists()
{
    m_tmdbClient.movieGenres(
        std::nullopt,
        [this](Infrastructure::TmdbClient::GenreListResult result) {
            auto *genres = std::get_if<Infrastructure::TmdbGenreListResponseDto>(&result);
            if (!genres)
            {
                RR_LOG_W() << "Movie genre list failed to load";
                return;
            }
            m_movieGenreEntries.clear();
            m_movieGenreEntries.reserve(genres->genres.size());
            for (const Infrastructure::TmdbGenreDto &genre : genres->genres)
            {
                m_movieGenreEntries.push_back(
                    {genre.id, QString::fromStdString(genre.name), false});
            }
            RR_LOG_I() << "Movie genre list loaded" << m_movieGenreEntries.size();
            refreshGenreModel();
        });

    m_tmdbClient.tvGenres(
        std::nullopt,
        [this](Infrastructure::TmdbClient::GenreListResult result) {
            auto *genres = std::get_if<Infrastructure::TmdbGenreListResponseDto>(&result);
            if (!genres)
            {
                RR_LOG_W() << "TV genre list failed to load";
                return;
            }
            m_tvGenreEntries.clear();
            m_tvGenreEntries.reserve(genres->genres.size());
            for (const Infrastructure::TmdbGenreDto &genre : genres->genres)
            {
                m_tvGenreEntries.push_back(
                    {genre.id, QString::fromStdString(genre.name), false});
            }
            RR_LOG_I() << "TV genre list loaded" << m_tvGenreEntries.size();
            refreshGenreModel();
        });
}

void HomeController::refreshGenreModel()
{
    ViewModels::Models::GenreListModel::Entries visibleGenres =
        m_editableFilters.mediaType() == Domain::MediaTypeFilter::Tv
            ? m_tvGenreEntries
            : m_movieGenreEntries;
    const Domain::FilterCriteria::GenreIds &selectedIds = m_editableFilters.genreIds();
    for (ViewModels::Models::GenreListModel::Entry &genre : visibleGenres)
    {
        genre.selected = std::find(selectedIds.cbegin(), selectedIds.cend(), genre.id)
            != selectedIds.cend();
    }
    m_genreModel.updateEntries(std::move(visibleGenres));
}

QStringList HomeController::genreNamesFor(
    Domain::MediaType mediaType,
    const std::vector<std::int32_t> &genreIds) const
{
    const ViewModels::Models::GenreListModel::Entries &entries =
        mediaType == Domain::MediaType::Tv ? m_tvGenreEntries : m_movieGenreEntries;

    QStringList names;
    for (const std::int32_t genreId : genreIds)
    {
        const auto found = std::find_if(
            entries.cbegin(),
            entries.cend(),
            [genreId](const ViewModels::Models::GenreListModel::Entry &entry) {
                return entry.id == genreId;
            });
        if (found != entries.cend())
        {
            names.push_back(found->name);
        }
    }
    return names;
}

}
