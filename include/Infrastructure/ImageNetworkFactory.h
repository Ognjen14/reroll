#pragma once

#include <QQmlNetworkAccessManagerFactory>
#include <QString>
#include <QtGlobal>

namespace Reroll::Infrastructure
{

class ImageNetworkFactory final : public QQmlNetworkAccessManagerFactory
{
public:
    static constexpr qint64 MaximumPosterCacheBytes = 64LL * 1024LL * 1024LL;

    explicit ImageNetworkFactory(QString cacheDirectory);

    [[nodiscard]] QNetworkAccessManager *create(QObject *parent) override;

private:
    QString m_cacheDirectory;
};

}
