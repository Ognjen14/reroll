#pragma once

#include "Candidate.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace Reroll::Domain
{

class CandidatePool final
{
public:
    using Collection = std::vector<Candidate>;
    using SizeType = Collection::size_type;
    using PageNumber = std::uint32_t;
    using FetchedPages = std::vector<PageNumber>;

    CandidatePool() = default;

    [[nodiscard]] const Collection &candidates() const noexcept;
    [[nodiscard]] SizeType size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] bool insert(Candidate candidate);
    [[nodiscard]] SizeType insertAll(Collection candidates);
    [[nodiscard]] bool recordFetchedPage(PageNumber page, PageNumber totalPages);
    [[nodiscard]] PageNumber totalPages() const noexcept;
    [[nodiscard]] const FetchedPages &fetchedPages() const noexcept;
    [[nodiscard]] bool hasFetchedPage(PageNumber page) const noexcept;
    [[nodiscard]] bool canRequestAnotherPage() const noexcept;
    [[nodiscard]] std::optional<PageNumber> nextUnfetchedPage() const noexcept;

private:
    Collection m_candidates;
    PageNumber m_totalPages{0};
    FetchedPages m_fetchedPages;
};

}
