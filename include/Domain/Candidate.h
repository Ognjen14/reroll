#pragma once

#include "CandidateIdentity.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Reroll::Domain
{

class Candidate final
{
public:
    using Id = CandidateIdentity::TmdbId;
    using GenreId = std::int32_t;

    Candidate(Id tmdbId,
              MediaType mediaType,
              std::string title,
              int releaseYear,
              std::vector<GenreId> genreIds,
              double rating,
              std::int64_t voteCount,
              double popularity,
              std::string overview,
              std::string posterPath);

    [[nodiscard]] const CandidateIdentity &identity() const noexcept;
    [[nodiscard]] Id tmdbId() const noexcept;
    [[nodiscard]] MediaType mediaType() const noexcept;
    [[nodiscard]] const std::string &title() const noexcept;
    [[nodiscard]] int releaseYear() const noexcept;
    [[nodiscard]] const std::vector<GenreId> &genreIds() const noexcept;
    [[nodiscard]] double rating() const noexcept;
    [[nodiscard]] std::int64_t voteCount() const noexcept;
    [[nodiscard]] double popularity() const noexcept;
    [[nodiscard]] const std::string &overview() const noexcept;
    [[nodiscard]] const std::string &posterPath() const noexcept;

private:
    CandidateIdentity m_identity;
    std::string m_title;
    int m_releaseYear;
    std::vector<GenreId> m_genreIds;
    double m_rating;
    std::int64_t m_voteCount;
    double m_popularity;
    std::string m_overview;
    std::string m_posterPath;
};

}
