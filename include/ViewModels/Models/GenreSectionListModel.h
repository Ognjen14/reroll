#pragma once

#include "TitleListModel.h"

#include <QAbstractListModel>
#include <QString>

#include <cstdint>
#include <utility>
#include <vector>

namespace Reroll::ViewModels::Models
{

class GenreSectionListModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        GenreIdRole = Qt::UserRole + 1,
        NameRole,
        TitleModelRole,
        TotalResultsRole
    };
    Q_ENUM(Role)

    explicit GenreSectionListModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(
        const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index,
                                int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void setGenres(std::vector<std::pair<std::int32_t, QString>> genres);
    [[nodiscard]] TitleListModel *titleModelForRow(int row) noexcept;
    void setTotalResults(int row, qint64 totalResults);

private:
    struct Section final
    {
        std::int32_t genreId{0};
        QString name;
        TitleListModel *titleModel{nullptr};
        qint64 totalResults{0};
    };

    std::vector<Section> m_sections;
};

}
