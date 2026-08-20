#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <random>
#include <variant>
#include <vector>

namespace Reroll::Domain
{

class WeightedPicker final
{
public:
    using Index = std::size_t;
    using Seed = std::mt19937::result_type;
    using Weights = std::vector<double>;
    using UnitRandomGenerator = std::function<double()>;

    explicit WeightedPicker(UnitRandomGenerator randomGenerator);
    explicit WeightedPicker(Seed seed);

    [[nodiscard]] std::optional<Index> pickIndex(const Weights &weights);

private:
    using RandomSource = std::variant<std::mt19937, UnitRandomGenerator>;

    [[nodiscard]] double nextUnitValue();

    RandomSource m_randomSource;
};

}
