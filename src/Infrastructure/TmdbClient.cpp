#include "Infrastructure/TmdbClient.h"

#include "Infrastructure/TmdbJsonReader.h"

#include "RRLog.h"

#include <QByteArray>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVariant>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace Reroll::Infrastructure
{
namespace
{

TmdbError invalidFieldError(const char *fieldName)
{
    return TmdbError(
        TmdbError::Category::Parse,
        std::string("TMDB returned invalid data for the '") + fieldName + "' field.");
}

using GenreParseResult = std::variant<std::vector<TmdbGenreId>, TmdbError>;

GenreParseResult parseGenreIds(const std::optional<QJsonArray> &array)
{
    std::vector<TmdbGenreId> genreIds;
    if (!array.has_value())
    {
        return genreIds;
    }

    genreIds.reserve(static_cast<std::size_t>(array->size()));
    for (const QJsonValue &value : *array)
    {
        if (!value.isDouble())
        {
            return invalidFieldError("genre_ids");
        }

        const double rawId = value.toDouble();
        const long double wideId = static_cast<long double>(rawId);
        const long double minimum = static_cast<long double>(
            std::numeric_limits<TmdbGenreId>::lowest());
        const long double maximum = static_cast<long double>(
            std::numeric_limits<TmdbGenreId>::max());

        if (!std::isfinite(rawId)
            || std::trunc(wideId) != wideId
            || wideId < minimum
            || wideId > maximum)
        {
            return invalidFieldError("genre_ids");
        }

        genreIds.push_back(static_cast<TmdbGenreId>(rawId));
    }

    return genreIds;
}

std::optional<TmdbPageNumber> pageNumber(std::int64_t value, bool allowZero)
{
    const std::int64_t minimum = allowZero ? 0 : 1;
    if (value < minimum
        || static_cast<std::uint64_t>(value)
               > std::numeric_limits<TmdbPageNumber>::max())
    {
        return std::nullopt;
    }

    return static_cast<TmdbPageNumber>(value);
}

std::optional<TmdbResultCount> resultCount(std::int64_t value)
{
    if (value < 0)
    {
        return std::nullopt;
    }

    return static_cast<TmdbResultCount>(value);
}

std::optional<TmdbError::HttpStatus> httpStatus(const QNetworkReply &reply)
{
    const QVariant statusAttribute = reply.attribute(
        QNetworkRequest::HttpStatusCodeAttribute);
    if (!statusAttribute.isValid())
    {
        return std::nullopt;
    }

    return statusAttribute.toInt();
}

std::optional<TmdbError::TmdbStatusCode> tmdbStatusCode(const QByteArray &body)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        return std::nullopt;
    }

    TmdbJsonReader reader(document.object());
    const auto statusCode = reader.optionalInteger(QStringLiteral("status_code"));
    if (reader.hasError()
        || !statusCode.has_value()
        || *statusCode < std::numeric_limits<TmdbError::TmdbStatusCode>::lowest()
        || *statusCode > std::numeric_limits<TmdbError::TmdbStatusCode>::max())
    {
        return std::nullopt;
    }

    return static_cast<TmdbError::TmdbStatusCode>(*statusCode);
}

std::optional<TmdbError::RetryAfter> retryAfter(const QNetworkReply &reply)
{
    const QByteArray header = reply.rawHeader(QLatin1StringView("Retry-After"));
    if (header.isEmpty())
    {
        return std::nullopt;
    }

    bool secondsOk = false;
    const qint64 seconds = header.trimmed().toLongLong(&secondsOk);
    if (secondsOk && seconds >= 0)
    {
        return TmdbError::RetryAfter(seconds);
    }

    const QDateTime retryDate = QDateTime::fromString(
        QString::fromLatin1(header),
        Qt::RFC2822Date);
    if (!retryDate.isValid())
    {
        return std::nullopt;
    }

    const qint64 secondsUntilRetry = std::max<qint64>(
        0,
        QDateTime::currentDateTimeUtc().secsTo(retryDate.toUTC()));
    return TmdbError::RetryAfter(secondsUntilRetry);
}

TmdbError mappedReplyError(QNetworkReply &reply, const QByteArray &body)
{
    const auto responseStatus = httpStatus(reply);
    const auto tmdbStatus = tmdbStatusCode(body);

    if (responseStatus == 429 || tmdbStatus == 25)
    {
        return TmdbError(TmdbError::Category::RateLimited,
                         "TMDB is temporarily limiting requests. Try again shortly.",
                         responseStatus,
                         tmdbStatus,
                         retryAfter(reply));
    }

    switch (reply.error())
    {
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::ProxyNotFoundError:
        return TmdbError(TmdbError::Category::Dns,
                         "The TMDB server address could not be found.",
                         responseStatus,
                         tmdbStatus);
    case QNetworkReply::TimeoutError:
    case QNetworkReply::ProxyTimeoutError:
        return TmdbError(TmdbError::Category::Timeout,
                         "The TMDB request timed out.",
                         responseStatus,
                         tmdbStatus);
    case QNetworkReply::TemporaryNetworkFailureError:
    case QNetworkReply::NetworkSessionFailedError:
        return TmdbError(TmdbError::Category::Offline,
                         "A network connection is not currently available.",
                         responseStatus,
                         tmdbStatus);
    case QNetworkReply::OperationCanceledError:
        return TmdbError(TmdbError::Category::Cancelled,
                         "The TMDB request was cancelled.",
                         responseStatus,
                         tmdbStatus);
    default:
        break;
    }

    if (tmdbStatus.has_value() && *tmdbStatus != 1)
    {
        return TmdbError(TmdbError::Category::Tmdb,
                         "TMDB rejected the request.",
                         responseStatus,
                         tmdbStatus);
    }

    if (responseStatus.has_value()
        && (*responseStatus < 200 || *responseStatus >= 300))
    {
        return TmdbError(TmdbError::Category::Http,
                         "TMDB could not complete the request.",
                         responseStatus);
    }

    return TmdbError(TmdbError::Category::Network,
                     "The app could not connect to TMDB.",
                     responseStatus,
                     tmdbStatus);
}

TmdbClient::TitleParseResult parseTitleResult(const QJsonObject &object,
                                              TmdbMediaType mediaType,
                                              const QString &titleField,
                                              const QString &dateField)
{
    TmdbJsonReader reader(object);

    const auto id = reader.requiredInteger(QStringLiteral("id"));
    const auto title = reader.requiredString(titleField);
    const auto releaseDate = reader.optionalString(dateField);
    const auto rawGenreIds = reader.optionalArray(QStringLiteral("genre_ids"));
    const auto rating = reader.optionalNumber(QStringLiteral("vote_average"));
    const auto voteCount = reader.optionalInteger(QStringLiteral("vote_count"));
    const auto popularity = reader.optionalNumber(QStringLiteral("popularity"));
    const auto overview = reader.optionalString(QStringLiteral("overview"));
    const auto posterPath = reader.optionalString(QStringLiteral("poster_path"));

    if (reader.hasError())
    {
        return *reader.error();
    }

    GenreParseResult genreResult = parseGenreIds(rawGenreIds);
    if (auto *error = std::get_if<TmdbError>(&genreResult))
    {
        return std::move(*error);
    }

    TmdbTitleResultDto dto;
    dto.id = *id;
    dto.mediaType = mediaType;
    dto.title = *title;
    dto.releaseDate = releaseDate;
    dto.genreIds = std::move(std::get<std::vector<TmdbGenreId>>(genreResult));
    dto.rating = rating.value_or(0.0);
    dto.voteCount = voteCount.value_or(0);
    dto.popularity = popularity.value_or(0.0);
    dto.overview = overview.value_or(std::string{});
    dto.posterPath = posterPath;
    return dto;
}

}

TmdbClient::TmdbClient(QNetworkAccessManager &networkAccessManager,
                       const TmdbRequestBuilder &requestBuilder,
                       QObject *parent)
    : QObject(parent)
    , m_networkAccessManager(networkAccessManager)
    , m_requestBuilder(requestBuilder)
{
}

TmdbClient::~TmdbClient()
{
    discardAllRequests();
}

TmdbClient::RequestId TmdbClient::discoverMovie(
    TmdbDiscoverRequestDto request,
    DiscoverCompletionHandler completionHandler)
{
    request.mediaType = TmdbMediaType::Movie;
    return discover(std::move(request), std::move(completionHandler));
}

TmdbClient::RequestId TmdbClient::discoverTv(
    TmdbDiscoverRequestDto request,
    DiscoverCompletionHandler completionHandler)
{
    request.mediaType = TmdbMediaType::Tv;
    return discover(std::move(request), std::move(completionHandler));
}

TmdbClient::RequestId TmdbClient::configuration(
    ConfigurationCompletionHandler completionHandler)
{
    RR_LOG_D() << "TMDB configuration requested";

    InternalCompletionHandler internalHandler =
        [completionHandler = std::move(completionHandler)](
            InternalResult result) mutable {
        if (!completionHandler)
        {
            return;
        }

        if (auto *configuration = std::get_if<TmdbConfigurationResponseDto>(&result))
        {
            completionHandler(std::move(*configuration));
            return;
        }

        if (auto *error = std::get_if<TmdbError>(&result))
        {
            completionHandler(std::move(*error));
            return;
        }

        completionHandler(TmdbError(
            TmdbError::Category::Parse,
            "TMDB returned an unexpected configuration response."));
    };

    return enqueue(m_requestBuilder.build(TmdbConfigurationRequestDto{}),
                   RequestKind::Configuration,
                   std::move(internalHandler));
}

TmdbClient::RequestId TmdbClient::movieGenres(
    std::optional<std::string> language,
    GenreListCompletionHandler completionHandler)
{
    TmdbGenreListRequestDto request;
    request.mediaType = TmdbMediaType::Movie;
    request.language = std::move(language);
    return genreList(std::move(request), std::move(completionHandler));
}

TmdbClient::RequestId TmdbClient::tvGenres(
    std::optional<std::string> language,
    GenreListCompletionHandler completionHandler)
{
    TmdbGenreListRequestDto request;
    request.mediaType = TmdbMediaType::Tv;
    request.language = std::move(language);
    return genreList(std::move(request), std::move(completionHandler));
}

TmdbClient::RequestId TmdbClient::trendingMovie(
    TmdbPageNumber page,
    DiscoverCompletionHandler completionHandler)
{
    TmdbTrendingRequestDto request;
    request.mediaType = TmdbMediaType::Movie;
    request.page = page;
    RR_LOG_D() << "TMDB trending requested movie page" << page;
    return enqueueDiscoverShapedRequest(m_requestBuilder.build(request),
                                        RequestKind::TrendingMovie,
                                        std::move(completionHandler));
}

TmdbClient::RequestId TmdbClient::trendingTv(
    TmdbPageNumber page,
    DiscoverCompletionHandler completionHandler)
{
    TmdbTrendingRequestDto request;
    request.mediaType = TmdbMediaType::Tv;
    request.page = page;
    RR_LOG_D() << "TMDB trending requested tv page" << page;
    return enqueueDiscoverShapedRequest(m_requestBuilder.build(request),
                                        RequestKind::TrendingTv,
                                        std::move(completionHandler));
}

TmdbClient::RequestId TmdbClient::popularMovie(
    TmdbPageNumber page,
    DiscoverCompletionHandler completionHandler)
{
    TmdbPopularRequestDto request;
    request.mediaType = TmdbMediaType::Movie;
    request.page = page;
    RR_LOG_D() << "TMDB popular requested movie page" << page;
    return enqueueDiscoverShapedRequest(m_requestBuilder.build(request),
                                        RequestKind::PopularMovie,
                                        std::move(completionHandler));
}

TmdbClient::RequestId TmdbClient::popularTv(
    TmdbPageNumber page,
    DiscoverCompletionHandler completionHandler)
{
    TmdbPopularRequestDto request;
    request.mediaType = TmdbMediaType::Tv;
    request.page = page;
    RR_LOG_D() << "TMDB popular requested tv page" << page;
    return enqueueDiscoverShapedRequest(m_requestBuilder.build(request),
                                        RequestKind::PopularTv,
                                        std::move(completionHandler));
}

TmdbClient::RequestId TmdbClient::searchMovie(
    TmdbSearchRequestDto request,
    DiscoverCompletionHandler completionHandler)
{
    request.mediaType = TmdbMediaType::Movie;
    RR_LOG_D() << "TMDB search requested movie" << QString::fromStdString(request.query);
    return enqueueDiscoverShapedRequest(m_requestBuilder.build(request),
                                        RequestKind::SearchMovie,
                                        std::move(completionHandler));
}

TmdbClient::RequestId TmdbClient::searchTv(
    TmdbSearchRequestDto request,
    DiscoverCompletionHandler completionHandler)
{
    request.mediaType = TmdbMediaType::Tv;
    RR_LOG_D() << "TMDB search requested tv" << QString::fromStdString(request.query);
    return enqueueDiscoverShapedRequest(m_requestBuilder.build(request),
                                        RequestKind::SearchTv,
                                        std::move(completionHandler));
}

TmdbClient::RequestId TmdbClient::videos(
    TmdbMediaType mediaType,
    TmdbId id,
    VideosCompletionHandler completionHandler)
{
    TmdbVideosRequestDto request;
    request.mediaType = mediaType;
    request.id = id;

    RR_LOG_D() << "TMDB videos requested"
               << (mediaType == TmdbMediaType::Movie ? "movie" : "tv") << id;

    InternalCompletionHandler internalHandler =
        [completionHandler = std::move(completionHandler)](
            InternalResult result) mutable {
        if (!completionHandler)
        {
            return;
        }

        if (auto *videos = std::get_if<TmdbVideosResponseDto>(&result))
        {
            completionHandler(std::move(*videos));
            return;
        }

        if (auto *error = std::get_if<TmdbError>(&result))
        {
            completionHandler(std::move(*error));
            return;
        }

        completionHandler(TmdbError(
            TmdbError::Category::Parse,
            "TMDB returned an unexpected videos response."));
    };

    return enqueue(m_requestBuilder.build(request),
                   RequestKind::Videos,
                   std::move(internalHandler));
}

TmdbClient::RequestId TmdbClient::watchProviders(
    TmdbMediaType mediaType,
    TmdbId id,
    WatchProvidersCompletionHandler completionHandler)
{
    TmdbWatchProvidersRequestDto request;
    request.mediaType = mediaType;
    request.id = id;

    RR_LOG_D() << "TMDB watch providers requested"
               << (mediaType == TmdbMediaType::Movie ? "movie" : "tv") << id;

    InternalCompletionHandler internalHandler =
        [completionHandler = std::move(completionHandler)](
            InternalResult result) mutable {
        if (!completionHandler)
        {
            return;
        }

        if (auto *providers = std::get_if<TmdbWatchProvidersResponseDto>(&result))
        {
            completionHandler(std::move(*providers));
            return;
        }

        if (auto *error = std::get_if<TmdbError>(&result))
        {
            completionHandler(std::move(*error));
            return;
        }

        completionHandler(TmdbError(
            TmdbError::Category::Parse,
            "TMDB returned an unexpected watch providers response."));
    };

    return enqueue(m_requestBuilder.build(request),
                   RequestKind::WatchProviders,
                   std::move(internalHandler));
}

TmdbClient::RequestId TmdbClient::titleDetails(
    TmdbMediaType mediaType,
    TmdbId id,
    TitleDetailsCompletionHandler completionHandler)
{
    TmdbTitleDetailsRequestDto request;
    request.mediaType = mediaType;
    request.id = id;

    const RequestKind kind = mediaType == TmdbMediaType::Movie
        ? RequestKind::TitleDetailsMovie
        : RequestKind::TitleDetailsTv;

    RR_LOG_D() << "TMDB title details requested"
               << (mediaType == TmdbMediaType::Movie ? "movie" : "tv") << id;

    InternalCompletionHandler internalHandler =
        [completionHandler = std::move(completionHandler)](
            InternalResult result) mutable {
        if (!completionHandler)
        {
            return;
        }

        if (auto *details = std::get_if<TmdbTitleDetailsDto>(&result))
        {
            completionHandler(std::move(*details));
            return;
        }

        if (auto *error = std::get_if<TmdbError>(&result))
        {
            completionHandler(std::move(*error));
            return;
        }

        completionHandler(TmdbError(
            TmdbError::Category::Parse,
            "TMDB returned an unexpected title details response."));
    };

    return enqueue(m_requestBuilder.build(request),
                   kind,
                   std::move(internalHandler));
}

bool TmdbClient::cancelRequest(RequestId requestId)
{
    const auto pending = std::find_if(
        m_pendingRequests.begin(),
        m_pendingRequests.end(),
        [requestId](const PendingRequest &request) {
            return request.id == requestId;
        });

    if (pending != m_pendingRequests.end())
    {
        RR_LOG_W() << "TMDB request cancelled while pending" << requestId
                   << requestKindLabel(pending->kind);
        InternalCompletionHandler completionHandler =
            std::move(pending->completionHandler);
        m_pendingRequests.erase(pending);
        if (completionHandler)
        {
            completionHandler(cancelledError());
        }
        return true;
    }

    const auto active = m_activeRequests.find(requestId);
    if (active == m_activeRequests.end())
    {
        return false;
    }

    RR_LOG_W() << "TMDB request cancelled while active" << requestId
               << requestKindLabel(active->kind);
    ActiveRequest activeRequest = std::move(active.value());
    m_activeRequests.erase(active);
    if (activeRequest.reply)
    {
        disconnect(activeRequest.reply, nullptr, this, nullptr);
        activeRequest.reply->abort();
        activeRequest.reply->deleteLater();
    }

    startQueuedRequests();
    if (activeRequest.completionHandler)
    {
        activeRequest.completionHandler(cancelledError());
    }
    return true;
}

void TmdbClient::cancelAllRequests()
{
    RR_LOG_W() << "Cancelling all TMDB requests" << m_pendingRequests.size()
               << "pending" << m_activeRequests.size() << "active";

    std::vector<InternalCompletionHandler> completionHandlers;
    completionHandlers.reserve(m_pendingRequests.size()
                               + static_cast<std::size_t>(m_activeRequests.size()));

    for (PendingRequest &pendingRequest : m_pendingRequests)
    {
        completionHandlers.push_back(std::move(pendingRequest.completionHandler));
    }
    m_pendingRequests.clear();

    for (auto active = m_activeRequests.begin();
         active != m_activeRequests.end();
         ++active)
    {
        ActiveRequest &activeRequest = active.value();
        completionHandlers.push_back(std::move(activeRequest.completionHandler));
        if (activeRequest.reply)
        {
            disconnect(activeRequest.reply, nullptr, this, nullptr);
            activeRequest.reply->abort();
            activeRequest.reply->deleteLater();
        }
    }
    m_activeRequests.clear();

    for (InternalCompletionHandler &completionHandler : completionHandlers)
    {
        if (completionHandler)
        {
            completionHandler(cancelledError());
        }
    }
}

std::size_t TmdbClient::activeRequestCount() const noexcept
{
    return static_cast<std::size_t>(m_activeRequests.size());
}

std::size_t TmdbClient::pendingRequestCount() const noexcept
{
    return m_pendingRequests.size();
}

TmdbClient::TitleParseResult TmdbClient::parseMovieResult(const QJsonObject &object)
{
    return parseTitleResult(object,
                            TmdbMediaType::Movie,
                            QStringLiteral("title"),
                            QStringLiteral("release_date"));
}

TmdbClient::TitleParseResult TmdbClient::parseTvResult(const QJsonObject &object)
{
    return parseTitleResult(object,
                            TmdbMediaType::Tv,
                            QStringLiteral("name"),
                            QStringLiteral("first_air_date"));
}

TmdbClient::DiscoverResult TmdbClient::parseDiscoverPage(
    const QJsonObject &object,
    TmdbMediaType mediaType)
{
    TmdbJsonReader reader(object);
    const auto rawPage = reader.requiredInteger(QStringLiteral("page"));
    const auto rawTotalPages = reader.requiredInteger(QStringLiteral("total_pages"));
    const auto rawTotalResults = reader.requiredInteger(QStringLiteral("total_results"));
    const auto rawResults = reader.requiredArray(QStringLiteral("results"));

    if (reader.hasError())
    {
        return *reader.error();
    }

    const auto parsedPage = pageNumber(*rawPage, false);
    const auto parsedTotalPages = pageNumber(*rawTotalPages, true);
    const auto parsedTotalResults = resultCount(*rawTotalResults);
    if (!parsedPage.has_value()
        || !parsedTotalPages.has_value()
        || !parsedTotalResults.has_value())
    {
        return TmdbError(TmdbError::Category::Parse,
                         "TMDB returned invalid page information.");
    }

    TmdbDiscoverPageDto page;
    page.page = *parsedPage;
    page.totalPages = *parsedTotalPages;
    page.totalResults = *parsedTotalResults;
    page.results.reserve(static_cast<std::size_t>(rawResults->size()));

    for (const QJsonValue &value : *rawResults)
    {
        if (!value.isObject())
        {
            return invalidFieldError("results");
        }

        TitleParseResult titleResult = mediaType == TmdbMediaType::Movie
            ? parseMovieResult(value.toObject())
            : parseTvResult(value.toObject());

        if (auto *error = std::get_if<TmdbError>(&titleResult))
        {
            return std::move(*error);
        }

        page.results.push_back(
            std::move(std::get<TmdbTitleResultDto>(titleResult)));
    }

    return page;
}

TmdbClient::ConfigurationResult TmdbClient::parseConfigurationResponse(
    const QJsonObject &object)
{
    TmdbJsonReader rootReader(object);
    const auto imagesObject = rootReader.requiredObject(QStringLiteral("images"));
    if (rootReader.hasError())
    {
        return *rootReader.error();
    }

    TmdbJsonReader imagesReader(*imagesObject);
    const auto baseUrl = imagesReader.requiredString(QStringLiteral("base_url"));
    const auto secureBaseUrl = imagesReader.requiredString(
        QStringLiteral("secure_base_url"));
    const auto rawPosterSizes = imagesReader.requiredArray(
        QStringLiteral("poster_sizes"));
    if (imagesReader.hasError())
    {
        return *imagesReader.error();
    }

    std::vector<std::string> posterSizes;
    posterSizes.reserve(static_cast<std::size_t>(rawPosterSizes->size()));
    for (const QJsonValue &value : *rawPosterSizes)
    {
        if (!value.isString())
        {
            return invalidFieldError("poster_sizes");
        }
        posterSizes.push_back(value.toString().toUtf8().toStdString());
    }

    TmdbConfigurationResponseDto response;
    response.images.baseUrl = *baseUrl;
    response.images.secureBaseUrl = *secureBaseUrl;
    response.images.posterSizes = std::move(posterSizes);
    return response;
}

TmdbClient::GenreListResult TmdbClient::parseGenreListResponse(
    const QJsonObject &object)
{
    TmdbJsonReader rootReader(object);
    const auto rawGenres = rootReader.requiredArray(QStringLiteral("genres"));
    if (rootReader.hasError())
    {
        return *rootReader.error();
    }

    TmdbGenreListResponseDto response;
    response.genres.reserve(static_cast<std::size_t>(rawGenres->size()));
    for (const QJsonValue &value : *rawGenres)
    {
        if (!value.isObject())
        {
            return invalidFieldError("genres");
        }

        TmdbJsonReader genreReader(value.toObject());
        const auto rawId = genreReader.requiredInteger(QStringLiteral("id"));
        const auto name = genreReader.requiredString(QStringLiteral("name"));
        if (genreReader.hasError())
        {
            return *genreReader.error();
        }
        if (*rawId <= 0
            || static_cast<std::uint64_t>(*rawId)
                > static_cast<std::uint64_t>(
                    std::numeric_limits<TmdbGenreId>::max())
            || name->empty())
        {
            return invalidFieldError("genres");
        }

        response.genres.push_back(
            TmdbGenreDto{static_cast<TmdbGenreId>(*rawId), *name});
    }

    return response;
}

TmdbClient::VideosResult TmdbClient::parseVideosResponse(const QJsonObject &object)
{
    TmdbJsonReader rootReader(object);
    const auto rawResults = rootReader.requiredArray(QStringLiteral("results"));
    if (rootReader.hasError())
    {
        return *rootReader.error();
    }

    TmdbVideosResponseDto response;
    response.results.reserve(static_cast<std::size_t>(rawResults->size()));
    for (const QJsonValue &value : *rawResults)
    {
        if (!value.isObject())
        {
            continue;
        }

        TmdbJsonReader videoReader(value.toObject());
        const auto key = videoReader.optionalString(QStringLiteral("key"));
        const auto site = videoReader.optionalString(QStringLiteral("site"));
        const auto type = videoReader.optionalString(QStringLiteral("type"));
        const auto official = videoReader.optionalBoolean(QStringLiteral("official"));
        if (videoReader.hasError() || !key.has_value() || key->empty())
        {
            continue;
        }

        response.results.push_back(TmdbVideoDto{
            *key,
            site.value_or(std::string{}),
            type.value_or(std::string{}),
            official.value_or(false)});
    }

    return response;
}

TmdbClient::WatchProvidersResult TmdbClient::parseWatchProvidersResponse(
    const QJsonObject &object)
{
    // TMDB's watch/providers data is region-keyed with no region-less
    // "global" list. Fixed to a single region (not the user's locale) so
    // the app never does country-detection or localization here.
    static const QString region = QStringLiteral("US");

    TmdbJsonReader rootReader(object);
    const auto resultsObject = rootReader.optionalObject(QStringLiteral("results"));
    if (rootReader.hasError())
    {
        return *rootReader.error();
    }

    TmdbWatchProvidersResponseDto response;
    if (!resultsObject.has_value())
    {
        return response;
    }

    TmdbJsonReader resultsReader(*resultsObject);
    const auto regionObject = resultsReader.optionalObject(region);
    if (resultsReader.hasError())
    {
        return *resultsReader.error();
    }
    if (!regionObject.has_value())
    {
        return response;
    }

    TmdbJsonReader regionReader(*regionObject);
    const auto rawFlatrate = regionReader.optionalArray(QStringLiteral("flatrate"));
    if (regionReader.hasError())
    {
        return *regionReader.error();
    }
    if (!rawFlatrate.has_value())
    {
        return response;
    }

    response.providers.reserve(static_cast<std::size_t>(rawFlatrate->size()));
    for (const QJsonValue &value : *rawFlatrate)
    {
        if (!value.isObject())
        {
            continue;
        }

        TmdbJsonReader providerReader(value.toObject());
        const auto providerId = providerReader.optionalInteger(
            QStringLiteral("provider_id"));
        const auto providerName = providerReader.optionalString(
            QStringLiteral("provider_name"));
        const auto logoPath = providerReader.optionalString(
            QStringLiteral("logo_path"));
        if (providerReader.hasError()
            || !providerName.has_value()
            || providerName->empty())
        {
            continue;
        }

        response.providers.push_back(TmdbWatchProviderDto{
            providerId.value_or(0),
            *providerName,
            logoPath.value_or(std::string{})});
    }

    return response;
}

namespace
{

TmdbClient::TitleDetailsResult parseTitleDetails(const QJsonObject &object,
                                                 TmdbMediaType mediaType,
                                                 const QString &titleField,
                                                 const QString &dateField)
{
    TmdbJsonReader reader(object);

    const auto id = reader.requiredInteger(QStringLiteral("id"));
    const auto title = reader.requiredString(titleField);
    const auto releaseDate = reader.optionalString(dateField);
    const auto rawGenres = reader.optionalArray(QStringLiteral("genres"));
    const auto rating = reader.optionalNumber(QStringLiteral("vote_average"));
    const auto overview = reader.optionalString(QStringLiteral("overview"));
    const auto posterPath = reader.optionalString(QStringLiteral("poster_path"));

    if (reader.hasError())
    {
        return *reader.error();
    }

    std::vector<std::string> genreNames;
    if (rawGenres.has_value())
    {
        genreNames.reserve(static_cast<std::size_t>(rawGenres->size()));
        for (const QJsonValue &value : *rawGenres)
        {
            if (!value.isObject())
            {
                continue;
            }

            TmdbJsonReader genreReader(value.toObject());
            const auto name = genreReader.optionalString(QStringLiteral("name"));
            if (genreReader.hasError() || !name.has_value() || name->empty())
            {
                continue;
            }

            genreNames.push_back(*name);
        }
    }

    TmdbTitleDetailsDto dto;
    dto.id = *id;
    dto.mediaType = mediaType;
    dto.title = *title;
    dto.releaseDate = releaseDate;
    dto.genreNames = std::move(genreNames);
    dto.rating = rating.value_or(0.0);
    dto.overview = overview.value_or(std::string{});
    dto.posterPath = posterPath;
    return dto;
}

}

TmdbClient::TitleDetailsResult TmdbClient::parseMovieDetailsResponse(
    const QJsonObject &object)
{
    return parseTitleDetails(object,
                             TmdbMediaType::Movie,
                             QStringLiteral("title"),
                             QStringLiteral("release_date"));
}

TmdbClient::TitleDetailsResult TmdbClient::parseTvDetailsResponse(
    const QJsonObject &object)
{
    return parseTitleDetails(object,
                             TmdbMediaType::Tv,
                             QStringLiteral("name"),
                             QStringLiteral("first_air_date"));
}

TmdbClient::RequestId TmdbClient::discover(
    TmdbDiscoverRequestDto request,
    DiscoverCompletionHandler completionHandler)
{
    const RequestKind kind = request.mediaType == TmdbMediaType::Movie
        ? RequestKind::DiscoverMovie
        : RequestKind::DiscoverTv;

    RR_LOG_D() << "TMDB discover requested" << requestKindLabel(kind)
               << "page" << request.page
               << "genres" << request.genreIds.size()
               << "minRating" << request.minimumRating;

    return enqueueDiscoverShapedRequest(m_requestBuilder.build(request),
                                        kind,
                                        std::move(completionHandler));
}

TmdbClient::RequestId TmdbClient::enqueueDiscoverShapedRequest(
    QNetworkRequest networkRequest,
    RequestKind kind,
    DiscoverCompletionHandler completionHandler)
{
    InternalCompletionHandler internalHandler =
        [completionHandler = std::move(completionHandler)](
            InternalResult result) mutable {
        if (!completionHandler)
        {
            return;
        }

        if (auto *page = std::get_if<TmdbDiscoverPageDto>(&result))
        {
            completionHandler(std::move(*page));
            return;
        }

        if (auto *error = std::get_if<TmdbError>(&result))
        {
            completionHandler(std::move(*error));
            return;
        }

        completionHandler(TmdbError(
            TmdbError::Category::Parse,
            "TMDB returned an unexpected discover-shaped response."));
    };

    return enqueue(std::move(networkRequest),
                   kind,
                   std::move(internalHandler));
}

TmdbClient::RequestId TmdbClient::genreList(
    TmdbGenreListRequestDto request,
    GenreListCompletionHandler completionHandler)
{
    const RequestKind kind = request.mediaType == TmdbMediaType::Movie
        ? RequestKind::GenreListMovie
        : RequestKind::GenreListTv;

    RR_LOG_D() << "TMDB genre list requested" << requestKindLabel(kind);

    InternalCompletionHandler internalHandler =
        [completionHandler = std::move(completionHandler)](
            InternalResult result) mutable {
        if (!completionHandler)
        {
            return;
        }

        if (auto *genres = std::get_if<TmdbGenreListResponseDto>(&result))
        {
            completionHandler(std::move(*genres));
            return;
        }

        if (auto *error = std::get_if<TmdbError>(&result))
        {
            completionHandler(std::move(*error));
            return;
        }

        completionHandler(TmdbError(
            TmdbError::Category::Parse,
            "TMDB returned an unexpected genre-list response."));
    };

    return enqueue(m_requestBuilder.build(request),
                   kind,
                   std::move(internalHandler));
}

TmdbClient::RequestId TmdbClient::enqueue(
    QNetworkRequest networkRequest,
    RequestKind kind,
    InternalCompletionHandler completionHandler)
{
    ++m_nextRequestId;
    if (m_nextRequestId == 0)
    {
        ++m_nextRequestId;
    }

    const RequestId requestId = m_nextRequestId;
    RR_LOG_D() << "TMDB request enqueued" << requestId << requestKindLabel(kind)
               << "active" << activeRequestCount()
               << "pending" << m_pendingRequests.size();
    m_pendingRequests.push_back(
        PendingRequest{requestId,
                       std::move(networkRequest),
                       kind,
                       std::move(completionHandler)});
    startQueuedRequests();
    return requestId;
}

void TmdbClient::startQueuedRequests()
{
    while (!m_pendingRequests.empty()
           && activeRequestCount() < MaximumConcurrentRequests)
    {
        PendingRequest pendingRequest = std::move(m_pendingRequests.front());
        m_pendingRequests.pop_front();
        startRequest(std::move(pendingRequest));
    }
}

void TmdbClient::startRequest(PendingRequest pendingRequest)
{
    QNetworkReply *reply = m_networkAccessManager.get(
        pendingRequest.networkRequest);
    const RequestId requestId = pendingRequest.id;

    RR_LOG_D() << "TMDB request started" << requestId
               << requestKindLabel(pendingRequest.kind);

    m_activeRequests.insert(
        requestId,
        ActiveRequest{reply,
                      pendingRequest.kind,
                      std::move(pendingRequest.completionHandler)});

    connect(reply, &QObject::destroyed, this, [this, requestId]() {
        handleReplyDestroyed(requestId);
    });

    connect(reply, &QNetworkReply::finished, this, [this, requestId]() {
        finishRequest(requestId);
    });
}

void TmdbClient::finishRequest(RequestId requestId)
{
    const auto active = m_activeRequests.find(requestId);
    if (active == m_activeRequests.end())
    {
        return;
    }

    ActiveRequest activeRequest = std::move(active.value());
    m_activeRequests.erase(active);

    InternalResult result = parseReply(*activeRequest.reply, activeRequest.kind);

    if (auto *error = std::get_if<TmdbError>(&result))
    {
        RR_LOG_W() << "TMDB request failed" << requestId
                   << requestKindLabel(activeRequest.kind)
                   << "category" << static_cast<int>(error->category())
                   << "httpStatus" << error->httpStatus().value_or(0);
    }
    else
    {
        RR_LOG_D() << "TMDB request finished" << requestId
                   << requestKindLabel(activeRequest.kind);
    }

    activeRequest.reply->deleteLater();
    startQueuedRequests();
    if (activeRequest.completionHandler)
    {
        activeRequest.completionHandler(std::move(result));
    }
}

void TmdbClient::handleReplyDestroyed(RequestId requestId)
{
    const auto active = m_activeRequests.find(requestId);
    if (active == m_activeRequests.end())
    {
        return;
    }

    RR_LOG_W() << "TMDB reply destroyed before finishing" << requestId
               << requestKindLabel(active->kind);

    InternalCompletionHandler completionHandler =
        std::move(active->completionHandler);
    m_activeRequests.erase(active);
    startQueuedRequests();

    if (completionHandler)
    {
        completionHandler(TmdbError(
            TmdbError::Category::Network,
            "The TMDB request ended before a response was received."));
    }
}

TmdbClient::InternalResult TmdbClient::parseReply(QNetworkReply &reply,
                                                  RequestKind kind)
{
    const QByteArray body = reply.readAll();
    const auto responseStatus = httpStatus(reply);
    const auto tmdbStatus = tmdbStatusCode(body);
    const bool unsuccessfulHttpStatus = responseStatus.has_value()
        && (*responseStatus < 200 || *responseStatus >= 300);
    const bool tmdbFailure = tmdbStatus.has_value() && *tmdbStatus != 1;

    if (reply.error() != QNetworkReply::NoError
        || unsuccessfulHttpStatus
        || tmdbFailure)
    {
        return mappedReplyError(reply, body);
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        return TmdbError(TmdbError::Category::Parse,
                         "TMDB returned a response that the app could not read.");
    }

    if (kind == RequestKind::Configuration)
    {
        ConfigurationResult result = parseConfigurationResponse(document.object());
        if (auto *configuration = std::get_if<TmdbConfigurationResponseDto>(&result))
        {
            return std::move(*configuration);
        }
        return std::move(std::get<TmdbError>(result));
    }

    if (kind == RequestKind::GenreListMovie
        || kind == RequestKind::GenreListTv)
    {
        GenreListResult result = parseGenreListResponse(document.object());
        if (auto *genres = std::get_if<TmdbGenreListResponseDto>(&result))
        {
            return std::move(*genres);
        }
        return std::move(std::get<TmdbError>(result));
    }

    if (kind == RequestKind::Videos)
    {
        VideosResult result = parseVideosResponse(document.object());
        if (auto *videos = std::get_if<TmdbVideosResponseDto>(&result))
        {
            return std::move(*videos);
        }
        return std::move(std::get<TmdbError>(result));
    }

    if (kind == RequestKind::WatchProviders)
    {
        WatchProvidersResult result = parseWatchProvidersResponse(document.object());
        if (auto *providers = std::get_if<TmdbWatchProvidersResponseDto>(&result))
        {
            return std::move(*providers);
        }
        return std::move(std::get<TmdbError>(result));
    }

    if (kind == RequestKind::TitleDetailsMovie || kind == RequestKind::TitleDetailsTv)
    {
        TitleDetailsResult result = kind == RequestKind::TitleDetailsMovie
            ? parseMovieDetailsResponse(document.object())
            : parseTvDetailsResponse(document.object());
        if (auto *details = std::get_if<TmdbTitleDetailsDto>(&result))
        {
            return std::move(*details);
        }
        return std::move(std::get<TmdbError>(result));
    }

    const bool isMovieKind = kind == RequestKind::DiscoverMovie
        || kind == RequestKind::TrendingMovie
        || kind == RequestKind::PopularMovie
        || kind == RequestKind::SearchMovie;
    const TmdbMediaType mediaType = isMovieKind
        ? TmdbMediaType::Movie
        : TmdbMediaType::Tv;
    DiscoverResult result = parseDiscoverPage(document.object(), mediaType);
    if (auto *page = std::get_if<TmdbDiscoverPageDto>(&result))
    {
        return std::move(*page);
    }
    return std::move(std::get<TmdbError>(result));
}

void TmdbClient::discardAllRequests() noexcept
{
    m_pendingRequests.clear();

    for (auto active = m_activeRequests.begin();
         active != m_activeRequests.end();
         ++active)
    {
        QNetworkReply *reply = active->reply;
        if (reply)
        {
            disconnect(reply, nullptr, this, nullptr);
            reply->abort();
            reply->deleteLater();
        }
    }
    m_activeRequests.clear();
}

TmdbError TmdbClient::cancelledError()
{
    return TmdbError(TmdbError::Category::Cancelled,
                     "The TMDB request was cancelled.");
}

const char *TmdbClient::requestKindLabel(RequestKind kind) noexcept
{
    switch (kind)
    {
    case RequestKind::DiscoverMovie:
        return "discoverMovie";
    case RequestKind::DiscoverTv:
        return "discoverTv";
    case RequestKind::GenreListMovie:
        return "genreListMovie";
    case RequestKind::GenreListTv:
        return "genreListTv";
    case RequestKind::Configuration:
        return "configuration";
    case RequestKind::TrendingMovie:
        return "trendingMovie";
    case RequestKind::TrendingTv:
        return "trendingTv";
    case RequestKind::PopularMovie:
        return "popularMovie";
    case RequestKind::PopularTv:
        return "popularTv";
    case RequestKind::Videos:
        return "videos";
    case RequestKind::SearchMovie:
        return "searchMovie";
    case RequestKind::SearchTv:
        return "searchTv";
    case RequestKind::WatchProviders:
        return "watchProviders";
    case RequestKind::TitleDetailsMovie:
        return "titleDetailsMovie";
    case RequestKind::TitleDetailsTv:
        return "titleDetailsTv";
    }

    return "unknown";
}

}
