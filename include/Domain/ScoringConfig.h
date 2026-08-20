#pragma once

#include <cstdint>

namespace Reroll::Domain
{

class ScoringConfig final
{
public:
    using VoteCount = std::int64_t;

    static constexpr double RatingScaleMinimum{0.0};
    static constexpr double RatingScaleMaximum{10.0};
    static constexpr double DefaultBayesianPriorMean{7.3};
    static constexpr VoteCount DefaultBayesianMinimumVotes{120};

    ScoringConfig(double bayesianPriorMean = DefaultBayesianPriorMean,
                  VoteCount bayesianMinimumVotes = DefaultBayesianMinimumVotes);

    [[nodiscard]] double bayesianPriorMean() const noexcept;
    [[nodiscard]] VoteCount bayesianMinimumVotes() const noexcept;

    friend bool operator==(const ScoringConfig &left,
                           const ScoringConfig &right) noexcept;
    friend bool operator!=(const ScoringConfig &left,
                           const ScoringConfig &right) noexcept;

private:
    double m_bayesianPriorMean;
    VoteCount m_bayesianMinimumVotes;
};

}
