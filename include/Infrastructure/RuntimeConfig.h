#pragma once

#include "TmdbRuntimeConfig.h"

namespace Reroll::Infrastructure
{

class RuntimeConfig final : public TmdbRuntimeConfig
{
public:
    RuntimeConfig();

    [[nodiscard]] TmdbRequestAccess requestAccess() const override;

private:
    TmdbRequestAccess m_localDefaults;
};

}
