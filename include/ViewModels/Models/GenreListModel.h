#pragma once

#include "Domain/FilterCriteria.h"

#include <QAbstractListModel>
#include <QString>

#include <vector>

namespace Reroll::ViewModels::Models
{

class GenreListModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    struct Entry final
    {
        Domain::FilterCriteria::GenreId id{0};
        QString name;
        bool selected{false};
    };

    using Entries = std::vector<Entry>;

    enum Role
    {
        GenreIdRole = Qt::UserRole + 1,
        NameRole,
        SelectedRole
    };
    Q_ENUM(Role)

    explicit GenreListModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(
        const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index,
                                int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] const Entries &entries() const noexcept;
    void updateEntries(Entries entries);
    [[nodiscard]] bool setSelected(Domain::FilterCriteria::GenreId genreId,
                                   bool selected);

private:
    Entries m_entries;
};

}
