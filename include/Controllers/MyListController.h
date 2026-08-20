#pragma once

#include "Domain/CandidateIdentity.h"
#include "Domain/TitleSnapshot.h"
#include "ViewModels/Models/MyListFilterModel.h"
#include "ViewModels/Models/TitleListModel.h"

#include <QObject>
#include <QString>
#include <QVariant>

namespace Reroll::Infrastructure
{
class JsonStore;
class PosterUrlResolver;
}

namespace Reroll::Controllers
{

class MyListController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Reroll::ViewModels::Models::TitleListModel *model READ model CONSTANT FINAL)
    Q_PROPERTY(Reroll::ViewModels::Models::MyListFilterModel *filteredModel
                   READ filteredModel CONSTANT FINAL)
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged FINAL)

public:
    MyListController(Infrastructure::JsonStore &jsonStore,
                     Infrastructure::PosterUrlResolver &posterUrlResolver,
                     QObject *parent = nullptr);

    [[nodiscard]] ViewModels::Models::TitleListModel *model() noexcept;
    [[nodiscard]] ViewModels::Models::MyListFilterModel *filteredModel() noexcept;
    [[nodiscard]] int revision() const noexcept;

    [[nodiscard]] bool isWatched(const Domain::CandidateIdentity &identity) const noexcept;
    [[nodiscard]] bool isHidden(const Domain::CandidateIdentity &identity) const noexcept;

    Q_INVOKABLE bool isInWatchlist(qlonglong tmdbId, int mediaType) const;
    Q_INVOKABLE bool isMarkedWatched(qlonglong tmdbId, int mediaType) const;
    Q_INVOKABLE int totalCount() const noexcept;

    Q_INVOKABLE void setWatchlist(qlonglong tmdbId,
                                  int mediaType,
                                  const QString &title,
                                  int releaseYear,
                                  const QVariantList &genreIds,
                                  const QString &posterPath,
                                  double rating,
                                  qlonglong voteCount,
                                  bool watchlist);
    Q_INVOKABLE void setWatched(qlonglong tmdbId,
                                int mediaType,
                                const QString &title,
                                int releaseYear,
                                const QVariantList &genreIds,
                                const QString &posterPath,
                                double rating,
                                qlonglong voteCount,
                                bool watched);
    Q_INVOKABLE void setHidden(qlonglong tmdbId,
                               int mediaType,
                               const QString &title,
                               int releaseYear,
                               const QVariantList &genreIds,
                               const QString &posterPath,
                               double rating,
                               qlonglong voteCount,
                               bool hidden);

signals:
    void watchedChanged();
    void revisionChanged();

private:
    [[nodiscard]] Domain::TitleSnapshot buildSnapshot(qlonglong tmdbId,
                                                       int mediaType,
                                                       const QString &title,
                                                       int releaseYear,
                                                       const QVariantList &genreIds,
                                                       const QString &posterPath,
                                                       double rating,
                                                       qlonglong voteCount) const;
    void persist();
    void bumpRevision();

    Infrastructure::JsonStore &m_jsonStore;
    Infrastructure::PosterUrlResolver &m_posterUrlResolver;
    ViewModels::Models::TitleListModel m_model;
    ViewModels::Models::MyListFilterModel m_filterModel;
    int m_revision{0};
};

}
