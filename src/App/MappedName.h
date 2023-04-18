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
#include <memory>

#include <boost/algorithm/string/predicate.hpp>

#include "ComplexGeoData.h"
#include "IndexedName.h"
#include "LazyClass.hpp"

namespace Data
{

typedef Lazy<std::string> LazyString;


// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

/// The MappedName class maintains a two-part name: the first part ("data") is considered immutable
/// once created, while the second part ("postfix") can be modified/appended to by later operations.
/// It uses shared data when possible (see the fromRawData() members). Despite storing data and
/// postfix separately, they can be accessed via calls to size(), operator[], etc. as though they
/// were a single array.
class MappedName;
typedef std::shared_ptr<MappedName> MappedNameRef;


class AppExport MappedName
{
public:

    explicit MappedName(const std::string& name)
    {
        if (boost::starts_with(name, ComplexGeoData::elementMapPrefix())) 
        {
            data = name.substr(ComplexGeoData::elementMapPrefix().size());
        }
        else
        {
            data = name;
        }
        postfixStartIdx = data.size();
    }

    explicit MappedName(const char* name, size_t size = std::string::npos)
        : MappedName(size != std::string::npos ? std::string(name, size) : std::string(name)) 
    {}


    explicit MappedName(const IndexedName& element)
        : data(element.getType())
    {
        if (element.getIndex() > 0) {
            data += std::to_string(element.getIndex());
        }
        postfixStartIdx = data.size();
    }

    MappedName() = default;

    MappedName(const MappedName& other) = default;
    MappedName& operator=(const MappedName& other) = default; 

    MappedName(const MappedName& other, size_t startPosition, size_t size = std::string::npos) 
    {
        append(other, startPosition, size);
    }

    MappedName(const MappedName& other, const char* postfix) 
        : data(other.data + postfix),
          postfixStartIdx(other.size())
    {}

    /// Move constructor
    MappedName(MappedName&& other) noexcept
        : data(std::move(other.data)),
          postfixStartIdx(other.postfixStartIdx)
    {
        other.postfixStartIdx = 0; //properly reset other object
    }

    MappedName& operator=(MappedName&& other) noexcept
    {
        this->data = std::move(other.data);
        this->postfixStartIdx = other.postfixStartIdx;
        other.postfixStartIdx = 0; //properly reset other object
        return *this;
    }

    ~MappedName() = default;

    void copy(MappedNameRef reference)
    {
        *this = std::move(MappedName(*reference));
    }


    /// Create a new MappedName from a std::string: the string's data is copied.
    MappedName& operator=(const std::string& other)
    {
        *this = MappedName(other);
        return *this;
    }

    /// Create a new MappedName from a const char *. The character data is copied.
    MappedName& operator=(const char* other)
    {
        *this = MappedName(other);
        return *this;
    }


    friend std::ostream& operator<<(std::ostream& stream, const MappedName& mappedName) 
    {
        stream.write(mappedName.data.c_str(), mappedName.data.size());
        return stream;
    }

    /// Two MappedNames are equal if the concatenation of their data and postfix is equal. The
    /// individual data and postfix may NOT be equal in this case.
    bool operator==(const MappedName& other) const
    {
        return this->data == other.data; //NOT && this->postfixStartIdx == other.postfixStartIdx;
    }

    bool operator!=(const MappedName& other) const
    {
        return !(this->operator==(other));
    }

    MappedName operator+(const MappedName& other) const
    {
        MappedName res(*this);
        res += other;
        return res;
    }

    /// Returns a new MappedName whose data is the LHS argument's data and whose postfix is the LHS
    /// argument's postfix with the RHS argument appended to it. The character data is copied.
    MappedName operator+(const char* other) const
    {
        MappedName res(*this);
        res += other;
        return res;
    }

    /// Returns a new MappedName whose data is the LHS argument's data and whose postfix is the LHS
    /// argument's postfix with the RHS argument appended to it. The character data is copied.
    MappedName operator+(const std::string& other) const
    {
        MappedName res(*this);
        res += other;
        return res;
    }

    MappedName& operator+=(const char* other) 
    {
        this->data.append(other);
        return *this;
    }

    /// Appends other to this instance's postfix. The character data from the string is copied.
    MappedName& operator+=(const std::string& other)
    {
        this->data.append(other);
        return *this;
    }

    MappedName& operator+=(const MappedName& other) 
    {
        append(other);
        return *this;
    }

    void append(const char* dataToAppend, size_t size = std::string::npos)
    {
        std::string stringToAppend = size != std::string::npos ? std::string(dataToAppend, size) : std::string(dataToAppend); 
        if (this->data.size() == 0)
        {
            postfixStartIdx = stringToAppend.size();
        }
        this->data.append(stringToAppend); 
    }

    void append(const MappedName& other, size_t startPosition = 0, size_t size = std::string::npos)
    {
        std::string stringToAppend = other.data.substr(startPosition, size);
        if (this->data.size() == 0 && other.postfixStartIdx >= startPosition)
        {
            postfixStartIdx = other.postfixStartIdx - startPosition;
        }
        this->data.append(stringToAppend);
    }

    std::string toString() const { return this->data; }
    const std::string name() const { return this->data.substr(0, postfixStartIdx); }
    const std::string postfix() const { return this->data.substr(postfixStartIdx); }


    /// Create an IndexedName from the data portion of this MappedName. If this data has a postfix,
    /// the function returns an empty IndexedName. The function will fail if this->data contains
    /// anything other than the ASCII letter a-z, A-Z, and the underscore, with an optional integer
    /// suffix, returning an empty IndexedName (e.g. an IndexedName that evaluates to boolean
    /// false and isNull() == true).
    ///
    /// \return a new IndexedName that shares its data with this instance's data member.
    IndexedName toIndexedName() const
    {
        if (this->postfixStartIdx == this->data.size()) {
            return IndexedName(this->data.c_str()); 
        }
        return IndexedName();
    }


    /// Equivalent to C++20 operator<=>. Performs byte-by-byte comparison of this and other,
    /// starting at the first byte and continuing through both data and postfix, ignoring which is
    /// which. If the combined data and postfix members are of unequal size but start with the same
    /// data, the shorter array is considered "less than" the longer.
    int compare(const MappedName& other) const
    {
        auto val = this->data.compare(other.data);
        if (val < 0) { return -1; }
        if (val > 0) { return 1; }
        return 0;
    }

    /// \see compare()
    bool operator<(const MappedName& other) const
    {
        return compare(other) < 0;
    }


    char operator[](size_t index) const { return this->data[index]; }

    size_t size() const { return this->data.size(); }
    bool empty() const { return this->data.empty(); }
    explicit operator bool() const { return !empty(); }

    /// Reset this instance, clearing anything in data and postfix.
    void clear()
    {
        this->data.clear();
        this->postfixStartIdx = 0;
    }


    size_t find(const char* searchTarget, size_t startPosition = 0) const
    {
        return this->data.find(searchTarget, startPosition);
    }

    size_t find(const std::string& searchTarget, size_t startPosition = 0) const
    {
        return this->data.find(searchTarget, startPosition);
    }

    size_t rfind(const char* searchTarget, size_t startPosition = std::string::npos) const
    {
        return this->data.rfind(searchTarget, startPosition);
    }

    size_t rfind(const std::string& searchTarget, size_t startPosition = std::string::npos) const
    {
        return this->data.rfind(searchTarget, startPosition);
    }

    /* These are string builtin in c++20 only, have to use boost I guess */
    bool endsWith(const char* searchTarget) const
    {
        return boost::ends_with(this->data, searchTarget);
    }

    /// Returns true if this MappedName ends with the search target. If there is a postfix, only the
    /// postfix is considered. If not, then only the data is considered. A search string that
    /// overlaps the two will not be found.
    bool endsWith(const std::string& searchTarget) const
    {
        return boost::ends_with(this->data, searchTarget);
    }

    bool startsWith(const char* searchTarget, size_t offset = 0) const
    {
        return boost::starts_with(this->data.substr(offset), searchTarget);
    }

    bool startsWith(const std::string& searchTarget, size_t offset = 0) const
    {
        return boost::starts_with(this->data.substr(offset), searchTarget);
    }


private:
    LazyString data;
    size_t postfixStartIdx = 0;
};

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)


}// namespace Data


#endif// APP_MAPPED_NAME_H