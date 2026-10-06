#include "Domain/SuggestionSession.h"

#include <algorithm>
#include <utility>

namespace Reroll::Domain
{

bool SuggestionSession::selectInitial(Candidate candidate)
{
    if (!recordShown(candidate.identity()))
    {
        return false;
    }

    m_currentSelection = std::move(candidate);
    m_exhausted = false;
    return true;
}

bool SuggestionSession::select(Candidate candidate)
{
    if (!recordShown(candidate.identity()))
    {
        return false;
    }

    if (m_currentSelection.has_value())
    {
        ++m_rerollCount;
        pushBack(std::move(*m_currentSelection));
    }

    m_forwardHistory.clear();
    m_currentSelection = std::move(candidate);
    m_exhausted = false;
    return true;
}

const std::optional<Candidate> &SuggestionSession::currentSelection() const noexcept
{
    return m_currentSelection;
}

bool SuggestionSession::canGoBack() const noexcept
{
    return !m_backHistory.empty();
}

bool SuggestionSession::canGoForward() const noexcept
{
    return !m_forwardHistory.empty();
}

bool SuggestionSession::goBack()
{
    if (m_backHistory.empty())
    {
        return false;
    }

    if (m_currentSelection.has_value())
    {
        m_forwardHistory.push_back(std::move(*m_currentSelection));
    }
    m_currentSelection = std::move(m_backHistory.back());
    m_backHistory.pop_back();
    return true;
}

bool SuggestionSession::goForward()
{
    if (m_forwardHistory.empty())
    {
        return false;
    }

    if (m_currentSelection.has_value())
    {
        pushBack(std::move(*m_currentSelection));
    }
    m_currentSelection = std::move(m_forwardHistory.back());
    m_forwardHistory.pop_back();
    return true;
}

const SuggestionSession::History &SuggestionSession::backHistory() const noexcept
{
    return m_backHistory;
}

const SuggestionSession::History &SuggestionSession::forwardHistory() const noexcept
{
    return m_forwardHistory;
}

void SuggestionSession::discardBackEntry(SizeType index)
{
    if (index < m_backHistory.size())
    {
        m_backHistory.erase(m_backHistory.begin() + static_cast<History::difference_type>(index));
    }
}

void SuggestionSession::discardForwardEntry(SizeType index)
{
    if (index < m_forwardHistory.size())
    {
        m_forwardHistory.erase(
            m_forwardHistory.begin() + static_cast<History::difference_type>(index));
    }
}

void SuggestionSession::pushBack(Candidate candidate)
{
    m_backHistory.push_back(std::move(candidate));
    if (m_backHistory.size() > MaximumHistorySize)
    {
        m_backHistory.erase(m_backHistory.begin());
    }
}

bool SuggestionSession::recordShown(CandidateIdentity identity)
{
    if (hasBeenShown(identity))
    {
        return false;
    }

    m_shownIdentities.push_back(identity);
    return true;
}

bool SuggestionSession::hasBeenShown(const CandidateIdentity &identity) const noexcept
{
    return std::find(m_shownIdentities.cbegin(), m_shownIdentities.cend(), identity)
        != m_shownIdentities.cend();
}

const SuggestionSession::ShownIdentities &
SuggestionSession::shownIdentities() const noexcept
{
    return m_shownIdentities;
}

SuggestionSession::SizeType SuggestionSession::shownCount() const noexcept
{
    return m_shownIdentities.size();
}

SuggestionSession::SizeType SuggestionSession::rerollCount() const noexcept
{
    return m_rerollCount;
}

bool SuggestionSession::exhausted() const noexcept
{
    return m_exhausted;
}

void SuggestionSession::markExhausted() noexcept
{
    m_exhausted = true;
}

void SuggestionSession::restartCycle() noexcept
{
    m_shownIdentities.clear();
    m_exhausted = false;
}

void SuggestionSession::reset() noexcept
{
    m_currentSelection.reset();
    m_backHistory.clear();
    m_forwardHistory.clear();
    m_shownIdentities.clear();
    m_rerollCount = 0;
    m_exhausted = false;
}

}
