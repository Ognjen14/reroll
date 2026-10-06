#pragma once

#include "Candidate.h"

#include <optional>
#include <vector>

namespace Reroll::Domain
{

class SuggestionSession final
{
public:
    using ShownIdentities = std::vector<CandidateIdentity>;
    using SizeType = ShownIdentities::size_type;
    using History = std::vector<Candidate>;

    static constexpr SizeType MaximumHistorySize = 50;

    SuggestionSession() = default;

    [[nodiscard]] bool selectInitial(Candidate candidate);
    [[nodiscard]] bool select(Candidate candidate);
    [[nodiscard]] const std::optional<Candidate> &currentSelection() const noexcept;
    [[nodiscard]] bool canGoBack() const noexcept;
    [[nodiscard]] bool canGoForward() const noexcept;
    [[nodiscard]] bool goBack();
    [[nodiscard]] bool goForward();
    [[nodiscard]] const History &backHistory() const noexcept;
    [[nodiscard]] const History &forwardHistory() const noexcept;
    void discardBackEntry(SizeType index);
    void discardForwardEntry(SizeType index);
    [[nodiscard]] bool recordShown(CandidateIdentity identity);
    [[nodiscard]] bool hasBeenShown(const CandidateIdentity &identity) const noexcept;
    [[nodiscard]] const ShownIdentities &shownIdentities() const noexcept;
    [[nodiscard]] SizeType shownCount() const noexcept;
    [[nodiscard]] SizeType rerollCount() const noexcept;
    [[nodiscard]] bool exhausted() const noexcept;
    void markExhausted() noexcept;
    void restartCycle() noexcept;
    void reset() noexcept;

private:
    void pushBack(Candidate candidate);

    std::optional<Candidate> m_currentSelection;
    History m_backHistory;
    History m_forwardHistory;
    ShownIdentities m_shownIdentities;
    SizeType m_rerollCount{0};
    bool m_exhausted{false};
};

}
