#include "Domain/FilterCriteria.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace
{

double normalizeMinimumRating(double rating) noexcept
{
    if (std::isnan(rating))
    {
        return Reroll::Domain::FilterCriteria::MinimumAllowedRating;
    }

    return std::clamp(rating,
                      Reroll::Domain::FilterCriteria::MinimumAllowedRating,
                      Reroll::Domain::FilterCriteria::MaximumAllowedRating);
}

Reroll::Domain::FilterCriteria::GenreIds normalizeGenreIds(
    Reroll::Domain::FilterCriteria::GenreIds genreIds)
{
    std::sort(genreIds.begin(), genreIds.end());
    genreIds.erase(std::unique(genreIds.begin(), genreIds.end()), genreIds.end());
    return genreIds;
}

}

namespace Reroll::Domain
{

FilterCriteria::FilterCriteria(MediaTypeFilter mediaType,
                               std::optional<Year> minimumYear,
                               std::optional<Year> maximumYear,
                               double minimumRating,
                               GenreIds genreIds,
                               GenreMatchMode genreMatchMode,
                               bool excludeWatched,
                               std::string originalLanguage)
    : m_mediaType(mediaType)
    , m_minimumYear(minimumYear)
    , m_maximumYear(maximumYear)
    , m_minimumRating(normalizeMinimumRating(minimumRating))
    , m_genreIds(normalizeGenreIds(std::move(genreIds)))
    , m_genreMatchMode(genreMatchMode)
    , m_excludeWatched(excludeWatched)
    , m_originalLanguage(originalLanguage.empty() ? DefaultOriginalLanguage
                                                   : std::move(originalLanguage))
{
    if (m_minimumYear.has_value()
        && m_maximumYear.has_value()
        && *m_minimumYear > *m_maximumYear)
    {
        std::swap(m_minimumYear, m_maximumYear);
    }
}

MediaTypeFilter FilterCriteria::mediaType() const noexcept
{
    return m_mediaType;
}

const std::optional<FilterCriteria::Year> &FilterCriteria::minimumYear() const noexcept
{
    return m_minimumYear;
}

const std::optional<FilterCriteria::Year> &FilterCriteria::maximumYear() const noexcept
{
    return m_maximumYear;
}

double FilterCriteria::minimumRating() const noexcept
{
    return m_minimumRating;
}

const FilterCriteria::GenreIds &FilterCriteria::genreIds() const noexcept
{
    return m_genreIds;
}

GenreMatchMode FilterCriteria::genreMatchMode() const noexcept
{
    return m_genreMatchMode;
}

bool FilterCriteria::excludeWatched() const noexcept
{
    return m_excludeWatched;
}

const std::string &FilterCriteria::originalLanguage() const noexcept
{
    return m_originalLanguage;
}

bool operator==(const FilterCriteria &left, const FilterCriteria &right) noexcept
{
    return left.m_mediaType == right.m_mediaType
        && left.m_minimumYear == right.m_minimumYear
        && left.m_maximumYear == right.m_maximumYear
        && left.m_minimumRating == right.m_minimumRating
        && left.m_genreIds == right.m_genreIds
        && left.m_genreMatchMode == right.m_genreMatchMode
        && left.m_excludeWatched == right.m_excludeWatched
        && left.m_originalLanguage == right.m_originalLanguage;
}

bool operator!=(const FilterCriteria &left, const FilterCriteria &right) noexcept
{
    return !(left == right);
}

}
