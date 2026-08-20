#include "Infrastructure/ImageNetworkFactory.h"

#include "RRLog.h"

#include <QNetworkAccessManager>
#include <QNetworkDiskCache>

#include <stdexcept>
#include <utility>

namespace Reroll::Infrastructure
{

ImageNetworkFactory::ImageNetworkFactory(QString cacheDirectory)
    : m_cacheDirectory(std::move(cacheDirectory))
{
    if (m_cacheDirectory.isEmpty())
    {
        throw std::invalid_argument("Image cache directory must not be empty");
    }
}

QNetworkAccessManager *ImageNetworkFactory::create(QObject *parent)
{
    RR_LOG_D() << "Poster network access manager created" << m_cacheDirectory;

    auto *networkAccessManager = new QNetworkAccessManager(parent);
    auto *diskCache = new QNetworkDiskCache(networkAccessManager);
    diskCache->setCacheDirectory(m_cacheDirectory);
    diskCache->setMaximumCacheSize(MaximumPosterCacheBytes);
    networkAccessManager->setCache(diskCache);
    return networkAccessManager;
}

}
