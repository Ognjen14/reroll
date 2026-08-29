#pragma once

#include "Infrastructure/TmdbClient.h"
#include "ViewModels/Models/GenreSectionListModel.h"
#include "ViewModels/Models/TitleListModel.h"

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariant>

#include <cstdint>
#include <functional>
#include <vector>

namespace Reroll::Infrastructure
{
class PosterUrlResolver;
}

namespace Reroll::Controllers
{

class DiscoverController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Reroll::ViewModels::Models::TitleListModel *trendingMovies
                   READ trendingMovies CONSTANT FINAL)
    Q_PROPERTY(Reroll::ViewModels::Models::TitleListModel *trendingTv
                   READ trendingTv CONSTANT FINAL)
    Q_PROPERTY(Reroll::ViewModels::Models::TitleListModel *popularMovies
                   READ popularMovies CONSTANT FINAL)
    Q_PROPERTY(Reroll::ViewModels::Models::TitleListModel *popularTv
                   READ popularTv CONSTANT FINAL)
    Q_PROPERTY(Reroll::ViewModels::Models::GenreSectionListModel *genreSections
                   READ genreSections CONSTANT FINAL)
    Q_PROPERTY(Reroll::ViewModels::Models::GenreSectionListModel *tvGenreSections
                   READ tvGenreSections CONSTANT FINAL)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged FINAL)
    Q_PROPERTY(bool loadFailed READ loadFailed NOTIFY loadFailedChanged FINAL)

    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery
                   NOTIFY searchQueryChanged FINAL)
    Q_PROPERTY(int searchScope READ searchScope WRITE setSearchScope
                   NOTIFY searchScopeChanged FINAL)
    Q_PROPERTY(Reroll::ViewModels::Models::TitleListModel *searchResults
                   READ searchResults CONSTANT FINAL)
    Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged FINAL)
    Q_PROPERTY(bool loadingMore READ loadingMore NOTIFY loadingMoreChanged FINAL)

    Q_PROPERTY(bool titleDetailsLoading READ titleDetailsLoading
                   NOTIFY titleDetailsChanged FINAL)
    Q_PROPERTY(qint64 titleDetailsTmdbId READ titleDetailsTmdbId
                   NOTIFY titleDetailsChanged FINAL)
    Q_PROPERTY(QString titleDetailsOverview READ titleDetailsOverview
                   NOTIFY titleDetailsChanged FINAL)
    Q_PROPERTY(QStringList titleDetailsGenreNames READ titleDetailsGenreNames
                   NOTIFY titleDetailsChanged FINAL)
    Q_PROPERTY(QVariantList titleDetailsStreamingProviders READ titleDetailsStreamingProviders
                   NOTIFY titleDetailsChanged FINAL)
    Q_PROPERTY(bool titleDetailsTrailerLoading READ titleDetailsTrailerLoading
                   NOTIFY titleDetailsTrailerLoadingChanged FINAL)

public:
    enum SearchScope
    {
        ScopeAll,
        ScopeMovie,
        ScopeTv
    };
    Q_ENUM(SearchScope)

    DiscoverController(Infrastructure::TmdbClient &tmdbClient,
                       Infrastructure::PosterUrlResolver &posterUrlResolver,
                       QObject *parent = nullptr);

    [[nodiscard]] ViewModels::Models::TitleListModel *trendingMovies() noexcept;
    [[nodiscard]] ViewModels::Models::TitleListModel *trendingTv() noexcept;
    [[nodiscard]] ViewModels::Models::TitleListModel *popularMovies() noexcept;
    [[nodiscard]] ViewModels::Models::TitleListModel *popularTv() noexcept;
    [[nodiscard]] ViewModels::Models::GenreSectionListModel *genreSections() noexcept;
    [[nodiscard]] ViewModels::Models::GenreSectionListModel *tvGenreSections() noexcept;
    [[nodiscard]] bool loading() const noexcept;
    [[nodiscard]] bool loadFailed() const noexcept;

    [[nodiscard]] QString searchQuery() const noexcept;
    void setSearchQuery(const QString &query);
    [[nodiscard]] int searchScope() const noexcept;
    void setSearchScope(int scope);
    [[nodiscard]] ViewModels::Models::TitleListModel *searchResults() noexcept;
    [[nodiscard]] bool searching() const noexcept;
    [[nodiscard]] bool loadingMore() const noexcept;

    [[nodiscard]] bool titleDetailsLoading() const noexcept;
    [[nodiscard]] qint64 titleDetailsTmdbId() const noexcept;
    [[nodiscard]] QString titleDetailsOverview() const noexcept;
    [[nodiscard]] QStringList titleDetailsGenreNames() const noexcept;
    [[nodiscard]] QVariantList titleDetailsStreamingProviders() const noexcept;
    [[nodiscard]] bool titleDetailsTrailerLoading() const noexcept;

    Q_INVOKABLE void start();
    Q_INVOKABLE void retryInitialLoad();
    Q_INVOKABLE void loadMoreTrendingMovies();
    Q_INVOKABLE void loadMoreForGenre(int genreId);
    Q_INVOKABLE void loadMoreForTvGenre(int genreId);
    Q_INVOKABLE void loadMoreSearchResults();
    Q_INVOKABLE void loadTitleDetails(qint64 tmdbId, int mediaType);
    Q_INVOKABLE void playTitleDetailsTrailer();

signals:
    void loadingChanged();
    void loadFailedChanged();
    void searchQueryChanged();
    void searchScopeChanged();
    void searchingChanged();
    void loadingMoreChanged();
    void titleDetailsChanged();
    void titleDetailsTrailerLoadingChanged();

private:
    struct PaginationState final
    {
        Infrastructure::TmdbPageNumber nextPage{2};
        Infrastructure::TmdbPageNumber totalPages{1};
        bool loadingMore{false};
    };

    using RequestFn = std::function<Infrastructure::TmdbClient::RequestId(
        Infrastructure::TmdbPageNumber,
        Infrastructure::TmdbClient::DiscoverCompletionHandler)>;

    void fetchInto(ViewModels::Models::TitleListModel &target,
                   const char *label,
                   RequestFn requestFn,
                   PaginationState *paginationState = nullptr);
    void fetchGenreSections(bool isTv);
    void fetchGenreRow(bool isTv,
                       std::int32_t genreId,
                       int row,
                       ViewModels::Models::TitleListModel &target);
    void loadMoreForGenreImpl(bool isTv, int genreId);
    void trackInitialSectionResult(bool succeeded);
    void beginRequest();
    void endRequest();
    void beginLoadingMore();
    void endLoadingMore();

    void loadMoreGeneric(ViewModels::Models::TitleListModel &target,
                         PaginationState &state,
                         RequestFn requestFn,
                         const char *label);

    void performSearch();
    void handleSearchResult(std::uint64_t generation,
                            bool isMovie,
                            Infrastructure::TmdbClient::DiscoverResult result);
    void setSearching(bool searching);

    Infrastructure::TmdbClient &m_tmdbClient;
    Infrastructure::PosterUrlResolver &m_posterUrlResolver;

    ViewModels::Models::TitleListModel m_trendingMovies;
    ViewModels::Models::TitleListModel m_trendingTv;
    ViewModels::Models::TitleListModel m_popularMovies;
    ViewModels::Models::TitleListModel m_popularTv;
    ViewModels::Models::GenreSectionListModel m_genreSections;
    ViewModels::Models::GenreSectionListModel m_tvGenreSections;

    PaginationState m_trendingMoviesPagination;
    QHash<std::int32_t, PaginationState> m_genrePagination;
    QHash<std::int32_t, ViewModels::Models::TitleListModel *> m_genreTitleModels;
    QHash<std::int32_t, PaginationState> m_tvGenrePagination;
    QHash<std::int32_t, ViewModels::Models::TitleListModel *> m_tvGenreTitleModels;

    QString m_searchQuery;
    int m_searchScope{ScopeAll};
    ViewModels::Models::TitleListModel m_searchResults;
    QTimer m_searchDebounceTimer;
    std::uint64_t m_searchGeneration{0};
    bool m_searchMoviePending{false};
    bool m_searchTvPending{false};
    bool m_searching{false};
    bool m_searchLoadingMore{false};
    std::vector<Infrastructure::TmdbTitleResultDto> m_searchMovieBuffer;
    std::vector<Infrastructure::TmdbTitleResultDto> m_searchTvBuffer;
    Infrastructure::TmdbPageNumber m_searchMovieNextPage{2};
    Infrastructure::TmdbPageNumber m_searchMovieTotalPages{1};
    Infrastructure::TmdbPageNumber m_searchTvNextPage{2};
    Infrastructure::TmdbPageNumber m_searchTvTotalPages{1};

    bool m_started{false};
    int m_pendingRequestCount{0};
    int m_loadingMoreCount{0};
    int m_initialSectionsPending{0};
    int m_initialSectionsSucceeded{0};
    bool m_loadFailed{false};

    std::uint64_t m_titleDetailsGeneration{0};
    bool m_titleDetailsLoading{false};
    qint64 m_titleDetailsTmdbId{0};
    int m_titleDetailsMediaType{0};
    QString m_titleDetailsOverview;
    QStringList m_titleDetailsGenreNames;
    QVariantList m_titleDetailsStreamingProviders;
    bool m_titleDetailsTrailerLoading{false};
};

}
