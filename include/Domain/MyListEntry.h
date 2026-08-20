#pragma once

#include "Domain/TitleSnapshot.h"

namespace Reroll::Domain
{

class MyListEntry final
{
public:
    MyListEntry(TitleSnapshot snapshot, bool watchlist, bool watched, bool hidden);

    [[nodiscard]] const TitleSnapshot &snapshot() const noexcept;
    [[nodiscard]] bool watchlist() const noexcept;
    [[nodiscard]] bool watched() const noexcept;
    [[nodiscard]] bool hidden() const noexcept;
    [[nodiscard]] bool isEmpty() const noexcept;

    void setWatchlist(bool value) noexcept;
    void setWatched(bool value) noexcept;
    void setHidden(bool value) noexcept;

    friend bool operator==(const MyListEntry &left,
                           const MyListEntry &right) noexcept;
    friend bool operator!=(const MyListEntry &left,
                           const MyListEntry &right) noexcept;

private:
    TitleSnapshot m_snapshot;
    bool m_watchlist;
    bool m_watched;
    bool m_hidden;
};

}
