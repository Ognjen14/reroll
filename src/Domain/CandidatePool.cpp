#include "Domain/CandidatePool.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace Reroll::Domain
{

const CandidatePool::Collection &CandidatePool::candidates() const noexcept
{
    return m_candidates;
}

CandidatePool::SizeType CandidatePool::size() const noexcept
{
    return m_candidates.size();
}

bool CandidatePool::empty() const noexcept
{
    return m_candidates.empty();
}

bool CandidatePool::insert(Candidate candidate)
{
    const auto hasSameIdentity = [&candidate](const Candidate &existing) {
        return existing.identity() == candidate.identity();
    };

    if (std::any_of(m_candidates.cbegin(), m_candidates.cend(), hasSameIdentity))
    {
        return false;
    }

    m_candidates.push_back(std::move(candidate));
    return true;
}

CandidatePool::SizeType CandidatePool::insertAll(Collection candidates)
{
    SizeType insertedCount = 0;

    for (Candidate &candidate : candidates)
    {
        if (insert(std::move(candidate)))
        {
            ++insertedCount;
        }
    }

    return insertedCount;
}

bool CandidatePool::recordFetchedPage(PageNumber page, PageNumber totalPages)
{
    if (page == 0)
    {
        return false;
    }

    m_totalPages = totalPages;
    const auto position = std::lower_bound(m_fetchedPages.cbegin(),
                                           m_fetchedPages.cend(),
                                           page);
    if (position != m_fetchedPages.cend() && *position == page)
    {
        return false;
    }

    m_fetchedPages.insert(position, page);
    return true;
}

CandidatePool::PageNumber CandidatePool::totalPages() const noexcept
{
    return m_totalPages;
}

const CandidatePool::FetchedPages &CandidatePool::fetchedPages() const noexcept
{
    return m_fetchedPages;
}

bool CandidatePool::hasFetchedPage(PageNumber page) const noexcept
{
    return std::binary_search(m_fetchedPages.cbegin(), m_fetchedPages.cend(), page);
}

bool CandidatePool::canRequestAnotherPage() const noexcept
{
    return nextUnfetchedPage().has_value();
}

std::optional<CandidatePool::PageNumber> CandidatePool::nextUnfetchedPage() const noexcept
{
    PageNumber expected = 1;
    for (const PageNumber fetched : m_fetchedPages)
    {
        if (fetched > m_totalPages)
        {
            break;
        }
        if (fetched != expected)
        {
            return expected;
        }
        ++expected;
    }
    return expected <= m_totalPages ? std::optional<PageNumber>(expected) : std::nullopt;
}

}
