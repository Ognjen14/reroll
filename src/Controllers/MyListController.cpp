#include "Controllers/MyListController.h"

#include "Infrastructure/JsonStore.h"
#include "Infrastructure/PosterUrlResolver.h"

#include "RRLog.h"

#include <algorithm>
#include <utility>

namespace Reroll::Controllers
{

MyListController::MyListController(Infrastructure::JsonStore &jsonStore,
                                   Infrastructure::PosterUrlResolver &posterUrlResolver,
                                   QObject *parent)
    : QObject(parent)
    , m_jsonStore(jsonStore)
    , m_posterUrlResolver(posterUrlResolver)
{
    ViewModels::Models::TitleListModel::Entries entries(
        m_jsonStore.myList().cbegin(), m_jsonStore.myList().cend());
    RR_LOG_I() << "MyListController loaded" << entries.size() << "entries";
    m_model.setEntries(std::move(entries));
    m_filterModel.setSourceModel(&m_model);

    connect(&m_posterUrlResolver,
            &Infrastructure::PosterUrlResolver::configurationLoaded,
            &m_model,
            [this]() {
                if (m_model.rowCount() > 0)
                {
                    emit m_model.dataChanged(
                        m_model.index(0, 0),
                        m_model.index(m_model.rowCount() - 1, 0),
                        {ViewModels::Models::TitleListModel::PosterPathRole});
                }
            });
}

ViewModels::Models::TitleListModel *MyListController::model() noexcept
{
    return &m_model;
}

ViewModels::Models::MyListFilterModel *MyListController::filteredModel() noexcept
{
    return &m_filterModel;
}

int MyListController::revision() const noexcept
{
    return m_revision;
}

bool MyListController::isWatched(const Domain::CandidateIdentity &identity) const noexcept
{
    const auto &entries = m_model.entries();
    const auto found = std::find_if(
        entries.cbegin(),
        entries.cend(),
        [&identity](const Domain::MyListEntry &entry) {
            return entry.snapshot().identity() == identity;
        });
    return found != entries.cend() && found->watched();
}

bool MyListController::isHidden(const Domain::CandidateIdentity &identity) const noexcept
{
    const auto &entries = m_model.entries();
    const auto found = std::find_if(
        entries.cbegin(),
        entries.cend(),
        [&identity](const Domain::MyListEntry &entry) {
            return entry.snapshot().identity() == identity;
        });
    return found != entries.cend() && found->hidden();
}

bool MyListController::isInWatchlist(qlonglong tmdbId, int mediaType) const
{
    const Domain::CandidateIdentity identity(
        mediaType == 1 ? Domain::MediaType::Tv : Domain::MediaType::Movie, tmdbId);
    const auto &entries = m_model.entries();
    const auto found = std::find_if(
        entries.cbegin(),
        entries.cend(),
        [&identity](const Domain::MyListEntry &entry) {
            return entry.snapshot().identity() == identity;
        });
    return found != entries.cend() && found->watchlist();
}

bool MyListController::isMarkedWatched(qlonglong tmdbId, int mediaType) const
{
    const Domain::CandidateIdentity identity(
        mediaType == 1 ? Domain::MediaType::Tv : Domain::MediaType::Movie, tmdbId);
    return isWatched(identity);
}

int MyListController::totalCount() const noexcept
{
    const auto &entries = m_model.entries();
    return static_cast<int>(std::count_if(
        entries.cbegin(),
        entries.cend(),
        [](const Domain::MyListEntry &entry) {
            return entry.watchlist() || entry.watched();
        }));
}

Domain::TitleSnapshot MyListController::buildSnapshot(qlonglong tmdbId,
                                                       int mediaType,
                                                       const QString &title,
                                                       int releaseYear,
                                                       const QVariantList &genreIds,
                                                       const QString &posterPath,
                                                       double rating,
                                                       qlonglong voteCount) const
{
    Domain::TitleSnapshot::GenreIds ids;
    ids.reserve(static_cast<std::size_t>(genreIds.size()));
    for (const QVariant &genreId : genreIds)
    {
        ids.push_back(genreId.toInt());
    }

    return Domain::TitleSnapshot(
        tmdbId,
        mediaType == 1 ? Domain::MediaType::Tv : Domain::MediaType::Movie,
        title.toStdString(),
        releaseYear,
        std::move(ids),
        posterPath.toStdString(),
        rating,
        voteCount);
}

void MyListController::setWatchlist(qlonglong tmdbId,
                                    int mediaType,
                                    const QString &title,
                                    int releaseYear,
                                    const QVariantList &genreIds,
                                    const QString &posterPath,
                                    double rating,
                                    qlonglong voteCount,
                                    bool watchlist)
{
    Domain::TitleSnapshot snapshot = buildSnapshot(
        tmdbId, mediaType, title, releaseYear, genreIds, posterPath, rating, voteCount);
    const Domain::CandidateIdentity identity = snapshot.identity();

    const auto &entries = m_model.entries();
    const auto existing = std::find_if(
        entries.cbegin(),
        entries.cend(),
        [&identity](const Domain::MyListEntry &entry) {
            return entry.snapshot().identity() == identity;
        });
    const bool watched = existing != entries.cend() && existing->watched();
    const bool hidden = existing != entries.cend() && existing->hidden();

    Domain::MyListEntry entry(std::move(snapshot), watchlist, watched, hidden);
    RR_LOG_I() << "MyList watchlist set" << QString::fromStdString(entry.snapshot().title())
               << watchlist;

    if (entry.isEmpty())
    {
        m_model.removeEntry(identity);
    }
    else
    {
        m_model.upsertEntry(std::move(entry));
    }
    persist();
    bumpRevision();
}

void MyListController::setWatched(qlonglong tmdbId,
                                  int mediaType,
                                  const QString &title,
                                  int releaseYear,
                                  const QVariantList &genreIds,
                                  const QString &posterPath,
                                  double rating,
                                  qlonglong voteCount,
                                  bool watched)
{
    Domain::TitleSnapshot snapshot = buildSnapshot(
        tmdbId, mediaType, title, releaseYear, genreIds, posterPath, rating, voteCount);
    const Domain::CandidateIdentity identity = snapshot.identity();

    const auto &entries = m_model.entries();
    const auto existing = std::find_if(
        entries.cbegin(),
        entries.cend(),
        [&identity](const Domain::MyListEntry &entry) {
            return entry.snapshot().identity() == identity;
        });
    const bool watchlist = existing != entries.cend() && existing->watchlist();
    const bool hidden = existing != entries.cend() && existing->hidden();

    Domain::MyListEntry entry(std::move(snapshot), watchlist, watched, hidden);
    RR_LOG_I() << "MyList watched set" << QString::fromStdString(entry.snapshot().title())
               << watched;

    if (entry.isEmpty())
    {
        m_model.removeEntry(identity);
    }
    else
    {
        m_model.upsertEntry(std::move(entry));
    }
    persist();
    bumpRevision();
    emit watchedChanged();
}

void MyListController::setHidden(qlonglong tmdbId,
                                 int mediaType,
                                 const QString &title,
                                 int releaseYear,
                                 const QVariantList &genreIds,
                                 const QString &posterPath,
                                 double rating,
                                 qlonglong voteCount,
                                 bool hidden)
{
    Domain::TitleSnapshot snapshot = buildSnapshot(
        tmdbId, mediaType, title, releaseYear, genreIds, posterPath, rating, voteCount);
    const Domain::CandidateIdentity identity = snapshot.identity();

    const auto &entries = m_model.entries();
    const auto existing = std::find_if(
        entries.cbegin(),
        entries.cend(),
        [&identity](const Domain::MyListEntry &entry) {
            return entry.snapshot().identity() == identity;
        });
    const bool watchlist = existing != entries.cend() && existing->watchlist();
    const bool watched = existing != entries.cend() && existing->watched();

    Domain::MyListEntry entry(std::move(snapshot), watchlist, watched, hidden);
    RR_LOG_I() << "MyList hidden set" << QString::fromStdString(entry.snapshot().title())
               << hidden;

    if (entry.isEmpty())
    {
        m_model.removeEntry(identity);
    }
    else
    {
        m_model.upsertEntry(std::move(entry));
    }
    persist();
    bumpRevision();
}

void MyListController::persist()
{
    m_jsonStore.setMyList(m_model.entries());
}

void MyListController::bumpRevision()
{
    ++m_revision;
    emit revisionChanged();
}

}
