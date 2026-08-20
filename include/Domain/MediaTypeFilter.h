#pragma once

#include <cstdint>

namespace Reroll::Domain
{

enum class MediaTypeFilter : std::uint8_t
{
    Movie,
    Tv,
    Both
};

}
