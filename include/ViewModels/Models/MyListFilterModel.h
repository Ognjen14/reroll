#pragma once

#include <QSortFilterProxyModel>

namespace Reroll::ViewModels::Models
{

class MyListFilterModel final : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY modeChanged FINAL)

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

    explicit MyListFilterModel(QObject *parent = nullptr);

    [[nodiscard]] int mode() const noexcept;
    void setMode(int mode);

signals:
    void modeChanged();

protected:
    [[nodiscard]] bool filterAcceptsRow(
        int sourceRow, const QModelIndex &sourceParent) const override;

private:
    int m_mode{All};
};

}
