
#include "Infrastructure/JsonStore.h"

#include "RRLog.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSaveFile>
#include <QStandardPaths>

#include <optional>
#include <utility>

namespace Reroll::Infrastructure
{
namespace
{

QString mediaTypeToString(Domain::MediaType mediaType)
{
    return mediaType == Domain::MediaType::Movie
        ? QStringLiteral("movie")
        : QStringLiteral("tv");
}

std::optional<Domain::MediaType> mediaTypeFromString(const QJsonValue &value)
{
    if (value.toString() == QStringLiteral("movie"))
    {
        return Domain::MediaType::Movie;
    }
    if (value.toString() == QStringLiteral("tv"))
    {
        return Domain::MediaType::Tv;
    }
    return std::nullopt;
}

QString mediaTypeFilterToString(Domain::MediaTypeFilter mediaType)
{
    switch (mediaType)
    {
    case Domain::MediaTypeFilter::Movie:
        return QStringLiteral("movie");
    case Domain::MediaTypeFilter::Tv:
        return QStringLiteral("tv");
    case Domain::MediaTypeFilter::Both:
        return QStringLiteral("both");
    }
    return QStringLiteral("movie");
}

std::optional<Domain::MediaTypeFilter> mediaTypeFilterFromString(const QJsonValue &value)
{
    if (value.toString() == QStringLiteral("movie"))
    {
        return Domain::MediaTypeFilter::Movie;
    }
    if (value.toString() == QStringLiteral("tv"))
    {
        return Domain::MediaTypeFilter::Tv;
    }
    if (value.toString() == QStringLiteral("both"))
    {
        return Domain::MediaTypeFilter::Both;
    }
    return std::nullopt;
}

QString genreMatchModeToString(Domain::GenreMatchMode mode)
{
    return mode == Domain::GenreMatchMode::And
        ? QStringLiteral("and")
        : QStringLiteral("or");
}

std::optional<Domain::GenreMatchMode> genreMatchModeFromString(const QJsonValue &value)
{
    if (value.toString() == QStringLiteral("or"))
    {
        return Domain::GenreMatchMode::Or;
    }
    if (value.toString() == QStringLiteral("and"))
    {
        return Domain::GenreMatchMode::And;
    }
    return std::nullopt;
}

std::optional<Domain::FilterCriteria::Year> optionalYear(const QJsonValue &value)
{
    if (value.isNull() || value.isUndefined() || !value.isDouble())
    {
        return std::nullopt;
    }
    return static_cast<Domain::FilterCriteria::Year>(value.toInteger());
}

QJsonArray genreIdsToJson(const Domain::FilterCriteria::GenreIds &genreIds)
{
    QJsonArray array;
    for (const auto genreId : genreIds)
    {
        array.append(genreId);
    }
    return array;
}

Domain::FilterCriteria::GenreIds genreIdsFromJson(const QJsonValue &value)
{
    Domain::FilterCriteria::GenreIds genreIds;
    if (!value.isArray())
    {
        return genreIds;
    }
    for (const QJsonValue &entry : value.toArray())
    {
        if (entry.isDouble())
        {
            genreIds.push_back(
                static_cast<Domain::FilterCriteria::GenreId>(entry.toInteger()));
        }
    }
    return genreIds;
}

QJsonObject filtersToJson(const Domain::FilterCriteria &filters)
{
    QJsonObject object;
    object.insert(QStringLiteral("mediaType"), mediaTypeFilterToString(filters.mediaType()));
    object.insert(QStringLiteral("minimumYear"),
                 filters.minimumYear().has_value()
                     ? QJsonValue(*filters.minimumYear())
                     : QJsonValue());
    object.insert(QStringLiteral("maximumYear"),
                 filters.maximumYear().has_value()
                     ? QJsonValue(*filters.maximumYear())
                     : QJsonValue());
    object.insert(QStringLiteral("minimumRating"), filters.minimumRating());
    object.insert(QStringLiteral("genreIds"), genreIdsToJson(filters.genreIds()));
    object.insert(QStringLiteral("genreMatchMode"),
                 genreMatchModeToString(filters.genreMatchMode()));
    object.insert(QStringLiteral("excludeWatched"), filters.excludeWatched());
    object.insert(QStringLiteral("originalLanguage"),
                 QString::fromStdString(filters.originalLanguage()));
    return object;
}

Domain::FilterCriteria filtersFromJson(const QJsonValue &value)
{
    if (!value.isObject())
    {
        return {};
    }

    const QJsonObject object = value.toObject();
    const auto mediaType = mediaTypeFilterFromString(object.value(QStringLiteral("mediaType")));
    const auto matchMode = genreMatchModeFromString(
        object.value(QStringLiteral("genreMatchMode")));
    if (!mediaType.has_value() || !matchMode.has_value())
    {
        RR_LOG_W() << "JsonStore filters block malformed, using defaults";
        return {};
    }

    return Domain::FilterCriteria(
        *mediaType,
        optionalYear(object.value(QStringLiteral("minimumYear"))),
        optionalYear(object.value(QStringLiteral("maximumYear"))),
        object.value(QStringLiteral("minimumRating")).toDouble(),
        genreIdsFromJson(object.value(QStringLiteral("genreIds"))),
        *matchMode,
        object.value(QStringLiteral("excludeWatched")).toBool(),
        object.value(QStringLiteral("originalLanguage")).toString().toStdString());
}

QJsonObject myListEntryToJson(const Domain::MyListEntry &entry)
{
    const Domain::TitleSnapshot &snapshot = entry.snapshot();
    QJsonObject object;
    object.insert(QStringLiteral("tmdbId"), static_cast<double>(snapshot.tmdbId()));
    object.insert(QStringLiteral("mediaType"), mediaTypeToString(snapshot.mediaType()));
    object.insert(QStringLiteral("title"),
                 QString::fromStdString(snapshot.title()));
    object.insert(QStringLiteral("releaseYear"), snapshot.releaseYear());
    object.insert(QStringLiteral("genreIds"), genreIdsToJson(snapshot.genreIds()));
    object.insert(QStringLiteral("posterPath"),
                 QString::fromStdString(snapshot.posterPath()));
    object.insert(QStringLiteral("rating"), snapshot.rating());
    object.insert(QStringLiteral("voteCount"),
                 static_cast<double>(snapshot.voteCount()));
    object.insert(QStringLiteral("watchlist"), entry.watchlist());
    object.insert(QStringLiteral("watched"), entry.watched());
    object.insert(QStringLiteral("hidden"), entry.hidden());
    return object;
}

std::optional<Domain::MyListEntry> myListEntryFromJson(const QJsonValue &value)
{
    if (!value.isObject())
    {
        return std::nullopt;
    }

    const QJsonObject object = value.toObject();
    const auto mediaType = mediaTypeFromString(object.value(QStringLiteral("mediaType")));
    const QJsonValue idValue = object.value(QStringLiteral("tmdbId"));
    const QJsonValue titleValue = object.value(QStringLiteral("title"));
    const QJsonValue yearValue = object.value(QStringLiteral("releaseYear"));
    const QJsonValue posterPathValue = object.value(QStringLiteral("posterPath"));
    const QJsonValue ratingValue = object.value(QStringLiteral("rating"));
    const QJsonValue voteCountValue = object.value(QStringLiteral("voteCount"));

    if (!mediaType.has_value()
        || !idValue.isDouble()
        || !titleValue.isString()
        || !yearValue.isDouble()
        || !posterPathValue.isString()
        || !ratingValue.isDouble()
        || !voteCountValue.isDouble())
    {
        return std::nullopt;
    }

    Domain::TitleSnapshot snapshot(
        idValue.toInteger(),
        *mediaType,
        titleValue.toString().toStdString(),
        static_cast<int>(yearValue.toInteger()),
        genreIdsFromJson(object.value(QStringLiteral("genreIds"))),
        posterPathValue.toString().toStdString(),
        ratingValue.toDouble(),
        voteCountValue.toInteger());

    return Domain::MyListEntry(
        std::move(snapshot),
        object.value(QStringLiteral("watchlist")).toBool(),
        object.value(QStringLiteral("watched")).toBool(),
        object.value(QStringLiteral("hidden")).toBool());
}

}

JsonStore::JsonStore(QString filePath)
    : m_filePath(std::move(filePath))
{
}

QString JsonStore::defaultFilePath()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
        .filePath(QStringLiteral("app.json"));
}

void JsonStore::load()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        RR_LOG_I() << "JsonStore found no existing app data file, using defaults"
                   << m_filePath;
        m_filters = Domain::FilterCriteria{};
        m_myList.clear();
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        RR_LOG_W() << "JsonStore could not parse app data file, using defaults"
                   << parseError.errorString();
        m_filters = Domain::FilterCriteria{};
        m_myList.clear();
        return;
    }

    const QJsonObject root = document.object();
    const int schemaVersion = root.value(QStringLiteral("schemaVersion")).toInt(-1);
    if (schemaVersion != SchemaVersion)
    {
        RR_LOG_W() << "JsonStore found unexpected schema version, attempting to "
                      "recover filters/MyList field-by-field anyway"
                   << schemaVersion;
    }

    m_filters = filtersFromJson(root.value(QStringLiteral("filters")));

    m_myList.clear();
    const QJsonValue myListValue = root.value(QStringLiteral("myList"));
    if (myListValue.isArray())
    {
        int skipped = 0;
        for (const QJsonValue &entryValue : myListValue.toArray())
        {
            auto entry = myListEntryFromJson(entryValue);
            if (entry.has_value())
            {
                m_myList.push_back(std::move(*entry));
            }
            else
            {
                ++skipped;
            }
        }
        if (skipped > 0)
        {
            RR_LOG_W() << "JsonStore skipped malformed MyList entries" << skipped;
        }
    }

    RR_LOG_I() << "JsonStore loaded app data" << m_myList.size() << "MyList entries";
}

const Domain::FilterCriteria &JsonStore::filters() const noexcept
{
    return m_filters;
}

void JsonStore::setFilters(Domain::FilterCriteria filters)
{
    m_filters = std::move(filters);
    RR_LOG_D() << "JsonStore filters updated";
    static_cast<void>(save());
}

const std::vector<Domain::MyListEntry> &JsonStore::myList() const noexcept
{
    return m_myList;
}

void JsonStore::setMyList(std::vector<Domain::MyListEntry> entries)
{
    m_myList = std::move(entries);
    RR_LOG_D() << "JsonStore MyList updated" << m_myList.size() << "entries";
    static_cast<void>(save());
}

bool JsonStore::save() const
{
    QJsonObject root;
    root.insert(QStringLiteral("schemaVersion"), SchemaVersion);
    root.insert(QStringLiteral("filters"), filtersToJson(m_filters));

    QJsonArray myListArray;
    for (const Domain::MyListEntry &entry : m_myList)
    {
        myListArray.append(myListEntryToJson(entry));
    }
    root.insert(QStringLiteral("myList"), myListArray);

    const QFileInfo fileInfo(m_filePath);
    if (!QDir().mkpath(fileInfo.absolutePath()))
    {
        RR_LOG_E() << "JsonStore could not create app data directory"
                   << fileInfo.absolutePath();
        return false;
    }

    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        RR_LOG_E() << "JsonStore could not open app data file for writing"
                   << m_filePath;
        return false;
    }

    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Compact);
    if (file.write(bytes) != bytes.size())
    {
        RR_LOG_E() << "JsonStore write was incomplete" << m_filePath;
        file.cancelWriting();
        return false;
    }

    if (!file.commit())
    {
        RR_LOG_E() << "JsonStore could not commit app data file" << m_filePath;
        return false;
    }

    RR_LOG_D() << "JsonStore saved app data" << m_filePath;
    return true;
}

}
