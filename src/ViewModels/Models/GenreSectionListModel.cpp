#include "ViewModels/Models/GenreSectionListModel.h"

#include <utility>

namespace Reroll::ViewModels::Models
{

GenreSectionListModel::GenreSectionListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int GenreSectionListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_sections.size());
}

QVariant GenreSectionListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()
        || index.row() < 0
        || static_cast<std::size_t>(index.row()) >= m_sections.size())
    {
        return {};
    }

    const Section &section = m_sections[static_cast<std::size_t>(index.row())];
    switch (role)
    {
    case Qt::DisplayRole:
    case NameRole:
        return section.name;
    case GenreIdRole:
        return section.genreId;
    case TitleModelRole:
        return QVariant::fromValue<QObject *>(section.titleModel);
    case TotalResultsRole:
        return section.totalResults;
    default:
        return {};
    }
}

QHash<int, QByteArray> GenreSectionListModel::roleNames() const
{
    return {
        {GenreIdRole, QByteArrayLiteral("genreId")},
        {NameRole, QByteArrayLiteral("name")},
        {TitleModelRole, QByteArrayLiteral("titleModel")},
        {TotalResultsRole, QByteArrayLiteral("totalResults")}};
}

void GenreSectionListModel::setGenres(std::vector<std::pair<std::int32_t, QString>> genres)
{
    beginResetModel();
    m_sections.clear();
    m_sections.reserve(genres.size());
    for (auto &[genreId, name] : genres)
    {
        Section section;
        section.genreId = genreId;
        section.name = std::move(name);
        section.titleModel = new TitleListModel(this);
        m_sections.push_back(std::move(section));
    }
    endResetModel();
}

TitleListModel *GenreSectionListModel::titleModelForRow(int row) noexcept
{
    if (row < 0 || static_cast<std::size_t>(row) >= m_sections.size())
    {
        return nullptr;
    }
    return m_sections[static_cast<std::size_t>(row)].titleModel;
}

void GenreSectionListModel::setTotalResults(int row, qint64 totalResults)
{
    if (row < 0 || static_cast<std::size_t>(row) >= m_sections.size())
    {
        return;
    }

    m_sections[static_cast<std::size_t>(row)].totalResults = totalResults;
    const QModelIndex changedIndex = index(row, 0);
    emit dataChanged(changedIndex, changedIndex, {TotalResultsRole});
}

}
