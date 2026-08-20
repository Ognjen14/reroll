#pragma once

#include "Domain/Candidate.h"
#include "Domain/CandidateIdentity.h"
#include "Domain/CandidatePool.h"
#include "Domain/FilterCriteria.h"
#include "Domain/ScoringConfig.h"
#include "Domain/SuggestionSession.h"
#include "Domain/WeightedPicker.h"

#include "Infrastructure/PosterUrlResolver.h"
#include "Infrastructure/TmdbClient.h"
#include "Infrastructure/TmdbDtos.h"

#include "ViewModels/Models/GenreListModel.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <cstdint>
#include <functional>
#include <optional>

namespace Reroll::Infrastructure
{
class JsonStore;
}

namespace Reroll::Controllers
{

class HomeController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(QString errorText READ errorText NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool canReroll READ canReroll NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool replenishing READ replenishing NOTIFY replenishingChanged FINAL)
    Q_PROPERTY(bool trailerLoading READ trailerLoading NOTIFY trailerLoadingChanged FINAL)

    Q_PROPERTY(bool hasSuggestion READ hasSuggestion NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(QString title READ title NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(int releaseYear READ releaseYear NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(bool isTv READ isTv NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(double rating READ rating NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(qint64 voteCount READ voteCount NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(QString overview READ overview NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(QString posterUrl READ posterUrl NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(qint64 currentTmdbId READ currentTmdbId NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(QString currentPosterPath READ currentPosterPath NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(QVariantList currentGenreIds READ currentGenreIds NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(QStringList currentGenreNames READ currentGenreNames NOTIFY suggestionChanged FINAL)
    Q_PROPERTY(QVariantList currentStreamingProviders READ currentStreamingProviders
                   NOTIFY streamingProvidersChanged FINAL)

    Q_PROPERTY(int rerollCount READ rerollCount NOTIFY sessionChanged FINAL)
    Q_PROPERTY(bool recycled READ recycled NOTIFY sessionChanged FINAL)

    Q_PROPERTY(bool hasUnappliedChanges READ hasUnappliedChanges
                   NOTIFY hasUnappliedChangesChanged FINAL)
    Q_PROPERTY(Reroll::ViewModels::Models::GenreListModel *genreModel
                   READ genreModel CONSTANT FINAL)

    Q_PROPERTY(int editableMediaType READ editableMediaType
                   NOTIFY editableFiltersChanged FINAL)
    Q_PROPERTY(int editableMinimumYear READ editableMinimumYear
                   NOTIFY editableFiltersChanged FINAL)
    Q_PROPERTY(int editableMaximumYear READ editableMaximumYear
                   NOTIFY editableFiltersChanged FINAL)
    Q_PROPERTY(double editableMinimumRating READ editableMinimumRating
                   NOTIFY editableFiltersChanged FINAL)
    Q_PROPERTY(int editableGenreMatchMode READ editableGenreMatchMode
                   NOTIFY editableFiltersChanged FINAL)
    Q_PROPERTY(bool editableExcludeWatched READ editableExcludeWatched
                   NOTIFY editableFiltersChanged FINAL)

    Q_PROPERTY(int appliedMediaType READ appliedMediaType
                   NOTIFY filtersChanged FINAL)
    Q_PROPERTY(int appliedMinimumYear READ appliedMinimumYear
                   NOTIFY filtersChanged FINAL)
    Q_PROPERTY(int appliedMaximumYear READ appliedMaximumYear
                   NOTIFY filtersChanged FINAL)
    Q_PROPERTY(double appliedMinimumRating READ appliedMinimumRating
                   NOTIFY filtersChanged FINAL)
    Q_PROPERTY(int appliedGenreCount READ appliedGenreCount
                   NOTIFY filtersChanged FINAL)
    Q_PROPERTY(bool appliedExcludeWatched READ appliedExcludeWatched
                   NOTIFY filtersChanged FINAL)

public:
    enum class State
    {
        Idle,
        Loading,
        Ready,
        Empty,
        NetworkError,
        RateLimited
    };
    Q_ENUM(State)

    using EligibilityFilter = std::function<bool(const Domain::CandidateIdentity &)>;

    HomeController(Infrastructure::TmdbClient &tmdbClient,
                   Infrastructure::JsonStore &jsonStore,
                   Infrastructure::PosterUrlResolver &posterUrlResolver,
                   QObject *parent = nullptr);

    void setEligibilityFilter(EligibilityFilter filter);

    [[nodiscard]] State state() const noexcept;
    [[nodiscard]] QString errorText() const noexcept;
    [[nodiscard]] bool canReroll() const noexcept;
    [[nodiscard]] bool replenishing() const noexcept;
    [[nodiscard]] bool trailerLoading() const noexcept;

    [[nodiscard]] bool hasSuggestion() const noexcept;
    [[nodiscard]] QString title() const;
    [[nodiscard]] int releaseYear() const noexcept;
    [[nodiscard]] bool isTv() const noexcept;
    [[nodiscard]] double rating() const noexcept;
    [[nodiscard]] qint64 voteCount() const noexcept;
    [[nodiscard]] QString overview() const;
    [[nodiscard]] QString posterUrl() const;
    [[nodiscard]] qint64 currentTmdbId() const noexcept;
    [[nodiscard]] QString currentPosterPath() const;
    [[nodiscard]] QVariantList currentGenreIds() const;
    [[nodiscard]] QStringList currentGenreNames() const;
    [[nodiscard]] QVariantList currentStreamingProviders() const noexcept;

    [[nodiscard]] int rerollCount() const noexcept;
    [[nodiscard]] bool recycled() const noexcept;

    [[nodiscard]] bool hasUnappliedChanges() const noexcept;
    [[nodiscard]] ViewModels::Models::GenreListModel *genreModel() noexcept;

    [[nodiscard]] int editableMediaType() const noexcept;
    [[nodiscard]] int editableMinimumYear() const noexcept;
    [[nodiscard]] int editableMaximumYear() const noexcept;
    [[nodiscard]] double editableMinimumRating() const noexcept;
    [[nodiscard]] int editableGenreMatchMode() const noexcept;
    [[nodiscard]] bool editableExcludeWatched() const noexcept;

    [[nodiscard]] int appliedMediaType() const noexcept;
    [[nodiscard]] int appliedMinimumYear() const noexcept;
    [[nodiscard]] int appliedMaximumYear() const noexcept;
    [[nodiscard]] double appliedMinimumRating() const noexcept;
    [[nodiscard]] int appliedGenreCount() const noexcept;
    [[nodiscard]] bool appliedExcludeWatched() const noexcept;

    Q_INVOKABLE void start();
    Q_INVOKABLE void reroll();
    Q_INVOKABLE void retry();

    Q_INVOKABLE bool setEditableMediaType(int mediaType);
    Q_INVOKABLE bool setEditableYearRange(int minimumYear, int maximumYear);
    Q_INVOKABLE bool setEditableMinimumRating(double minimumRating);
    Q_INVOKABLE bool setEditableGenreMatchMode(int genreMatchMode);
    Q_INVOKABLE bool setEditableExcludeWatched(bool excludeWatched);
    Q_INVOKABLE bool setGenreSelected(int genreId, bool selected);
    Q_INVOKABLE bool apply();
    Q_INVOKABLE bool reset();
    Q_INVOKABLE void playTrailer();

signals:
    void stateChanged();
    void replenishingChanged();
    void trailerLoadingChanged();
    void suggestionChanged();
    void streamingProvidersChanged();
    void sessionChanged();
    void filtersChanged();
    void editableFiltersChanged();
    void hasUnappliedChangesChanged();
    void recyclingStarted();

private:
    struct PickOutcome final
    {
        const Domain::Candidate *candidate{nullptr};
        bool hadEligibleCandidate{false};
    };

    enum class PickResult
    {
        Selected,
        Pending,
        Empty
    };

    [[nodiscard]] bool setEditableFilters(Domain::FilterCriteria updated);
    void applyFilters(Domain::FilterCriteria updated);
    void beginSessionLoad();
    void fetchPage(Domain::MediaType mediaType, quint32 page, bool isInitial);
    void handlePageResult(std::uint64_t generation,
                          Domain::MediaType mediaType,
                          quint32 page,
                          bool isInitial,
                          Infrastructure::TmdbClient::DiscoverResult result);
    void finishInitialLoadIfReady(std::uint64_t generation);
    void resolvePickResult(PickResult result);
    [[nodiscard]] PickResult pickAndSelect(bool isReroll);
    [[nodiscard]] PickOutcome pickEligibleCandidate();
    [[nodiscard]] bool isEligible(const Domain::Candidate &candidate) const;
    [[nodiscard]] bool canRequestMorePages() const noexcept;
    void requestReplenishmentIfNeeded();
    [[nodiscard]] bool transitionTo(State next) noexcept;
    [[nodiscard]] static bool isTransitionAllowed(State current, State next) noexcept;
    [[nodiscard]] quint32 randomStartPage() const;
    [[nodiscard]] Domain::CandidatePool &poolFor(Domain::MediaType mediaType) noexcept;
    [[nodiscard]] const Domain::CandidatePool &poolFor(
        Domain::MediaType mediaType) const noexcept;
    [[nodiscard]] static Infrastructure::TmdbDiscoverRequestDto buildRequest(
        const Domain::FilterCriteria &filters,
        quint32 page);
    void persistFilters();
    void fetchStreamingProviders();
    void fetchGenreLists();
    void refreshGenreModel();
    [[nodiscard]] QStringList genreNamesFor(
        Domain::MediaType mediaType,
        const std::vector<std::int32_t> &genreIds) const;

    Infrastructure::TmdbClient &m_tmdbClient;
    Infrastructure::JsonStore &m_jsonStore;
    Infrastructure::PosterUrlResolver &m_posterUrlResolver;
    EligibilityFilter m_eligibilityFilter;

    Domain::FilterCriteria m_appliedFilters;
    Domain::FilterCriteria m_editableFilters;
    Domain::CandidatePool m_moviePool;
    Domain::CandidatePool m_tvPool;
    Domain::SuggestionSession m_session;
    Domain::WeightedPicker m_picker;
    Domain::ScoringConfig m_scoringConfig;

    ViewModels::Models::GenreListModel m_genreModel;
    ViewModels::Models::GenreListModel::Entries m_movieGenreEntries;
    ViewModels::Models::GenreListModel::Entries m_tvGenreEntries;

    State m_state{State::Idle};
    QString m_errorText;
    std::uint64_t m_generation{0};

    bool m_movieActive{true};
    bool m_tvActive{false};
    bool m_moviePendingFetch{false};
    bool m_tvPendingFetch{false};
    int m_pendingInitialFetches{0};
    bool m_initialLoadFailed{false};

    bool m_awaitingReplenishmentForPick{false};
    bool m_lastPickWasReroll{false};
    bool m_recycled{false};
    bool m_trailerLoading{false};

    QVariantList m_currentStreamingProviders;
    qint64 m_streamingProvidersTmdbId{0};
    bool m_streamingProvidersIsTv{false};
    bool m_streamingProvidersLoaded{false};
};

}
