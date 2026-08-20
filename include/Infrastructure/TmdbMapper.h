#pragma once

#include "TmdbDtos.h"
#include "TmdbError.h"

#include "Domain/Candidate.h"

#include <QUrl>

#include <optional>
#include <string>
#include <variant>

namespace Reroll::Infrastructure
{

class TmdbMapper final
{
public:
    using CandidateResult = std::variant<Domain::Candidate, TmdbError>;

    [[nodiscard]] static CandidateResult toCandidate(
        const TmdbTitleResultDto &dto);

    [[nodiscard]] static std::optional<std::string> selectPosterSize(
        const TmdbImageConfigurationDto &configuration,
        int requestedPixelWidth);
    [[nodiscard]] static QUrl posterUrl(
        const TmdbImageConfigurationDto &configuration,
        const std::string &posterPath,
        int requestedPixelWidth);
};

}
