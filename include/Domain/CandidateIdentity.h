#pragma once

#include "MediaType.h"

#include <cstdint>

namespace Reroll::Domain
{

class CandidateIdentity final
{
public:
    using TmdbId = std::int64_t;

    constexpr CandidateIdentity(MediaType mediaType, TmdbId tmdbId) noexcept
        : m_mediaType(mediaType)
        , m_tmdbId(tmdbId)
    {
    }

    [[nodiscard]] constexpr MediaType mediaType() const noexcept
    {
        return m_mediaType;
    }

    [[nodiscard]] constexpr TmdbId tmdbId() const noexcept
    {
        return m_tmdbId;
    }

    [[nodiscard]] friend constexpr bool operator==(const CandidateIdentity &left,
                                                   const CandidateIdentity &right) noexcept
    {
        return left.m_mediaType == right.m_mediaType
            && left.m_tmdbId == right.m_tmdbId;
    }

    [[nodiscard]] friend constexpr bool operator!=(const CandidateIdentity &left,
                                                   const CandidateIdentity &right) noexcept
    {
        return !(left == right);
    }

private:
    MediaType m_mediaType;
    TmdbId m_tmdbId;
};

}
