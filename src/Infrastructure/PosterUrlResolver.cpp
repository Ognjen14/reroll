#include "Infrastructure/PosterUrlResolver.h"

#include "Infrastructure/TmdbClient.h"
#include "Infrastructure/TmdbMapper.h"

#include "RRLog.h"

namespace Reroll::Infrastructure
{

PosterUrlResolver::PosterUrlResolver(TmdbClient &tmdbClient, QObject *parent)
    : QObject(parent)
{
    tmdbClient.configuration(
        [this](TmdbClient::ConfigurationResult result) {
            if (auto *configuration = std::get_if<TmdbConfigurationResponseDto>(&result))
            {
                m_configuration = configuration->images;
                m_loaded = true;
                RR_LOG_I() << "PosterUrlResolver loaded TMDB image configuration";
                emit configurationLoaded();
                return;
            }

            RR_LOG_W() << "PosterUrlResolver failed to load TMDB image configuration";
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
