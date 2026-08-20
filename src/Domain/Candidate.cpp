#include "Domain/Candidate.h"

#include <utility>

namespace Reroll::Domain
{

Candidate::Candidate(Id tmdbId,
                     MediaType mediaType,
                     std::string title,
                     int releaseYear,
                     std::vector<GenreId> genreIds,
                     double rating,
                     std::int64_t voteCount,
                     double popularity,
                     std::string overview,
                     std::string posterPath)
    : m_identity(mediaType, tmdbId)
    , m_title(std::move(title))
    , m_releaseYear(releaseYear)
    , m_genreIds(std::move(genreIds))
    , m_rating(rating)
    , m_voteCount(voteCount)
    , m_popularity(popularity)
    , m_overview(std::move(overview))
    , m_posterPath(std::move(posterPath))
{
}

const CandidateIdentity &Candidate::identity() const noexcept
{
    return m_identity;
}

Candidate::Id Candidate::tmdbId() const noexcept
{
    return m_identity.tmdbId();
}

MediaType Candidate::mediaType() const noexcept
{
    return m_identity.mediaType();
}

const std::string &Candidate::title() const noexcept
{
    return m_title;
}

int Candidate::releaseYear() const noexcept
{
    return m_releaseYear;
}

const std::vector<Candidate::GenreId> &Candidate::genreIds() const noexcept
{
    return m_genreIds;
}

double Candidate::rating() const noexcept
{
    return m_rating;
}

std::int64_t Candidate::voteCount() const noexcept
{
    return m_voteCount;
}

double Candidate::popularity() const noexcept
{
    return m_popularity;
}

const std::string &Candidate::overview() const noexcept
{
    return m_overview;
}

const std::string &Candidate::posterPath() const noexcept
{
    return m_posterPath;
}

}
