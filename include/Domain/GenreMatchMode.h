#pragma once

#include <cstdint>

namespace Reroll::Domain
{

enum class GenreMatchMode : std::uint8_t
{
    Or,
    And
};

}
