#include "ViewModels/Models/MyListFilterModel.h"

#include "ViewModels/Models/TitleListModel.h"

#include "RRLog.h"

namespace Reroll::ViewModels::Models
{

MyListFilterModel::MyListFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::AscendingOrder);
}

int MyListFilterModel::sortMode() const noexcept
{
    return m_sortMode;
}

void MyListFilterModel::setSortMode(int sortMode)
{
    if (sortMode < DateAdded || sortMode > Rating || m_sortMode == sortMode)
    {
        return;
    }

    RR_LOG_I() << "MyList sort mode changed" << m_sortMode << "->" << sortMode;
    m_sortMode = sortMode;
    emit sortModeChanged();
    invalidate();
    sort(0, Qt::AscendingOrder);
}

bool MyListFilterModel::lessThan(const QModelIndex &sourceLeft,
                                 const QModelIndex &sourceRight) const
{
    const auto newerFirst = [&]() {
        return sourceLeft.row() > sourceRight.row();
    };
    const auto byTitle = [&]() {
        const int compared = QString::localeAwareCompare(
            sourceLeft.data(TitleListModel::TitleRole).toString(),
            sourceRight.data(TitleListModel::TitleRole).toString());
        return compared != 0 ? compared < 0 : newerFirst();
    };

    switch (m_sortMode)
    {
    case Title:
        return byTitle();
    case ReleaseYear:
    {
        const int leftYear = sourceLeft.data(TitleListModel::ReleaseYearRole).toInt();
        const int rightYear = sourceRight.data(TitleListModel::ReleaseYearRole).toInt();
        return leftYear != rightYear ? leftYear > rightYear : byTitle();
    }
    case Rating:
    {
        const double leftRating = sourceLeft.data(TitleListModel::RatingRole).toDouble();
        const double rightRating = sourceRight.data(TitleListModel::RatingRole).toDouble();
        return leftRating != rightRating ? leftRating > rightRating : byTitle();
    }
    case DateAdded:
    default:
        return newerFirst();
    }
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
