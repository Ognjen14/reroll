#pragma once

#include "TmdbDtos.h"

#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>

#include <string>

namespace Reroll::Infrastructure
{

class TmdbClient;

class PosterUrlResolver final : public QObject
{
    Q_OBJECT

public:
    explicit PosterUrlResolver(TmdbClient &tmdbClient, QObject *parent = nullptr);

    [[nodiscard]] bool isReady() const noexcept;
    [[nodiscard]] QUrl resolve(const std::string &posterPath, int requestedPixelWidth) const;
    [[nodiscard]] Q_INVOKABLE QString resolveUrl(const QString &posterPath,
                                                 int requestedPixelWidth) const;

signals:
    void configurationLoaded();

private:
    void fetchConfiguration();

    TmdbClient &m_tmdbClient;
    TmdbImageConfigurationDto m_configuration;
    bool m_loaded{false};
    QTimer m_retryTimer;
};

}
