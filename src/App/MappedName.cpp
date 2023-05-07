/****************************************************************************
 *   Copyright (c) 2020 Zheng, Lei (realthunder) <realthunder.dev@gmail.com>*
 *                                                                          *
 *   This file is part of the FreeCAD CAx development system.               *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Library General Public            *
 *   License as published by the Free Software Foundation; either           *
 *   version 2 of the License, or (at your option) any later version.       *
 *                                                                          *
 *   This library  is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Library General Public License for more details.                   *
 *                                                                          *
 *   You should have received a copy of the GNU Library General Public      *
 *   License along with this library; see the file COPYING.LIB. If not,     *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,          *
 *   Suite 330, Boston, MA  02111-1307, USA                                 *
 *                                                                          *
 ****************************************************************************/

#include "PreCompiled.h"

#include "MappedName.h"

#include <memory>

#include <boost/algorithm/string/predicate.hpp>

#include "ComplexGeoData.h"

using namespace Data;


MappedName::MappedName(const std::string& name)
{
    if (boost::starts_with(name, ComplexGeoData::elementMapPrefix())) {
        data = name.substr(ComplexGeoData::elementMapPrefix().size());
    }
    else {
        data = name;
    }
    postfixStartIdx = data->size();
}

MappedName::MappedName(const char* name, size_t size)
    : MappedName(size != std::string::npos ? std::string(name, size) : std::string(name))
{}

MappedName::MappedName(const IndexedName& element)
    : data(element.getType())
{
    if (element.getIndex() > 0) {
        *data += std::to_string(element.getIndex());
    }
    postfixStartIdx = data->size();
}

MappedName::MappedName(const MappedName& other, size_t startPosition, size_t size)
{
    append(other, startPosition, size);
}

MappedName::MappedName(const MappedName& other, const char* postfix)
    : data(*other.data + postfix),
      postfixStartIdx(other.size())
{}

MappedName::MappedName(MappedName&& other) noexcept
    : data(std::move(other.data)),
      postfixStartIdx(other.postfixStartIdx)
{
    other.postfixStartIdx = 0;// properly reset other object
}

MappedName& MappedName::operator=(MappedName&& other) noexcept
{
    this->data = std::move(other.data);
    this->postfixStartIdx = other.postfixStartIdx;
    other.postfixStartIdx = 0;// properly reset other object
    return *this;
}

MappedName& MappedName::operator=(const std::string& other)
{
    *this = MappedName(other);
    return *this;
}

MappedName& MappedName::operator=(const char* other)
{
    *this = MappedName(other);
    return *this;
}

bool MappedName::operator==(const MappedName& other) const
{
    return *this->data == *other.data;
}

bool MappedName::operator!=(const MappedName& other) const
{
    return !(this->operator==(other));
}

MappedName MappedName::operator+(const MappedName& other) const
{
    MappedName res(*this);
    res += other;
    return res;
}

MappedName MappedName::operator+(const char* other) const
{
    MappedName res(*this);
    res += other;
    return res;
}

MappedName MappedName::operator+(const std::string& other) const
{
    MappedName res(*this);
    res += other;
    return res;
}

MappedName& MappedName::operator+=(const char* other)
{
    this->data->append(other);
    return *this;
}

MappedName& MappedName::operator+=(const std::string& other)
{
    this->data->append(other);
    return *this;
}

MappedName& MappedName::operator+=(const MappedName& other)
{
    append(other);
    return *this;
}

void MappedName::append(const char* dataToAppend, size_t size)
{
    std::string stringToAppend =
        size != std::string::npos ? std::string(dataToAppend, size) : std::string(dataToAppend);
    if (this->data->size() == 0) {
        postfixStartIdx = stringToAppend.size();
    }
    this->data->append(stringToAppend);
}

void MappedName::append(const MappedName& other, size_t startPosition, size_t size)
{
    std::string stringToAppend = other.data->substr(startPosition, size);
    if (this->data->size() == 0 && other.postfixStartIdx >= startPosition) {
        postfixStartIdx = other.postfixStartIdx - startPosition;
    }
    this->data->append(stringToAppend);
}

std::string MappedName::toString() const
{
    return *this->data;
}

const std::string MappedName::name() const
{
    return this->data->substr(0, postfixStartIdx);
}

const std::string MappedName::postfix() const
{
    return this->data->substr(postfixStartIdx);
}

IndexedName MappedName::toIndexedName() const
{
    if (this->postfixStartIdx == this->data->size()) {
        return IndexedName(this->data->c_str());
    }
    return IndexedName();
}

int MappedName::compare(const MappedName& other) const
{
    auto val = this->data->compare(*other.data);
    if (val < 0) {
        return -1;
    }
    if (val > 0) {
        return 1;
    }
    return 0;
}

bool MappedName::operator<(const MappedName& other) const
{
    return compare(other) < 0;
}

char MappedName::operator[](size_t index) const
{
    return (*this->data)[index];
}

size_t MappedName::size() const
{
    return this->data->size();
}

bool MappedName::empty() const
{
    return this->data->empty();
}

MappedName::operator bool() const
{
    return !empty();
}

void MappedName::clear()
{
    this->data->clear();
    this->postfixStartIdx = 0;
}

size_t MappedName::find(const char* searchTarget, size_t startPosition) const
{
    return this->data->find(searchTarget, startPosition);
}

size_t MappedName::find(const std::string& searchTarget, size_t startPosition) const
{
    return this->data->find(searchTarget, startPosition);
}

size_t MappedName::rfind(const char* searchTarget, size_t startPosition) const
{
    return this->data->rfind(searchTarget, startPosition);
}

size_t MappedName::rfind(const std::string& searchTarget, size_t startPosition) const
{
    return this->data->rfind(searchTarget, startPosition);
}

/* These are string builtin in c++20 only, have to use boost I guess */
bool MappedName::endsWith(const char* searchTarget) const
{
    return boost::ends_with(*this->data, searchTarget);
}

bool MappedName::endsWith(const std::string& searchTarget) const
{
    return boost::ends_with(*this->data, searchTarget);
}

bool MappedName::startsWith(const char* searchTarget, size_t offset) const
{
    return boost::starts_with(this->data->substr(offset), searchTarget);
}

bool MappedName::startsWith(const std::string& searchTarget, size_t offset) const
{
    return boost::starts_with(this->data->substr(offset), searchTarget);
}
