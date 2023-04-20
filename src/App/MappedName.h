// SPDX-License-Identifier: LGPL-2.1-or-later

/****************************************************************************
 *   Copyright (c) 2022 Zheng, Lei (realthunder) <realthunder.dev@gmail.com>*
 *   Copyright (c) 2023 FreeCAD Project Association                         *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#ifndef APP_MAPPED_NAME_H
#define APP_MAPPED_NAME_H


#include <string>

#include "IndexedName.h"
#include "LazyClass.hpp"

#include <iostream>

namespace Data
{

typedef Lazy<std::string> LazyString;


// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

class AppExport MappedName
{
public:
    explicit MappedName(const std::string& name);
    explicit MappedName(const char* name, size_t size = std::string::npos);
    explicit MappedName(const IndexedName& element);

    MappedName() = default;

    MappedName(const MappedName& other) = default;
    MappedName& operator=(const MappedName& other) = default;

    MappedName(const MappedName& other, size_t startPosition, size_t size = std::string::npos);
    MappedName(const MappedName& other, const char* postfix);

    MappedName(MappedName&& other) noexcept;
    MappedName& operator=(MappedName&& other) noexcept;

    ~MappedName() = default;

    MappedName& operator=(const std::string& other);
    MappedName& operator=(const char* other);

    friend std::ostream& operator<<(std::ostream& stream, const MappedName& mappedName)
    {
        stream.write(((std::string)mappedName.data).c_str(), ((std::string)mappedName.data).size());
        return stream;
    }

    bool operator==(const MappedName& other) const;
    bool operator!=(const MappedName& other) const;

    MappedName operator+(const MappedName& other) const;
    MappedName operator+(const char* other) const;
    MappedName operator+(const std::string& other) const;

    MappedName& operator+=(const MappedName& other);
    MappedName& operator+=(const char* other);
    MappedName& operator+=(const std::string& other);

    void append(const char* dataToAppend, size_t size = std::string::npos);
    void append(const MappedName& other, size_t startPosition = 0, size_t size = std::string::npos);

    std::string toString() const;
    const std::string name() const;
    const std::string postfix() const;

    /// Create an IndexedName from the data portion of this MappedName. If this data has a postfix,
    /// the function returns an empty IndexedName. The function will fail if this->data contains
    /// anything other than the ASCII letter a-z, A-Z, and the underscore, with an optional integer
    /// suffix, returning an empty IndexedName (e.g. an IndexedName that evaluates to boolean
    /// false and isNull() == true).
    ///
    /// \return a new IndexedName that shares its data with this instance's data member.
    IndexedName toIndexedName() const;

    /// Equivalent to C++20 operator<=>. Performs byte-by-byte comparison of this and other,
    /// starting at the first byte and continuing through both data and postfix, ignoring which is
    /// which. If the combined data and postfix members are of unequal size but start with the same
    /// data, the shorter array is considered "less than" the longer.
    int compare(const MappedName& other) const;

    /// \see compare()
    bool operator<(const MappedName& other) const;

    char operator[](size_t index) const;

    size_t size() const;
    bool empty() const;
    explicit operator bool() const;
    void clear();

    size_t find(const char* searchTarget, size_t startPosition = 0) const;
    size_t find(const std::string& searchTarget, size_t startPosition = 0) const;

    size_t rfind(const char* searchTarget, size_t startPosition = std::string::npos) const;
    size_t rfind(const std::string& searchTarget, size_t startPosition = std::string::npos) const;

    bool endsWith(const char* searchTarget) const;
    bool endsWith(const std::string& searchTarget) const;

    bool startsWith(const char* searchTarget, size_t offset = 0) const;
    bool startsWith(const std::string& searchTarget, size_t offset = 0) const;


private:
    LazyString data; //TODO migrate to LazyString
    size_t postfixStartIdx = 0;
};

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)


}// namespace Data


#endif// APP_MAPPED_NAME_H