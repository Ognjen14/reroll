#include "ViewModels/Models/TitleListModel.h"

#include <QVariantList>

#include <algorithm>
#include <iterator>
#include <utility>

namespace Reroll::ViewModels::Models
{

TitleListModel::TitleListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TitleListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
}

QVariant TitleListModel::data(const QModelIndex &index, int role) const
{
    const Domain::MyListEntry *entry = entryAt(index.row());
    if (!index.isValid() || entry == nullptr)
    {
        return {};
    }

    const Domain::TitleSnapshot &snapshot = entry->snapshot();
    switch (role)
    {
    case Qt::DisplayRole:
    case TitleRole:
        return QString::fromStdString(snapshot.title());
    case TmdbIdRole:
        return QVariant::fromValue(static_cast<qlonglong>(snapshot.tmdbId()));
    case MediaTypeRole:
        return snapshot.mediaType() == Domain::MediaType::Movie ? 0 : 1;
    case ReleaseYearRole:
        return snapshot.releaseYear();
    case GenreIdsRole:
    {
        QVariantList genreIds;
        genreIds.reserve(static_cast<qsizetype>(snapshot.genreIds().size()));
        for (const Domain::TitleSnapshot::GenreId genreId : snapshot.genreIds())
        {
            genreIds.push_back(genreId);
        }
        return genreIds;
    }
    case PosterPathRole:
        return QString::fromStdString(snapshot.posterPath());
    case RatingRole:
        return snapshot.rating();
    case VoteCountRole:
        return QVariant::fromValue(static_cast<qlonglong>(snapshot.voteCount()));
    case WatchlistRole:
        return entry->watchlist();
    case WatchedRole:
        return entry->watched();
    case HiddenRole:
        return entry->hidden();
    default:
        return {};
    }
}

QHash<int, QByteArray> TitleListModel::roleNames() const
{
    return {
        {TmdbIdRole, QByteArrayLiteral("tmdbId")},
        {MediaTypeRole, QByteArrayLiteral("mediaType")},
        {TitleRole, QByteArrayLiteral("title")},
        {ReleaseYearRole, QByteArrayLiteral("releaseYear")},
        {GenreIdsRole, QByteArrayLiteral("genreIds")},
        {PosterPathRole, QByteArrayLiteral("posterPath")},
        {RatingRole, QByteArrayLiteral("rating")},
        {VoteCountRole, QByteArrayLiteral("voteCount")},
        {WatchlistRole, QByteArrayLiteral("watchlist")},
        {WatchedRole, QByteArrayLiteral("watched")},
        {HiddenRole, QByteArrayLiteral("hidden")}};
}

const TitleListModel::Entries &TitleListModel::entries() const noexcept
{
    return m_entries;
}

const Domain::MyListEntry *TitleListModel::entryAt(int row) const noexcept
{
    if (row < 0 || static_cast<std::size_t>(row) >= m_entries.size())
    {
        return nullptr;
    }
    return &m_entries[static_cast<std::size_t>(row)];
}

void TitleListModel::setEntries(Entries entries)
{
    beginResetModel();
    m_entries = std::move(entries);
    endResetModel();
}

void TitleListModel::appendEntries(Entries entries)
{
    if (entries.empty())
    {
        return;
    }

    const int startRow = static_cast<int>(m_entries.size());
    const int endRow = startRow + static_cast<int>(entries.size()) - 1;
    beginInsertRows(QModelIndex(), startRow, endRow);
    m_entries.insert(m_entries.end(),
                     std::make_move_iterator(entries.begin()),
                     std::make_move_iterator(entries.end()));
    endInsertRows();
}

void TitleListModel::upsertEntry(Domain::MyListEntry entry)
{
    const auto existing = std::find_if(
        m_entries.begin(),
        m_entries.end(),
        [&entry](const Domain::MyListEntry &item) {
            return item.snapshot().identity() == entry.snapshot().identity();
        });

    if (existing != m_entries.end())
    {
        *existing = std::move(entry);
        const int row = static_cast<int>(std::distance(m_entries.begin(), existing));
        const QModelIndex changedIndex = index(row, 0);
        emit dataChanged(changedIndex, changedIndex);
        return;
    }

    const int row = static_cast<int>(m_entries.size());
    beginInsertRows(QModelIndex(), row, row);
    m_entries.push_back(std::move(entry));
    endInsertRows();
}

bool TitleListModel::removeEntry(const Domain::CandidateIdentity &identity)
{
    const auto existing = std::find_if(
        m_entries.begin(),
        m_entries.end(),
        [&identity](const Domain::MyListEntry &item) {
            return item.snapshot().identity() == identity;
        });
    if (existing == m_entries.end())
    {
        return false;
    }

    const int row = static_cast<int>(std::distance(m_entries.begin(), existing));
    beginRemoveRows(QModelIndex(), row, row);
    m_entries.erase(existing);
    endRemoveRows();
    return true;
}

void TitleListModel::clear()
{
    if (m_entries.empty())
    {
        return;
    }

    beginRemoveRows(QModelIndex(), 0, static_cast<int>(m_entries.size()) - 1);
    m_entries.clear();
    endRemoveRows();
}

}
