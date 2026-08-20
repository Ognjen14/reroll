#include "ViewModels/Models/MyListFilterModel.h"

#include "ViewModels/Models/TitleListModel.h"

namespace Reroll::ViewModels::Models
{

MyListFilterModel::MyListFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

int MyListFilterModel::mode() const noexcept
{
    return m_mode;
}

void MyListFilterModel::setMode(int mode)
{
    if (m_mode == mode)
    {
        return;
    }

    m_mode = mode;
    emit modeChanged();
    invalidateFilter();
}

bool MyListFilterModel::filterAcceptsRow(
    int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    const bool watchlist = sourceIndex.data(TitleListModel::WatchlistRole).toBool();
    const bool watched = sourceIndex.data(TitleListModel::WatchedRole).toBool();

    switch (m_mode)
    {
    case Watchlist:
        return watchlist;
    case Watched:
        return watched;
    case Movie:
        return (watchlist || watched)
            && sourceIndex.data(TitleListModel::MediaTypeRole).toInt() == 0;
    case Tv:
        return (watchlist || watched)
            && sourceIndex.data(TitleListModel::MediaTypeRole).toInt() == 1;
    case Hidden:
        return sourceIndex.data(TitleListModel::HiddenRole).toBool();
    case All:
    default:
        return watchlist || watched;
    }
}

}
