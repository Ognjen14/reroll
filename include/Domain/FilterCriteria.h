#pragma once

#include "Domain/GenreMatchMode.h"
#include "Domain/MediaTypeFilter.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Reroll::Domain
{

class FilterCriteria final
{
public:
    using Year = int;
    using GenreId = std::int32_t;
    using GenreIds = std::vector<GenreId>;

    static constexpr double MinimumAllowedRating = 0.0;
    static constexpr double MaximumAllowedRating = 10.0;
    static constexpr const char *DefaultOriginalLanguage = "en";

    FilterCriteria() = default;
    FilterCriteria(MediaTypeFilter mediaType,
                   std::optional<Year> minimumYear,
                   std::optional<Year> maximumYear,
                   double minimumRating,
                   GenreIds genreIds,
                   GenreMatchMode genreMatchMode,
                   bool excludeWatched,
                   std::string originalLanguage = DefaultOriginalLanguage);

    [[nodiscard]] MediaTypeFilter mediaType() const noexcept;
    [[nodiscard]] const std::optional<Year> &minimumYear() const noexcept;
    [[nodiscard]] const std::optional<Year> &maximumYear() const noexcept;
    [[nodiscard]] double minimumRating() const noexcept;
    [[nodiscard]] const GenreIds &genreIds() const noexcept;
    [[nodiscard]] GenreMatchMode genreMatchMode() const noexcept;
    [[nodiscard]] bool excludeWatched() const noexcept;
    [[nodiscard]] const std::string &originalLanguage() const noexcept;

    friend bool operator==(const FilterCriteria &left,
                           const FilterCriteria &right) noexcept;
    friend bool operator!=(const FilterCriteria &left,
                           const FilterCriteria &right) noexcept;

private:
    MediaTypeFilter m_mediaType{MediaTypeFilter::Movie};
    std::optional<Year> m_minimumYear;
    std::optional<Year> m_maximumYear;
    double m_minimumRating{0.0};
    GenreIds m_genreIds;
    GenreMatchMode m_genreMatchMode{GenreMatchMode::Or};
    bool m_excludeWatched{false};
    std::string m_originalLanguage{DefaultOriginalLanguage};
};

}
