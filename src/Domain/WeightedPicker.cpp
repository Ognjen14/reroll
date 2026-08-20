#include "Domain/WeightedPicker.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace Reroll::Domain
{

WeightedPicker::WeightedPicker(UnitRandomGenerator randomGenerator)
    : m_randomSource(std::move(randomGenerator))
{
    if (!std::get<UnitRandomGenerator>(m_randomSource))
    {
        throw std::invalid_argument("WeightedPicker requires a random generator");
    }
}

WeightedPicker::WeightedPicker(Seed seed)
    : m_randomSource(std::mt19937(seed))
{
}

std::optional<WeightedPicker::Index> WeightedPicker::pickIndex(const Weights &weights)
{
    if (weights.empty())
    {
        return std::nullopt;
    }

    double totalWeight = 0.0;

    for (const double weight : weights)
    {
        if (!std::isfinite(weight) || weight < 0.0)
        {
            return std::nullopt;
        }

        totalWeight += weight;
    }

    if (!std::isfinite(totalWeight))
    {
        return std::nullopt;
    }

    if (weights.size() == 1)
    {
        return Index{0};
    }

    const double unitValue = nextUnitValue();
    if (!std::isfinite(unitValue) || unitValue < 0.0 || unitValue >= 1.0)
    {
        return std::nullopt;
    }

    if (totalWeight == 0.0)
    {
        const Index uniformIndex =
            static_cast<Index>(unitValue * static_cast<double>(weights.size()));

        return uniformIndex < weights.size() ? uniformIndex : weights.size() - 1;
    }

    const double target = unitValue * totalWeight;
    double cumulativeWeight = 0.0;

    for (Index index = 0; index < weights.size(); ++index)
    {
        cumulativeWeight += weights[index];
        if (target < cumulativeWeight)
        {
            return index;
        }
    }

    return std::nullopt;
}

double WeightedPicker::nextUnitValue()
{
    if (auto *engine = std::get_if<std::mt19937>(&m_randomSource))
    {
        constexpr double engineRange =
            static_cast<double>(std::mt19937::max())
            - static_cast<double>(std::mt19937::min())
            + 1.0;

        return (static_cast<double>((*engine)())
                - static_cast<double>(std::mt19937::min()))
            / engineRange;
    }

    return std::get<UnitRandomGenerator>(m_randomSource)();
}

}
