#include "Domain/ScoringConfig.h"

#include <cmath>
#include <stdexcept>

namespace Reroll::Domain
{

ScoringConfig::ScoringConfig(double bayesianPriorMean, VoteCount bayesianMinimumVotes)
    : m_bayesianPriorMean(bayesianPriorMean)
    , m_bayesianMinimumVotes(bayesianMinimumVotes)
{
    if (!std::isfinite(m_bayesianPriorMean)
        || m_bayesianPriorMean < RatingScaleMinimum
        || m_bayesianPriorMean > RatingScaleMaximum)
    {
        throw std::invalid_argument(
            "Bayesian prior mean must be finite and between 0 and 10");
    }
    if (m_bayesianMinimumVotes <= 0)
    {
        throw std::invalid_argument(
            "Bayesian minimum votes must be positive");
    }
}

double ScoringConfig::bayesianPriorMean() const noexcept
{
    return m_bayesianPriorMean;
}

ScoringConfig::VoteCount ScoringConfig::bayesianMinimumVotes() const noexcept
{
    return m_bayesianMinimumVotes;
}

bool operator==(const ScoringConfig &left, const ScoringConfig &right) noexcept
{
    return left.m_bayesianPriorMean == right.m_bayesianPriorMean
        && left.m_bayesianMinimumVotes == right.m_bayesianMinimumVotes;
}

bool operator!=(const ScoringConfig &left, const ScoringConfig &right) noexcept
{
    return !(left == right);
}

}
