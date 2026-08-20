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

    SuggestionSession() = default;

    [[nodiscard]] bool selectInitial(Candidate candidate);
    [[nodiscard]] bool select(Candidate candidate);
    [[nodiscard]] const std::optional<Candidate> &currentSelection() const noexcept;
    [[nodiscard]] bool recordShown(CandidateIdentity identity);
    [[nodiscard]] bool hasBeenShown(const CandidateIdentity &identity) const noexcept;
    [[nodiscard]] const ShownIdentities &shownIdentities() const noexcept;
    [[nodiscard]] SizeType shownCount() const noexcept;
    [[nodiscard]] SizeType rerollCount() const noexcept;
    [[nodiscard]] bool exhausted() const noexcept;
    void markExhausted() noexcept;
    void reset() noexcept;

private:
    std::optional<Candidate> m_currentSelection;
    ShownIdentities m_shownIdentities;
    SizeType m_rerollCount{0};
    bool m_exhausted{false};
};

}
