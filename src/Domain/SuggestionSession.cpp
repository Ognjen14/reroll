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
    }

    m_currentSelection = std::move(candidate);
    m_exhausted = false;
    return true;
}

const std::optional<Candidate> &SuggestionSession::currentSelection() const noexcept
{
    return m_currentSelection;
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

void SuggestionSession::reset() noexcept
{
    m_currentSelection.reset();
    m_shownIdentities.clear();
    m_rerollCount = 0;
    m_exhausted = false;
}

}
