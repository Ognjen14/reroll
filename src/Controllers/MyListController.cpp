#include "Controllers/MyListController.h"

#include "Infrastructure/JsonStore.h"
#include "Infrastructure/PosterUrlResolver.h"

#include "RRLog.h"

#include <algorithm>
#include <iterator>
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

int MyListController::countForMode(int mode) const
{
    using Mode = ViewModels::Models::MyListFilterModel::Mode;
    const auto &entries = m_model.entries();
    return static_cast<int>(std::count_if(
        entries.cbegin(),
        entries.cend(),
        [mode](const Domain::MyListEntry &entry) {
            const bool listed = entry.watchlist() || entry.watched();
            const bool isTv = entry.snapshot().mediaType() == Domain::MediaType::Tv;
            switch (mode)
            {
            case Mode::Watchlist:
                return entry.watchlist();
            case Mode::Watched:
                return entry.watched();
            case Mode::Movie:
                return listed && !isTv;
            case Mode::Tv:
                return listed && isTv;
            case Mode::Hidden:
                return entry.hidden();
            case Mode::All:
            default:
                return listed;
            }
        }));
}

QVariantMap MyListController::captureState(qlonglong tmdbId, int mediaType) const
{
    const Domain::CandidateIdentity identity(
        mediaType == 1 ? Domain::MediaType::Tv : Domain::MediaType::Movie, tmdbId);
    const auto &entries = m_model.entries();
    const auto existing = std::find_if(
        entries.cbegin(),
        entries.cend(),
        [&identity](const Domain::MyListEntry &entry) {
            return entry.snapshot().identity() == identity;
        });

    QVariantMap state;
    state.insert(QStringLiteral("tmdbId"), tmdbId);
    state.insert(QStringLiteral("mediaType"), mediaType);
    state.insert(QStringLiteral("exists"), existing != entries.cend());
    if (existing == entries.cend())
    {
        RR_LOG_D() << "MyList state captured for absent title" << tmdbId;
        return state;
    }

    const Domain::TitleSnapshot &snapshot = existing->snapshot();
    QVariantList genreIds;
    for (const auto genreId : snapshot.genreIds())
    {
        genreIds.push_back(genreId);
    }
    state.insert(QStringLiteral("row"),
                 static_cast<int>(std::distance(entries.cbegin(), existing)));
    state.insert(QStringLiteral("title"), QString::fromStdString(snapshot.title()));
    state.insert(QStringLiteral("releaseYear"), snapshot.releaseYear());
    state.insert(QStringLiteral("genreIds"), genreIds);
    state.insert(QStringLiteral("posterPath"), QString::fromStdString(snapshot.posterPath()));
    state.insert(QStringLiteral("rating"), snapshot.rating());
    state.insert(QStringLiteral("voteCount"), static_cast<qlonglong>(snapshot.voteCount()));
    state.insert(QStringLiteral("watchlist"), existing->watchlist());
    state.insert(QStringLiteral("watched"), existing->watched());
    state.insert(QStringLiteral("hidden"), existing->hidden());
    RR_LOG_D() << "MyList state captured" << QString::fromStdString(snapshot.title())
               << "row" << state.value(QStringLiteral("row")).toInt();
    return state;
}

void MyListController::restoreState(const QVariantMap &state)
{
    const qlonglong tmdbId = state.value(QStringLiteral("tmdbId")).toLongLong();
    const int mediaType = state.value(QStringLiteral("mediaType")).toInt();
    const Domain::CandidateIdentity identity(
        mediaType == 1 ? Domain::MediaType::Tv : Domain::MediaType::Movie, tmdbId);

    if (!state.value(QStringLiteral("exists")).toBool())
    {
        RR_LOG_I() << "MyList undo removes title" << tmdbId;
        m_model.removeEntry(identity);
    }
    else
    {
        Domain::MyListEntry entry(
            buildSnapshot(tmdbId,
                          mediaType,
                          state.value(QStringLiteral("title")).toString(),
                          state.value(QStringLiteral("releaseYear")).toInt(),
                          state.value(QStringLiteral("genreIds")).toList(),
                          state.value(QStringLiteral("posterPath")).toString(),
                          state.value(QStringLiteral("rating")).toDouble(),
                          state.value(QStringLiteral("voteCount")).toLongLong()),
            state.value(QStringLiteral("watchlist")).toBool(),
            state.value(QStringLiteral("watched")).toBool(),
            state.value(QStringLiteral("hidden")).toBool());
        RR_LOG_I() << "MyList undo restores title" << QString::fromStdString(entry.snapshot().title())
                   << "row" << state.value(QStringLiteral("row")).toInt();
        m_model.insertEntryAt(state.value(QStringLiteral("row")).toInt(), std::move(entry));
    }
    persist();
    bumpRevision();
    emit watchedChanged();
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
