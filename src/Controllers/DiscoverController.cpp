#include "Controllers/DiscoverController.h"

#include "Domain/Candidate.h"
#include "Domain/TitleSnapshot.h"

#include "Infrastructure/PosterUrlResolver.h"
#include "Infrastructure/TmdbMapper.h"

#include "RRLog.h"

#include <QDesktopServices>
#include <QUrl>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

namespace Reroll::Controllers
{
namespace
{

constexpr int SearchDebounceIntervalMs = 400;

ViewModels::Models::TitleListModel::Entries toEntries(
    const std::vector<Infrastructure::TmdbTitleResultDto> &results)
{
    ViewModels::Models::TitleListModel::Entries entries;
    entries.reserve(results.size());
    for (const Infrastructure::TmdbTitleResultDto &dto : results)
    {
        auto mapped = Infrastructure::TmdbMapper::toCandidate(dto);
        if (auto *candidate = std::get_if<Domain::Candidate>(&mapped))
        {
            entries.emplace_back(
                Domain::TitleSnapshot::fromCandidate(*candidate), false, false, false);
        }
    }
    return entries;
}

}

DiscoverController::DiscoverController(Infrastructure::TmdbClient &tmdbClient,
                                       Infrastructure::PosterUrlResolver &posterUrlResolver,
                                       QObject *parent)
    : QObject(parent)
    , m_tmdbClient(tmdbClient)
    , m_posterUrlResolver(posterUrlResolver)
{
    connect(&m_posterUrlResolver,
            &Infrastructure::PosterUrlResolver::configurationLoaded,
            this,
            [this]() {
                const auto refresh = [](ViewModels::Models::TitleListModel &model) {
                    if (model.rowCount() > 0)
                    {
                        emit model.dataChanged(
                            model.index(0, 0),
                            model.index(model.rowCount() - 1, 0),
                            {ViewModels::Models::TitleListModel::PosterPathRole});
                    }
                };
                refresh(m_trendingMovies);
                refresh(m_trendingTv);
                refresh(m_popularMovies);
                refresh(m_popularTv);
            });

    m_searchDebounceTimer.setSingleShot(true);
    m_searchDebounceTimer.setInterval(SearchDebounceIntervalMs);
    connect(&m_searchDebounceTimer, &QTimer::timeout,
           this, &DiscoverController::performSearch);
}

ViewModels::Models::TitleListModel *DiscoverController::trendingMovies() noexcept
{
    return &m_trendingMovies;
}

ViewModels::Models::TitleListModel *DiscoverController::trendingTv() noexcept
{
    return &m_trendingTv;
}

ViewModels::Models::TitleListModel *DiscoverController::popularMovies() noexcept
{
    return &m_popularMovies;
}

ViewModels::Models::TitleListModel *DiscoverController::popularTv() noexcept
{
    return &m_popularTv;
}

ViewModels::Models::GenreSectionListModel *DiscoverController::genreSections() noexcept
{
    return &m_genreSections;
}

ViewModels::Models::GenreSectionListModel *DiscoverController::tvGenreSections() noexcept
{
    return &m_tvGenreSections;
}

bool DiscoverController::loading() const noexcept
{
    return m_pendingRequestCount > 0;
}

bool DiscoverController::loadFailed() const noexcept
{
    return m_loadFailed;
}

QString DiscoverController::searchQuery() const noexcept
{
    return m_searchQuery;
}

void DiscoverController::setSearchQuery(const QString &query)
{
    if (m_searchQuery == query)
    {
        return;
    }

    m_searchQuery = query;
    emit searchQueryChanged();

    if (m_searchQuery.trimmed().isEmpty())
    {
        m_searchDebounceTimer.stop();
        performSearch();
        return;
    }

    m_searchDebounceTimer.start();
}

int DiscoverController::searchScope() const noexcept
{
    return m_searchScope;
}

void DiscoverController::setSearchScope(int scope)
{
    if (m_searchScope == scope)
    {
        return;
    }

    m_searchScope = scope;
    emit searchScopeChanged();

    if (!m_searchQuery.trimmed().isEmpty())
    {
        m_searchDebounceTimer.stop();
        performSearch();
    }
}

ViewModels::Models::TitleListModel *DiscoverController::searchResults() noexcept
{
    return &m_searchResults;
}

bool DiscoverController::searching() const noexcept
{
    return m_searching;
}

bool DiscoverController::loadingMore() const noexcept
{
    return m_loadingMoreCount > 0;
}

bool DiscoverController::titleDetailsLoading() const noexcept
{
    return m_titleDetailsLoading;
}

qint64 DiscoverController::titleDetailsTmdbId() const noexcept
{
    return m_titleDetailsTmdbId;
}

QString DiscoverController::titleDetailsOverview() const noexcept
{
    return m_titleDetailsOverview;
}

QStringList DiscoverController::titleDetailsGenreNames() const noexcept
{
    return m_titleDetailsGenreNames;
}

QVariantList DiscoverController::titleDetailsStreamingProviders() const noexcept
{
    return m_titleDetailsStreamingProviders;
}

bool DiscoverController::titleDetailsTrailerLoading() const noexcept
{
    return m_titleDetailsTrailerLoading;
}

void DiscoverController::start()
{
    if (m_started)
    {
        return;
    }
    m_started = true;
    m_loadFailed = false;
    m_initialSectionsPending = 6;
    m_initialSectionsSucceeded = 0;
    emit loadFailedChanged();

    RR_LOG_I() << "DiscoverController loading trending, popular, and genre sections";

    fetchInto(m_trendingMovies, "trending movies",
             [this](Infrastructure::TmdbPageNumber page,
                    Infrastructure::TmdbClient::DiscoverCompletionHandler handler) {
        return m_tmdbClient.trendingMovie(page, std::move(handler));
    }, &m_trendingMoviesPagination);
    fetchInto(m_trendingTv, "trending tv",
             [this](Infrastructure::TmdbPageNumber page,
                    Infrastructure::TmdbClient::DiscoverCompletionHandler handler) {
        return m_tmdbClient.trendingTv(page, std::move(handler));
    });
    fetchInto(m_popularMovies, "popular movies",
             [this](Infrastructure::TmdbPageNumber page,
                    Infrastructure::TmdbClient::DiscoverCompletionHandler handler) {
        return m_tmdbClient.popularMovie(page, std::move(handler));
    });
    fetchInto(m_popularTv, "popular tv",
             [this](Infrastructure::TmdbPageNumber page,
                    Infrastructure::TmdbClient::DiscoverCompletionHandler handler) {
        return m_tmdbClient.popularTv(page, std::move(handler));
    });

    fetchGenreSections(false);
    fetchGenreSections(true);
}

void DiscoverController::fetchInto(ViewModels::Models::TitleListModel &target,
                                   const char *label,
                                   RequestFn requestFn,
                                   PaginationState *paginationState)
{
    beginRequest();
    static_cast<void>(requestFn(
        1,
        [this, &target, label, paginationState](
            Infrastructure::TmdbClient::DiscoverResult result) {
            endRequest();
            if (auto *page = std::get_if<Infrastructure::TmdbDiscoverPageDto>(&result))
            {
                RR_LOG_I() << "Discover section loaded" << label
                           << page->results.size() << "results";
                target.setEntries(toEntries(page->results));
                if (paginationState)
                {
                    paginationState->nextPage = page->page + 1;
                    paginationState->totalPages = page->totalPages;
                }
                trackInitialSectionResult(true);
                return;
            }

            RR_LOG_W() << "Discover section failed" << label;
            trackInitialSectionResult(false);
        }));
}

void DiscoverController::fetchGenreSections(bool isTv)
{
    beginRequest();
    auto handler = [this, isTv](Infrastructure::TmdbClient::GenreListResult result) {
        endRequest();
        auto *genres = std::get_if<Infrastructure::TmdbGenreListResponseDto>(&result);
        if (!genres)
        {
            RR_LOG_W() << "Discover genre list failed" << (isTv ? "tv" : "movie");
            trackInitialSectionResult(false);
            return;
        }
        trackInitialSectionResult(true);

        std::vector<std::pair<std::int32_t, QString>> sections;
        sections.reserve(genres->genres.size());
        for (const Infrastructure::TmdbGenreDto &genre : genres->genres)
        {
            sections.emplace_back(genre.id, QString::fromStdString(genre.name));
        }
        RR_LOG_I() << "Discover genre sections loaded" << (isTv ? "tv" : "movie")
                   << sections.size() << "genres";

        ViewModels::Models::GenreSectionListModel &sectionsModel =
            isTv ? m_tvGenreSections : m_genreSections;
        QHash<std::int32_t, PaginationState> &pagination =
            isTv ? m_tvGenrePagination : m_genrePagination;
        QHash<std::int32_t, ViewModels::Models::TitleListModel *> &titleModels =
            isTv ? m_tvGenreTitleModels : m_genreTitleModels;

        sectionsModel.setGenres(sections);

        for (int row = 0; row < static_cast<int>(sections.size()); ++row)
        {
            const std::int32_t genreId = sections[static_cast<std::size_t>(row)].first;
            auto *rowModel = sectionsModel.titleModelForRow(row);
            if (rowModel)
            {
                titleModels.insert(genreId, rowModel);
                pagination.insert(genreId, PaginationState{});
                fetchGenreRow(isTv, genreId, row, *rowModel);
            }
        }
    };

    if (isTv)
    {
        static_cast<void>(m_tmdbClient.tvGenres(std::nullopt, std::move(handler)));
    }
    else
    {
        static_cast<void>(m_tmdbClient.movieGenres(std::nullopt, std::move(handler)));
    }
}

void DiscoverController::fetchGenreRow(bool isTv,
                                       std::int32_t genreId,
                                       int row,
                                       ViewModels::Models::TitleListModel &target)
{
    beginRequest();
    Infrastructure::TmdbDiscoverRequestDto request;
    request.page = 1;
    request.genreIds = {genreId};

    auto handler = [this, &target, isTv, genreId, row](
                       Infrastructure::TmdbClient::DiscoverResult result) {
        endRequest();
        if (auto *page = std::get_if<Infrastructure::TmdbDiscoverPageDto>(&result))
        {
            RR_LOG_D() << "Discover genre row loaded" << (isTv ? "tv" : "movie")
                      << genreId << page->results.size() << "results";
            target.setEntries(toEntries(page->results));

            ViewModels::Models::GenreSectionListModel &sectionsModel =
                isTv ? m_tvGenreSections : m_genreSections;
            sectionsModel.setTotalResults(row, static_cast<qint64>(page->totalResults));

            QHash<std::int32_t, PaginationState> &pagination =
                isTv ? m_tvGenrePagination : m_genrePagination;
            const auto stateIt = pagination.find(genreId);
            if (stateIt != pagination.end())
            {
                stateIt->nextPage = page->page + 1;
                stateIt->totalPages = page->totalPages;
            }
            return;
        }

        RR_LOG_W() << "Discover genre row failed" << (isTv ? "tv" : "movie") << genreId;
    };

    if (isTv)
    {
        static_cast<void>(m_tmdbClient.discoverTv(request, std::move(handler)));
    }
    else
    {
        static_cast<void>(m_tmdbClient.discoverMovie(request, std::move(handler)));
    }
}

void DiscoverController::loadMoreGeneric(ViewModels::Models::TitleListModel &target,
                                         PaginationState &state,
                                         RequestFn requestFn,
                                         const char *label)
{
    if (state.loadingMore || state.nextPage > state.totalPages)
    {
        return;
    }

    state.loadingMore = true;
    beginLoadingMore();
    const Infrastructure::TmdbPageNumber page = state.nextPage;
    RR_LOG_D() << "Discover load more requested" << label << "page" << page;

    static_cast<void>(requestFn(
        page,
        [this, &target, &state, label](Infrastructure::TmdbClient::DiscoverResult result) {
            state.loadingMore = false;
            endLoadingMore();
            if (auto *pageDto = std::get_if<Infrastructure::TmdbDiscoverPageDto>(&result))
            {
                RR_LOG_D() << "Discover load more finished" << label
                          << pageDto->results.size() << "results" << pageDto->page
                          << "of" << pageDto->totalPages;
                target.appendEntries(toEntries(pageDto->results));
                state.nextPage = pageDto->page + 1;
                state.totalPages = pageDto->totalPages;
                return;
            }

            RR_LOG_W() << "Discover load more failed" << label;
        }));
}

void DiscoverController::loadMoreTrendingMovies()
{
    loadMoreGeneric(
        m_trendingMovies,
        m_trendingMoviesPagination,
        [this](Infrastructure::TmdbPageNumber page,
              Infrastructure::TmdbClient::DiscoverCompletionHandler handler) {
            return m_tmdbClient.trendingMovie(page, std::move(handler));
        },
        "trendingMovies");
}

void DiscoverController::loadMoreForGenre(int genreId)
{
    loadMoreForGenreImpl(false, genreId);
}

void DiscoverController::loadMoreForTvGenre(int genreId)
{
    loadMoreForGenreImpl(true, genreId);
}

void DiscoverController::loadMoreForGenreImpl(bool isTv, int genreId)
{
    QHash<std::int32_t, PaginationState> &pagination =
        isTv ? m_tvGenrePagination : m_genrePagination;
    QHash<std::int32_t, ViewModels::Models::TitleListModel *> &titleModels =
        isTv ? m_tvGenreTitleModels : m_genreTitleModels;

    const auto stateIt = pagination.find(genreId);
    const auto modelIt = titleModels.find(genreId);
    if (stateIt == pagination.end() || modelIt == titleModels.end())
    {
        return;
    }

    loadMoreGeneric(
        *modelIt.value(),
        stateIt.value(),
        [this, isTv, genreId](Infrastructure::TmdbPageNumber page,
                              Infrastructure::TmdbClient::DiscoverCompletionHandler handler) {
            Infrastructure::TmdbDiscoverRequestDto request;
            request.page = page;
            request.genreIds = {genreId};
            return isTv
                ? m_tmdbClient.discoverTv(request, std::move(handler))
                : m_tmdbClient.discoverMovie(request, std::move(handler));
        },
        isTv ? "tvGenre" : "genre");
}

void DiscoverController::retryInitialLoad()
{
    if (!m_loadFailed)
    {
        return;
    }

    RR_LOG_I() << "Discover retrying initial load after a total failure";
    m_started = false;
    start();
}

void DiscoverController::trackInitialSectionResult(bool succeeded)
{
    if (succeeded)
    {
        ++m_initialSectionsSucceeded;
    }
    --m_initialSectionsPending;

    if (m_initialSectionsPending <= 0 && m_initialSectionsSucceeded == 0 && !m_loadFailed)
    {
        RR_LOG_W() << "Discover initial load failed across every section";
        m_loadFailed = true;
        emit loadFailedChanged();
    }
}

void DiscoverController::beginRequest()
{
    ++m_pendingRequestCount;
    if (m_pendingRequestCount == 1)
    {
        emit loadingChanged();
    }
}

void DiscoverController::endRequest()
{
    --m_pendingRequestCount;
    if (m_pendingRequestCount == 0)
    {
        emit loadingChanged();
    }
}

void DiscoverController::beginLoadingMore()
{
    ++m_loadingMoreCount;
    if (m_loadingMoreCount == 1)
    {
        emit loadingMoreChanged();
    }
}

void DiscoverController::endLoadingMore()
{
    --m_loadingMoreCount;
    if (m_loadingMoreCount == 0)
    {
        emit loadingMoreChanged();
    }
}

void DiscoverController::setSearching(bool searching)
{
    if (m_searching == searching)
    {
        return;
    }
    m_searching = searching;
    emit searchingChanged();
}

void DiscoverController::performSearch()
{
    ++m_searchGeneration;
    const std::uint64_t generation = m_searchGeneration;
    const QString query = m_searchQuery.trimmed();

    if (query.isEmpty())
    {
        RR_LOG_D() << "Discover search cleared";
        m_searchResults.setEntries({});
        setSearching(false);
        return;
    }

    RR_LOG_I() << "Discover search requested" << query << "scope" << m_searchScope;

    m_searchMovieBuffer.clear();
    m_searchTvBuffer.clear();
    m_searchMoviePending = m_searchScope == ScopeAll || m_searchScope == ScopeMovie;
    m_searchTvPending = m_searchScope == ScopeAll || m_searchScope == ScopeTv;
    m_searchMovieNextPage = 2;
    m_searchMovieTotalPages = 1;
    m_searchTvNextPage = 2;
    m_searchTvTotalPages = 1;
    m_searchLoadingMore = false;
    setSearching(true);

    if (m_searchMoviePending)
    {
        Infrastructure::TmdbSearchRequestDto request;
        request.query = query.toStdString();
        request.page = 1;
        static_cast<void>(m_tmdbClient.searchMovie(
            request,
            [this, generation](Infrastructure::TmdbClient::DiscoverResult result) {
                handleSearchResult(generation, true, std::move(result));
            }));
    }

    if (m_searchTvPending)
    {
        Infrastructure::TmdbSearchRequestDto request;
        request.query = query.toStdString();
        request.page = 1;
        static_cast<void>(m_tmdbClient.searchTv(
            request,
            [this, generation](Infrastructure::TmdbClient::DiscoverResult result) {
                handleSearchResult(generation, false, std::move(result));
            }));
    }
}

void DiscoverController::handleSearchResult(
    std::uint64_t generation,
    bool isMovie,
    Infrastructure::TmdbClient::DiscoverResult result)
{
    if (generation != m_searchGeneration)
    {
        RR_LOG_D() << "Discover search result discarded, stale generation";
        return;
    }

    if (isMovie)
    {
        m_searchMoviePending = false;
    }
    else
    {
        m_searchTvPending = false;
    }

    if (auto *page = std::get_if<Infrastructure::TmdbDiscoverPageDto>(&result))
    {
        if (isMovie)
        {
            m_searchMovieNextPage = page->page + 1;
            m_searchMovieTotalPages = page->totalPages;
        }
        else
        {
            m_searchTvNextPage = page->page + 1;
            m_searchTvTotalPages = page->totalPages;
        }
        (isMovie ? m_searchMovieBuffer : m_searchTvBuffer) = std::move(page->results);
    }
    else
    {
        RR_LOG_W() << "Discover search request failed" << isMovie;
    }

    if (m_searchMoviePending || m_searchTvPending)
    {
        return;
    }

    ViewModels::Models::TitleListModel::Entries entries = toEntries(m_searchMovieBuffer);
    ViewModels::Models::TitleListModel::Entries tvEntries = toEntries(m_searchTvBuffer);
    entries.insert(entries.end(),
                   std::make_move_iterator(tvEntries.begin()),
                   std::make_move_iterator(tvEntries.end()));
    RR_LOG_I() << "Discover search finished" << entries.size() << "results";
    m_searchResults.setEntries(std::move(entries));
    setSearching(false);
}

void DiscoverController::loadMoreSearchResults()
{
    if (m_searchLoadingMore)
    {
        return;
    }

    const QString query = m_searchQuery.trimmed();
    if (query.isEmpty())
    {
        return;
    }

    const bool wantMovie = (m_searchScope == ScopeAll || m_searchScope == ScopeMovie)
        && m_searchMovieNextPage <= m_searchMovieTotalPages;
    const bool wantTv = (m_searchScope == ScopeAll || m_searchScope == ScopeTv)
        && m_searchTvNextPage <= m_searchTvTotalPages;

    if (!wantMovie && !wantTv)
    {
        return;
    }

    RR_LOG_D() << "Discover search load more requested" << query
              << "movie" << wantMovie << "tv" << wantTv;

    m_searchLoadingMore = true;
    beginLoadingMore();
    const std::uint64_t generation = m_searchGeneration;
    auto remaining = std::make_shared<int>((wantMovie ? 1 : 0) + (wantTv ? 1 : 0));
    const auto finishOne = [this, remaining]() {
        --(*remaining);
        if (*remaining <= 0)
        {
            m_searchLoadingMore = false;
            endLoadingMore();
        }
    };

    if (wantMovie)
    {
        Infrastructure::TmdbSearchRequestDto request;
        request.query = query.toStdString();
        request.page = m_searchMovieNextPage;
        static_cast<void>(m_tmdbClient.searchMovie(
            request,
            [this, generation, finishOne](Infrastructure::TmdbClient::DiscoverResult result) {
                if (generation == m_searchGeneration)
                {
                    if (auto *page = std::get_if<Infrastructure::TmdbDiscoverPageDto>(&result))
                    {
                        m_searchResults.appendEntries(toEntries(page->results));
                        m_searchMovieNextPage = page->page + 1;
                        m_searchMovieTotalPages = page->totalPages;
                    }
                    else
                    {
                        RR_LOG_W() << "Discover search load more failed movie";
                    }
                }
                finishOne();
            }));
    }

    if (wantTv)
    {
        Infrastructure::TmdbSearchRequestDto request;
        request.query = query.toStdString();
        request.page = m_searchTvNextPage;
        static_cast<void>(m_tmdbClient.searchTv(
            request,
            [this, generation, finishOne](Infrastructure::TmdbClient::DiscoverResult result) {
                if (generation == m_searchGeneration)
                {
                    if (auto *page = std::get_if<Infrastructure::TmdbDiscoverPageDto>(&result))
                    {
                        m_searchResults.appendEntries(toEntries(page->results));
                        m_searchTvNextPage = page->page + 1;
                        m_searchTvTotalPages = page->totalPages;
                    }
                    else
                    {
                        RR_LOG_W() << "Discover search load more failed tv";
                    }
                }
                finishOne();
            }));
    }
}

void DiscoverController::loadTitleDetails(qint64 tmdbId, int mediaType)
{
    ++m_titleDetailsGeneration;
    const std::uint64_t generation = m_titleDetailsGeneration;

    RR_LOG_D() << "Title details requested" << tmdbId
              << (mediaType == 1 ? "tv" : "movie");

    m_titleDetailsLoading = true;
    m_titleDetailsTmdbId = tmdbId;
    m_titleDetailsMediaType = mediaType;
    m_titleDetailsOverview.clear();
    m_titleDetailsGenreNames.clear();
    m_titleDetailsStreamingProviders.clear();
    emit titleDetailsChanged();

    const Infrastructure::TmdbMediaType tmdbMediaType = mediaType == 1
        ? Infrastructure::TmdbMediaType::Tv
        : Infrastructure::TmdbMediaType::Movie;

    static_cast<void>(m_tmdbClient.titleDetails(
        tmdbMediaType,
        tmdbId,
        [this, generation](Infrastructure::TmdbClient::TitleDetailsResult result) {
            if (generation != m_titleDetailsGeneration)
            {
                RR_LOG_D() << "Title details result discarded, drawer moved on";
                return;
            }

            m_titleDetailsLoading = false;

            if (auto *error = std::get_if<Infrastructure::TmdbError>(&result))
            {
                RR_LOG_W() << "Title details lookup failed"
                          << "category" << static_cast<int>(error->category());
                emit titleDetailsChanged();
                return;
            }

            const auto &details = std::get<Infrastructure::TmdbTitleDetailsDto>(result);
            m_titleDetailsOverview = QString::fromStdString(details.overview);
            m_titleDetailsGenreNames.clear();
            m_titleDetailsGenreNames.reserve(
                static_cast<qsizetype>(details.genreNames.size()));
            for (const auto &genreName : details.genreNames)
            {
                m_titleDetailsGenreNames.push_back(QString::fromStdString(genreName));
            }
            emit titleDetailsChanged();
        }));

    static_cast<void>(m_tmdbClient.watchProviders(
        tmdbMediaType,
        tmdbId,
        [this, generation](Infrastructure::TmdbClient::WatchProvidersResult result) {
            if (generation != m_titleDetailsGeneration)
            {
                RR_LOG_D() << "Title details streaming providers discarded, drawer moved on";
                return;
            }

            if (auto *error = std::get_if<Infrastructure::TmdbError>(&result))
            {
                RR_LOG_W() << "Title details streaming providers lookup failed"
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

            m_titleDetailsStreamingProviders = std::move(list);
            emit titleDetailsChanged();
        }));
}

void DiscoverController::playTitleDetailsTrailer()
{
    if (m_titleDetailsTmdbId == 0 || m_titleDetailsTrailerLoading)
    {
        return;
    }

    const qint64 tmdbId = m_titleDetailsTmdbId;
    const int mediaType = m_titleDetailsMediaType;
    const Infrastructure::TmdbMediaType tmdbMediaType = mediaType == 1
        ? Infrastructure::TmdbMediaType::Tv
        : Infrastructure::TmdbMediaType::Movie;

    RR_LOG_D() << "Title details trailer requested" << tmdbId
              << (mediaType == 1 ? "tv" : "movie");

    m_titleDetailsTrailerLoading = true;
    emit titleDetailsTrailerLoadingChanged();

    static_cast<void>(m_tmdbClient.videos(
        tmdbMediaType,
        tmdbId,
        [this, tmdbId](Infrastructure::TmdbClient::VideosResult result) {
            m_titleDetailsTrailerLoading = false;
            emit titleDetailsTrailerLoadingChanged();

            if (m_titleDetailsTmdbId != tmdbId)
            {
                RR_LOG_D() << "Title details trailer result discarded, drawer moved on"
                          << tmdbId;
                return;
            }

            if (auto *error = std::get_if<Infrastructure::TmdbError>(&result))
            {
                RR_LOG_W() << "Title details trailer lookup failed" << tmdbId
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
        }));
}

}
