#pragma once

#include <QSortFilterProxyModel>

namespace Reroll::ViewModels::Models
{

class MyListFilterModel final : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY modeChanged FINAL)
    Q_PROPERTY(int sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged FINAL)

public:
    enum Mode
    {
        All,
        Watchlist,
        Watched,
        Movie,
        Tv,
        Hidden
    };
    Q_ENUM(Mode)

    enum SortMode
    {
        DateAdded,
        Title,
        ReleaseYear,
        Rating
    };
    Q_ENUM(SortMode)

    explicit MyListFilterModel(QObject *parent = nullptr);

    [[nodiscard]] int mode() const noexcept;
    void setMode(int mode);
    [[nodiscard]] int sortMode() const noexcept;
    void setSortMode(int sortMode);

signals:
    void modeChanged();
    void sortModeChanged();

protected:
    [[nodiscard]] bool filterAcceptsRow(
        int sourceRow, const QModelIndex &sourceParent) const override;
    [[nodiscard]] bool lessThan(const QModelIndex &sourceLeft,
                                const QModelIndex &sourceRight) const override;

private:
    int m_mode{All};
    int m_sortMode{DateAdded};
};

}
