#pragma once

#include "Domain/CandidateIdentity.h"
#include "Domain/MyListEntry.h"

#include <QAbstractListModel>

#include <vector>

namespace Reroll::ViewModels::Models
{

class TitleListModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        TmdbIdRole = Qt::UserRole + 1,
        MediaTypeRole,
        TitleRole,
        ReleaseYearRole,
        GenreIdsRole,
        PosterPathRole,
        RatingRole,
        VoteCountRole,
        WatchlistRole,
        WatchedRole,
        HiddenRole
    };
    Q_ENUM(Role)

    using Entries = std::vector<Domain::MyListEntry>;

    explicit TitleListModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(
        const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index,
                                int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] const Entries &entries() const noexcept;
    [[nodiscard]] const Domain::MyListEntry *entryAt(int row) const noexcept;

    void setEntries(Entries entries);
    void appendEntries(Entries entries);
    void upsertEntry(Domain::MyListEntry entry);
    bool removeEntry(const Domain::CandidateIdentity &identity);
    void clear();

private:
    Entries m_entries;
};

}
