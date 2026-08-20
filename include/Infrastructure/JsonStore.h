#pragma once

#include "Domain/FilterCriteria.h"
#include "Domain/MyListEntry.h"

#include <QString>

#include <vector>

namespace Reroll::Infrastructure
{

class JsonStore final
{
public:
    static constexpr int SchemaVersion = 1;

    explicit JsonStore(QString filePath = defaultFilePath());

    [[nodiscard]] static QString defaultFilePath();

    void load();

    [[nodiscard]] const Domain::FilterCriteria &filters() const noexcept;
    void setFilters(Domain::FilterCriteria filters);

    [[nodiscard]] const std::vector<Domain::MyListEntry> &myList() const noexcept;
    void setMyList(std::vector<Domain::MyListEntry> entries);

private:
    [[nodiscard]] bool save() const;

    QString m_filePath;
    Domain::FilterCriteria m_filters;
    std::vector<Domain::MyListEntry> m_myList;
};

}
