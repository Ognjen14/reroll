#include "Domain/MyListEntry.h"

#include <utility>

namespace Reroll::Domain
{

MyListEntry::MyListEntry(TitleSnapshot snapshot, bool watchlist, bool watched, bool hidden)
    : m_snapshot(std::move(snapshot))
    , m_watchlist(watchlist)
    , m_watched(watched)
    , m_hidden(hidden)
{
}

const TitleSnapshot &MyListEntry::snapshot() const noexcept
{
    return m_snapshot;
}

bool MyListEntry::watchlist() const noexcept
{
    return m_watchlist;
}

bool MyListEntry::watched() const noexcept
{
    return m_watched;
}

bool MyListEntry::hidden() const noexcept
{
    return m_hidden;
}

bool MyListEntry::isEmpty() const noexcept
{
    return !m_watchlist && !m_watched && !m_hidden;
}

void MyListEntry::setWatchlist(bool value) noexcept
{
    m_watchlist = value;
}

void MyListEntry::setWatched(bool value) noexcept
{
    m_watched = value;
}

void MyListEntry::setHidden(bool value) noexcept
{
    m_hidden = value;
}

bool operator==(const MyListEntry &left, const MyListEntry &right) noexcept
{
    return left.m_snapshot == right.m_snapshot
        && left.m_watchlist == right.m_watchlist
        && left.m_watched == right.m_watched
        && left.m_hidden == right.m_hidden;
}

bool operator!=(const MyListEntry &left, const MyListEntry &right) noexcept
{
    return !(left == right);
}

}
