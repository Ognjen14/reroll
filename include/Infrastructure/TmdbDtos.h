#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Reroll::Infrastructure
{

enum class
    TmdbMediaType : std::uint8_t
{
    Movie,
    Tv
};

enum class TmdbGenreMatchMode : std::uint8_t
{
    Or,
    And
};

using TmdbId = std::int64_t;
using TmdbGenreId = std::int32_t;
using TmdbPageNumber = std::uint32_t;
using TmdbResultCount = std::uint64_t;

struct TmdbGenreDto final
{
    TmdbGenreId id{0};
    std::string name;
};

struct TmdbDiscoverRequestDto final
{
    TmdbMediaType mediaType{TmdbMediaType::Movie};
    TmdbPageNumber page{1};
    std::optional<std::string> language;
    std::optional<int> minimumYear;
    std::optional<int> maximumYear;
    double minimumRating{0.0};
    std::vector<TmdbGenreId> genreIds;
    TmdbGenreMatchMode genreMatchMode{TmdbGenreMatchMode::Or};
    std::optional<std::string> originalLanguage{"en"};
};

struct TmdbConfigurationRequestDto final
{
};

struct TmdbGenreListRequestDto final
{
    TmdbMediaType mediaType{TmdbMediaType::Movie};
    std::optional<std::string> language;
};

struct TmdbTrendingRequestDto final
{
    TmdbMediaType mediaType{TmdbMediaType::Movie};
    TmdbPageNumber page{1};
};

struct TmdbPopularRequestDto final
{
    TmdbMediaType mediaType{TmdbMediaType::Movie};
    TmdbPageNumber page{1};
};

struct TmdbSearchRequestDto final
{
    TmdbMediaType mediaType{TmdbMediaType::Movie};
    std::string query;
    TmdbPageNumber page{1};
};

struct TmdbVideosRequestDto final
{
    TmdbMediaType mediaType{TmdbMediaType::Movie};
    TmdbId id{0};
    std::optional<std::string> language;
};

struct TmdbVideoDto final
{
    std::string key;
    std::string site;
    std::string type;
    bool official{false};
};

struct TmdbVideosResponseDto final
{
    std::vector<TmdbVideoDto> results;
};

struct TmdbWatchProvidersRequestDto final
{
    TmdbMediaType mediaType{TmdbMediaType::Movie};
    TmdbId id{0};
};

struct TmdbWatchProviderDto final
{
    TmdbId providerId{0};
    std::string providerName;
    std::string logoPath;
};

struct TmdbWatchProvidersResponseDto final
{
    std::vector<TmdbWatchProviderDto> providers;
};

struct TmdbTitleDetailsRequestDto final
{
    TmdbMediaType mediaType{TmdbMediaType::Movie};
    TmdbId id{0};
};

struct TmdbTitleDetailsDto final
{
    TmdbId id{0};
    TmdbMediaType mediaType{TmdbMediaType::Movie};
    std::string title;
    std::optional<std::string> releaseDate;
    std::vector<std::string> genreNames;
    double rating{0.0};
    std::string overview;
    std::optional<std::string> posterPath;
};

struct TmdbTitleResultDto final
{
    TmdbId id{0};
    TmdbMediaType mediaType{TmdbMediaType::Movie};
    std::string title;
    std::optional<std::string> releaseDate;
    std::vector<TmdbGenreId> genreIds;
    double rating{0.0};
    std::int64_t voteCount{0};
    double popularity{0.0};
    std::string overview;
    std::optional<std::string> posterPath;
};

struct TmdbDiscoverPageDto final
{
    std::vector<TmdbTitleResultDto> results;
    TmdbPageNumber page{1};
    TmdbPageNumber totalPages{1};
    TmdbResultCount totalResults{0};
};

struct TmdbImageConfigurationDto final
{
    std::string baseUrl;
    std::string secureBaseUrl;
    std::vector<std::string> posterSizes;
};

struct TmdbConfigurationResponseDto final
{
    TmdbImageConfigurationDto images;
};

struct TmdbGenreListResponseDto final
{
    std::vector<TmdbGenreDto> genres;
};

}
