#include "Domain/TitleSnapshot.h"

#include "Domain/Candidate.h"

#include <utility>

namespace Reroll::Domain
{

TitleSnapshot::TitleSnapshot(Id tmdbId,
                             MediaType mediaType,
                             std::string title,
                             int releaseYear,
                             GenreIds genreIds,
                             std::string posterPath,
                             double rating,
                             VoteCount voteCount)
    : m_identity(mediaType, tmdbId)
    , m_title(std::move(title))
    , m_releaseYear(releaseYear)
    , m_genreIds(std::move(genreIds))
    , m_posterPath(std::move(posterPath))
    , m_rating(rating)
    , m_voteCount(voteCount)
{
}

TitleSnapshot TitleSnapshot::fromCandidate(const Candidate &candidate)
{
    return TitleSnapshot(candidate.tmdbId(),
                         candidate.mediaType(),
                         candidate.title(),
                         candidate.releaseYear(),
                         candidate.genreIds(),
                         candidate.posterPath(),
                         candidate.rating(),
                         candidate.voteCount());
}

const CandidateIdentity &TitleSnapshot::identity() const noexcept
{
    return m_identity;
}

TitleSnapshot::Id TitleSnapshot::tmdbId() const noexcept
{
    return m_identity.tmdbId();
}

MediaType TitleSnapshot::mediaType() const noexcept
{
    return m_identity.mediaType();
}

const std::string &TitleSnapshot::title() const noexcept
{
    return m_title;
}

int TitleSnapshot::releaseYear() const noexcept
{
    return m_releaseYear;
}

const TitleSnapshot::GenreIds &TitleSnapshot::genreIds() const noexcept
{
    return m_genreIds;
}

const std::string &TitleSnapshot::posterPath() const noexcept
{
    return m_posterPath;
}

double TitleSnapshot::rating() const noexcept
{
    return m_rating;
}

TitleSnapshot::VoteCount TitleSnapshot::voteCount() const noexcept
{
    return m_voteCount;
}

bool operator==(const TitleSnapshot &left,
                const TitleSnapshot &right) noexcept
{
    return left.m_identity == right.m_identity
        && left.m_title == right.m_title
        && left.m_releaseYear == right.m_releaseYear
        && left.m_genreIds == right.m_genreIds
        && left.m_posterPath == right.m_posterPath
        && left.m_rating == right.m_rating
        && left.m_voteCount == right.m_voteCount;
}

bool operator!=(const TitleSnapshot &left,
                const TitleSnapshot &right) noexcept
{
    return !(left == right);
}

}
