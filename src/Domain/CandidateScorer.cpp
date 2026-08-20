#include "Domain/CandidateScorer.h"

#include <algorithm>
#include <cmath>

namespace Reroll::Domain
{

double CandidateScorer::bayesianQualityScore(
    const Candidate &candidate,
    const ScoringConfig &config) noexcept
{
    const double priorRating = config.bayesianPriorMean();
    const double observedRating = std::isfinite(candidate.rating())
        ? std::clamp(candidate.rating(),
                     ScoringConfig::RatingScaleMinimum,
                     ScoringConfig::RatingScaleMaximum)
        : priorRating;
    const double observedVotes = static_cast<double>(
        std::max<std::int64_t>(candidate.voteCount(), 0));
    const double priorVotes = static_cast<double>(
        config.bayesianMinimumVotes());
    const double adjustedRating =
        ((observedVotes * observedRating) + (priorVotes * priorRating))
        / (observedVotes + priorVotes);
    return adjustedRating / ScoringConfig::RatingScaleMaximum;
}

}
