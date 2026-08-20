#include "Infrastructure/RuntimeConfig.h"

#include "Config/ApiKeys.h"
#include "RRLog.h"

namespace Reroll::Infrastructure
{

RuntimeConfig::RuntimeConfig()
    : m_localDefaults{
          QUrl(QString::fromUtf8(Config::TmdbBaseUrl), QUrl::StrictMode),
          QString::fromUtf8(Config::TmdbApiKey)}
{
    RR_LOG_I() << "RuntimeConfig initialized with compiled-in TMDB base URL"
               << m_localDefaults.baseUrl.toString();
}

TmdbRequestAccess RuntimeConfig::requestAccess() const
{
    return m_localDefaults;
}

}
