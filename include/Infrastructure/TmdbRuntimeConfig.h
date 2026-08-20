#pragma once

#include <QString>
#include <QUrl>

namespace Reroll::Infrastructure
{

struct TmdbRequestAccess final
{
    QUrl baseUrl;
    QString apiKey;
};

class TmdbRuntimeConfig
{
public:
    virtual ~TmdbRuntimeConfig();

    [[nodiscard]] virtual TmdbRequestAccess requestAccess() const = 0;
};

}
