#include "Infrastructure/TmdbMapper.h"

#include "Domain/MediaType.h"

#include <QString>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <optional>
#include <string>
#include <system_error>

namespace Reroll::Infrastructure
{
namespace
{

Domain::MediaType domainMediaType(TmdbMediaType mediaType) noexcept
{
    switch (mediaType)
    {
    case TmdbMediaType::Movie:
        return Domain::MediaType::Movie;
    case TmdbMediaType::Tv:
        return Domain::MediaType::Tv;
    }

    return Domain::MediaType::Movie;
}

int releaseYear(const std::optional<std::string> &releaseDate) noexcept
{
    if (!releaseDate.has_value() || releaseDate->size() < 4)
    {
        return 0;
    }

    int year = 0;
    const char *first = releaseDate->data();
    const char *last = first + 4;
    const auto result = std::from_chars(first, last, year);
    if (result.ec != std::errc{} || result.ptr != last || year <= 0)
    {
        return 0;
    }

    return year;
}

TmdbError invalidCandidateError()
{
    return TmdbError(TmdbError::Category::Parse,
                     "TMDB returned a title that the app could not use.");
}

bool hasValidGenreIds(const std::vector<TmdbGenreId> &genreIds) noexcept
{
    return std::all_of(genreIds.cbegin(), genreIds.cend(), [](TmdbGenreId id) {
        return id > 0;
    });
}

struct PosterSize final
{
    int width;
    const char *name;
};

constexpr std::array<PosterSize, 4> SupportedPosterSizes{{
    {92, "w92"},
    {154, "w154"},
    {185, "w185"},
    {342, "w342"}
}};

bool hasPosterSize(const TmdbImageConfigurationDto &configuration,
                   const char *size)
{
    return std::find(configuration.posterSizes.cbegin(),
                     configuration.posterSizes.cend(),
                     size)
        != configuration.posterSizes.cend();
}

}

TmdbMapper::CandidateResult TmdbMapper::toCandidate(const TmdbTitleResultDto &dto)
{
    if (dto.id <= 0
        || dto.title.empty()
        || !std::isfinite(dto.rating)
        || dto.rating < 0.0
        || dto.rating > 10.0
        || dto.voteCount < 0
        || !std::isfinite(dto.popularity)
        || dto.popularity < 0.0
        || !hasValidGenreIds(dto.genreIds))
    {
        return invalidCandidateError();
    }

    return Domain::Candidate(dto.id,
                             domainMediaType(dto.mediaType),
                             dto.title,
                             releaseYear(dto.releaseDate),
                             dto.genreIds,
                             dto.rating,
                             dto.voteCount,
                             dto.popularity,
                             dto.overview,
                             dto.posterPath.value_or(std::string{}));
}

std::optional<std::string> TmdbMapper::selectPosterSize(
    const TmdbImageConfigurationDto &configuration,
    int requestedPixelWidth)
{
    const int safeRequestedWidth = std::max(1, requestedPixelWidth);

    for (const PosterSize &size : SupportedPosterSizes)
    {
        if (size.width >= safeRequestedWidth
            && hasPosterSize(configuration, size.name))
        {
            return std::string(size.name);
        }
    }

    for (auto size = SupportedPosterSizes.crbegin();
         size != SupportedPosterSizes.crend();
         ++size)
    {
        if (hasPosterSize(configuration, size->name))
        {
            return std::string(size->name);
        }
    }

    return std::nullopt;
}

QUrl TmdbMapper::posterUrl(
    const TmdbImageConfigurationDto &configuration,
    const std::string &posterPath,
    int requestedPixelWidth)
{
    const auto size = selectPosterSize(configuration, requestedPixelWidth);
    if (!size.has_value()
        || configuration.secureBaseUrl.empty()
        || posterPath.empty())
    {
        return {};
    }

    QUrl url(QString::fromStdString(configuration.secureBaseUrl), QUrl::StrictMode);
    if (!url.isValid() || url.scheme() != QStringLiteral("https"))
    {
        return {};
    }

    QString path = url.path();
    if (!path.endsWith(QLatin1Char('/')))
    {
        path.append(QLatin1Char('/'));
    }

    path.append(QString::fromStdString(*size));
    path.append(QLatin1Char('/'));

    QString relativePosterPath = QString::fromStdString(posterPath);
    while (relativePosterPath.startsWith(QLatin1Char('/')))
    {
        relativePosterPath.remove(0, 1);
    }
    path.append(relativePosterPath);

    url.setPath(path);
    url.setQuery(QString{});
    url.setFragment(QString{});
    return url;
}

}
