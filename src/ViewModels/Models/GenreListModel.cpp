#include "ViewModels/Models/GenreListModel.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace Reroll::ViewModels::Models
{

GenreListModel::GenreListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int GenreListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
}

QVariant GenreListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()
        || index.row() < 0
        || static_cast<std::size_t>(index.row()) >= m_entries.size())
    {
        return {};
    }

    const Entry &entry = m_entries[static_cast<std::size_t>(index.row())];
    switch (role)
    {
    case Qt::DisplayRole:
    case NameRole:
        return entry.name;
    case GenreIdRole:
        return entry.id;
    case SelectedRole:
        return entry.selected;
    default:
        return {};
    }
}

QHash<int, QByteArray> GenreListModel::roleNames() const
{
    return {
        {GenreIdRole, QByteArrayLiteral("genreId")},
        {NameRole, QByteArrayLiteral("name")},
        {SelectedRole, QByteArrayLiteral("selected")}};
}

const GenreListModel::Entries &GenreListModel::entries() const noexcept
{
    return m_entries;
}

void GenreListModel::updateEntries(Entries entries)
{
    const std::size_t sharedCount = std::min(m_entries.size(), entries.size());
    for (std::size_t row = 0; row < sharedCount; ++row)
    {
        QList<int> changedRoles;
        if (m_entries[row].id != entries[row].id)
        {
            changedRoles.push_back(GenreIdRole);
        }
        if (m_entries[row].name != entries[row].name)
        {
            changedRoles.push_back(Qt::DisplayRole);
            changedRoles.push_back(NameRole);
        }
        if (m_entries[row].selected != entries[row].selected)
        {
            changedRoles.push_back(SelectedRole);
        }

        m_entries[row] = std::move(entries[row]);
        if (!changedRoles.isEmpty())
        {
            const QModelIndex changedIndex = index(static_cast<int>(row), 0);
            emit dataChanged(changedIndex, changedIndex, changedRoles);
        }
    }

    if (m_entries.size() > entries.size())
    {
        const int firstRemovedRow = static_cast<int>(entries.size());
        const int lastRemovedRow = static_cast<int>(m_entries.size() - 1);
        beginRemoveRows(QModelIndex(), firstRemovedRow, lastRemovedRow);
        m_entries.resize(entries.size());
        endRemoveRows();
    }
    else if (m_entries.size() < entries.size())
    {
        const std::size_t firstInserted = m_entries.size();
        beginInsertRows(QModelIndex(),
                        static_cast<int>(firstInserted),
                        static_cast<int>(entries.size() - 1));
        for (std::size_t row = firstInserted; row < entries.size(); ++row)
        {
            m_entries.push_back(std::move(entries[row]));
        }
        endInsertRows();
    }
}

bool GenreListModel::setSelected(Domain::FilterCriteria::GenreId genreId,
                                 bool selected)
{
    const auto position = std::find_if(
        m_entries.begin(),
        m_entries.end(),
        [genreId](const Entry &entry) { return entry.id == genreId; });
    if (position == m_entries.end() || position->selected == selected)
    {
        return false;
    }

    position->selected = selected;
    const int row = static_cast<int>(std::distance(m_entries.begin(), position));
    const QModelIndex changedIndex = index(row, 0);
    emit dataChanged(changedIndex, changedIndex, {SelectedRole});
    return true;
}

}
