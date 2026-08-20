#pragma once

#include "Domain/Candidate.h"
#include "Domain/ScoringConfig.h"

namespace Reroll::Domain
{

class CandidateScorer final
{
public:
    [[nodiscard]] static double bayesianQualityScore(
        const Candidate &candidate,
        const ScoringConfig &config = ScoringConfig{}) noexcept;
};

}
