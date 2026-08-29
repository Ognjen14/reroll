#include "Infrastructure/PosterUrlResolver.h"

#include "Infrastructure/TmdbClient.h"
#include "Infrastructure/TmdbMapper.h"

#include "RRLog.h"

namespace Reroll::Infrastructure
{
namespace
{

constexpr int ConfigurationRetryIntervalMs = 5000;

}

PosterUrlResolver::PosterUrlResolver(TmdbClient &tmdbClient, QObject *parent)
    : QObject(parent)
    , m_tmdbClient(tmdbClient)
{
    m_retryTimer.setSingleShot(true);
    m_retryTimer.setInterval(ConfigurationRetryIntervalMs);
    connect(&m_retryTimer, &QTimer::timeout, this, &PosterUrlResolver::fetchConfiguration);

    fetchConfiguration();
}

void PosterUrlResolver::fetchConfiguration()
{
    m_tmdbClient.configuration(
        [this](TmdbClient::ConfigurationResult result) {
            if (auto *configuration = std::get_if<TmdbConfigurationResponseDto>(&result))
            {
                m_configuration = configuration->images;
                m_loaded = true;
                RR_LOG_I() << "PosterUrlResolver loaded TMDB image configuration";
                emit configurationLoaded();
                return;
            }

            RR_LOG_W() << "PosterUrlResolver failed to load TMDB image configuration,"
                          " retrying shortly";
            m_retryTimer.start();
        });
}

bool PosterUrlResolver::isReady() const noexcept
{
    return m_loaded;
}

QUrl PosterUrlResolver::resolve(const std::string &posterPath, int requestedPixelWidth) const
{
    if (!m_loaded || posterPath.empty())
    {
        return {};
    }

    return TmdbMapper::posterUrl(m_configuration, posterPath, requestedPixelWidth);
}

QString PosterUrlResolver::resolveUrl(const QString &posterPath, int requestedPixelWidth) const
{
    const QUrl url = resolve(posterPath.toStdString(), requestedPixelWidth);
    return url.isValid() ? url.toString() : QString();
}

}
