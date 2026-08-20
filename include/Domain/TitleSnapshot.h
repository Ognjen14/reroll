#pragma once

#include "Domain/CandidateIdentity.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Reroll::Domain
{

class Candidate;

class TitleSnapshot final
{
public:
    using Id = CandidateIdentity::TmdbId;
    using GenreId = std::int32_t;
    using GenreIds = std::vector<GenreId>;
    using VoteCount = std::int64_t;

    TitleSnapshot(Id tmdbId,
                  MediaType mediaType,
                  std::string title,
                  int releaseYear,
                  GenreIds genreIds,
                  std::string posterPath,
                  double rating,
                  VoteCount voteCount);

    [[nodiscard]] static TitleSnapshot fromCandidate(const Candidate &candidate);

    [[nodiscard]] const CandidateIdentity &identity() const noexcept;
    [[nodiscard]] Id tmdbId() const noexcept;
    [[nodiscard]] MediaType mediaType() const noexcept;
    [[nodiscard]] const std::string &title() const noexcept;
    [[nodiscard]] int releaseYear() const noexcept;
    [[nodiscard]] const GenreIds &genreIds() const noexcept;
    [[nodiscard]] const std::string &posterPath() const noexcept;
    [[nodiscard]] double rating() const noexcept;
    [[nodiscard]] VoteCount voteCount() const noexcept;

    friend bool operator==(const TitleSnapshot &left,
                           const TitleSnapshot &right) noexcept;
    friend bool operator!=(const TitleSnapshot &left,
                           const TitleSnapshot &right) noexcept;

private:
    CandidateIdentity m_identity;
    std::string m_title;
    int m_releaseYear;
    GenreIds m_genreIds;
    std::string m_posterPath;
    double m_rating;
    VoteCount m_voteCount;
};

}
