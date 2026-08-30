#include "Infrastructure/RuntimeConfig.h"

#include "Config/ApiKeys.h"
#include "RRLog.h"

#include <QByteArray>

namespace Reroll::Infrastructure
{
namespace
{

QString decodeApiKey()
{
    QByteArray decoded;
    decoded.reserve(static_cast<int>(Config::TmdbApiKeyLength));
    for (std::size_t i = 0; i < Config::TmdbApiKeyLength; ++i)
    {
        const unsigned char maskByte =
            Config::TmdbApiKeyMask[i % sizeof(Config::TmdbApiKeyMask)];
        decoded.append(static_cast<char>(Config::TmdbApiKeyObfuscated[i] ^ maskByte));
    }
    return QString::fromUtf8(decoded);
}

}

RuntimeConfig::RuntimeConfig()
    : m_localDefaults{
          QUrl(QString::fromUtf8(Config::TmdbBaseUrl), QUrl::StrictMode),
          decodeApiKey()}
{
    RR_LOG_I() << "RuntimeConfig initialized with compiled-in TMDB base URL"
               << m_localDefaults.baseUrl.toString();
}

TmdbRequestAccess RuntimeConfig::requestAccess() const
{
    return m_localDefaults;
}

}
