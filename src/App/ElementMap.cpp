

#include "ElementMap.h"

#include "App/Application.h"
#include "Base/Console.h"

//#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
//#include <boost/io/ios_state.hpp>
//#include <boost/regex.hpp>




#include <unordered_map>


FC_LOG_LEVEL_INIT("ComplexGeoData", true, 2);

namespace Data
{


//FIXME

namespace bio = boost::iostreams;
/** Extract tag and other information from a encoded element name
 *
 * @param name: encoded element name
 * @param tag: optional pointer to receive the extracted tag
 * @param len: optional pointer to receive the length field after the tag field.
 *             This gives the length of the previous hashsed element name starting
 *             from the beginning of the give element name.
 * @param postfix: optional pointer to receive the postfix starting at the found tag field.
 * @param type: optional pointer to receive the element type character
 * @param negative: return negative tag as it is. If disabled, then always return positive tag.
 *                  Negative tag is sometimes used for element disambiguation.
 * @param recursive: recursively find the last non-zero tag
 *
 * @return Return the end position of the tag field, or return -1 if not found.
 */
static int findTagInElementName(const MappedName& name, long* tag = 0, int* len = 0,
                                const char* postfix = 0, char* type = 0, bool negative = false,
                                bool recursive = true)
{
    bool hex = true;
    int pos = name.rfind(ComplexGeoData::tagPostfix());

    // Example name, tagPosfix == ;:H
    // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
    //                                     ^
    //                                     |
    //                                    pos

    if(pos < 0) {
        pos = name.rfind(POSTFIX_DECIMAL_TAG); //FIXME inconsistent
        if (pos < 0)
            return -1;
        hex = false;
    }
    int offset = pos + (int)ComplexGeoData::tagPostfix().size();
    long _tag = 0;
    int _len = 0;
    char sep = 0;
    char sep2 = 0;
    char tp = 0;
    char eof = 0;

    int size;
    const char *s = name.toConstString(offset, size);

    // check if the number followed by the tagPosfix is negative
    bool isNegative = (s[0] == '-');
    if (isNegative) {
        ++s;
        --size;
    }
    bio::stream<bio::array_source> iss(s, size);
    if (!hex) {
        // no hex is an older version of the encoding scheme
        iss >> _tag >> sep;
    } else {
        // The purpose of tag postfix is to encode one model operation. The
        // 'tag' field is used to record the own object ID of that model shape,
        // and the 'len' field indicates the length of the operation codes
        // before the tag postfix. These fields are in hex. The trailing 'F' is
        // the shape type of this element, 'F' for face, 'E' edge, and 'V' vertex.
        //
        // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
        //                     |              |   ^^ ^^
        //                     |              |   |   |  
        //                     ---len = 0x10---  tag len

        iss >> std::hex;
        // _tag field can be skipped, if it is 0
        if (s[0] == ',' || s[0] == ':')
            iss >> sep;
        else
            iss >> _tag >> sep;
    }

    if (isNegative)
        _tag = -_tag;

    if (sep == ':') {
        // ':' is followed by _len field.
        //
        // For decTagPostfix() (i.e. older encoding scheme), this is the length
        // of the string before the entire postfix (A postfix may contain
        // multiple segments usually separated by elementMapPrefix().
        //
        // For newer tagPostfix(), this counts the number of characters that
        // proceeds this tag postfix segment that forms the op code (see
        // example above).
        //
        // The reason of this change is so that the postfix can stay the same
        // regardless of the prefix, which can increase memory efficiency.
        //
        iss >> _len >> sep2 >> tp >> eof;

        // The next separator to look for is either ':' for older tag postfix, or ','
        if (!hex && sep2 == ':')
            sep2 = ',';
    }
    else if (hex && sep == ',') {
        // ',' is followed by a single character that indicates the element type.
        iss >> tp >> eof;
        sep = ':';
        sep2 = ',';
    }

    if (_len < 0 || sep != ':' || sep2 != ',' || tp == 0 || eof != 0)
        return -1;

    if (hex) {
        if (pos-_len < 0)
           return -1;
        if (_len && recursive && (tag || len)) {
            // in case of recursive tag postfix (used by hierarchy element
            // map), look for any embedded tag postifx
            int next = MappedName::fromRawData(name, pos-_len, _len).rfind(ComplexGeoData::tagPostfix());
            if (next >= 0) {
                next += pos - _len;
                // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
                //                     ^               ^
                //                     |               |
                //                    next            pos
                //
                // There maybe other operation codes after this embedded tag
                // postfix, search for the sperator.
                //
                int end;
                if (pos == next)
                    end = -1;
                else
                    end = MappedName::fromRawData(
                        name, next+1, pos-next-1).find(ComplexGeoData::elementMapPrefix());
                if (end >= 0) {
                    end += next+1;
                    // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
                    //                            ^
                    //                            |
                    //                           end
                    _len = pos - end;
                    // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
                    //                            |       |
                    //                            -- len --
                } else
                    _len = 0;
            }
        }

        // Now convert the 'len' field back to the length of the remaining name
        //
        // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
        // |                         |
        // ----------- len -----------
        _len = pos - _len;
    }
    if(type)
        *type = tp;
    if(tag) {
        if (_tag == 0 && recursive)
            return findTagInElementName(
                        MappedName(name, 0, _len), tag, len, postfix, type, negative);
        if(_tag>0 || negative)
            *tag = _tag;
        else
            *tag = -_tag;
    }
    if(len)
        *len = _len;
    if(postfix)
        name.toString(*postfix, pos);
    return pos;
}

















// Because the existence of hierarchical element maps, for the same document
// we may store an element map more than once in multiple objects. And because
// we may want to support partial loading, we choose to tolerate such redundancy
// for now.
//
// In order to not waste memory space when the file is loaded, we use the
// following two maps to assign a one-time id for each unique element map.  The
// id will be saved together with the element map.
//
// When restoring, we'll read back the id and lookup for an existing element map
// with the same id, and skip loading the current map if one is found.
//
// TODO: Note that the same redundancy can be found when saving OCC shapes,
// because we currently save shapes for each object separately. After restoring,
// any shape sharing is lost. But again, we do want to keep separate shape files
// because of partial loading. The same technique used here can be applied to
// restore shape sharing.
static std::unordered_map<const ElementMap*, unsigned> _ElementMapToId;
static std::unordered_map<unsigned, ElementMapPtr> _IdToElementMap;


ElementMap::ElementMap()
{
    static bool inited;
    if (!inited) {
        inited = true;
        ::App::GetApplication().signalStartSaveDocument.connect(
            [](const ::App::Document&, const std::string&) {
                _ElementMapToId.clear();
            });
        ::App::GetApplication().signalFinishSaveDocument.connect(
            [](const ::App::Document&, const std::string&) {
                _ElementMapToId.clear();
            });
        ::App::GetApplication().signalStartRestoreDocument.connect([](const ::App::Document&) {
            _IdToElementMap.clear();
        });
        ::App::GetApplication().signalFinishRestoreDocument.connect([](const ::App::Document&) {
            _IdToElementMap.clear();
        });
    }
}

void ElementMap::beforeSave(const ::App::StringHasherRef& hasher) const
{
    unsigned& id = _ElementMapToId[this];
    if (!id)
        id = _ElementMapToId.size();
    this->_id = id;

    for (auto& v : this->indexedNames) {
        for (const MappedNameRef& ref : v.second.names) {
            for (const MappedNameRef* r = &ref; r; r = r->next.get()) {
                for (const ::App::StringIDRef& sid : r->sids) {
                    if (sid.isFromSameHasher(hasher))
                        sid.mark();
                }
            }
        }
        for (auto& vv : v.second.children) {
            if (vv.second.elementMap)
                vv.second.elementMap->beforeSave(hasher);
            for (auto& sid : vv.second.sids) {
                if (sid.isFromSameHasher(hasher))
                    sid.mark();
            }
        }
    }
}

const MappedNameRef* ElementMap::findMappedRef(const IndexedName& idx) const
{
    auto iter = this->indexedNames.find(idx.getType());
    if (iter == this->indexedNames.end())
        return nullptr;
    auto& indices = iter->second;
    if (idx.getIndex() >= (int)indices.names.size())
        return nullptr;
    return &indices.names[idx.getIndex()];
}

MappedNameRef* ElementMap::findMappedRef(const IndexedName& idx)
{
    auto iter = this->indexedNames.find(idx.getType());
    if (iter == this->indexedNames.end())
        return nullptr;
    auto& indices = iter->second;
    if (idx.getIndex() >= (int)indices.names.size())
        return nullptr;
    return &indices.names[idx.getIndex()];
}

MappedNameRef& ElementMap::mappedRef(const IndexedName& idx)
{
    assert(idx);
    auto& indices = this->indexedNames[idx.getType()];
    if (idx.getIndex() >= (int)indices.names.size())
        indices.names.resize(idx.getIndex() + 1);
    return indices.names[idx.getIndex()];
}

void ElementMap::addPostfix(const QByteArray& postfix, std::map<QByteArray, int>& postfixMap,
                       std::vector<QByteArray>& postfixes)
{
    if (postfix.isEmpty())
        return;
    auto res = postfixMap.insert(std::make_pair(postfix, 0));
    if (res.second) {
        postfixes.push_back(postfix);
        res.first->second = (int)postfixes.size();
    }
}

void ElementMap::collectChildMaps(std::map<const ElementMap*, int>& childMapSet,
                      std::vector<const ElementMap*>& childMaps,
                      std::map<QByteArray, int>& postfixMap,
                      std::vector<QByteArray>& postfixes) const
{
    auto res = childMapSet.insert(std::make_pair(this, 0));
    if (!res.second)
        return;

    for (auto& v : this->indexedNames) {
        addPostfix(QByteArray::fromRawData(v.first, qstrlen(v.first)), postfixMap, postfixes);

        for (auto& vv : v.second.children) {
            auto& child = vv.second;
            if (child.elementMap)
                child.elementMap->collectChildMaps(childMapSet, childMaps, postfixMap, postfixes);
        }
    }

    for (auto& v : this->mappedNames)
        addPostfix(v.first.constPostfix(), postfixMap, postfixes);

    childMaps.push_back(this);
    res.first->second = (int)childMaps.size();
}

void ElementMap::save(std::ostream& s, int index, const std::map<const ElementMap*, int>& childMapSet,
          const std::map<QByteArray, int>& postfixMap) const
{
    s << "\nElementMap " << index << ' ' << this->_id << ' ' << this->indexedNames.size() << '\n';

    for (auto& v : this->indexedNames) {
        s << '\n' << v.first << '\n';

        s << "\nChildCount " << v.second.children.size() << '\n';
        for (auto& vv : v.second.children) {
            auto& child = vv.second;
            int mapIndex = 0;
            if (child.elementMap) {
                auto it = childMapSet.find(child.elementMap.get());
                if (it == childMapSet.end() || it->second == 0)
                    FC_ERR("Invalid child element map");
                else
                    mapIndex = it->second;
            }
            s << child.indexedName.getIndex() << ' ' << child.offset << ' ' << child.count << ' '
              << child.tag << ' ' << mapIndex << ' ';
            s.write(child.postfix.constData(), child.postfix.size());
            s << ' ' << '0';
            for (auto& sid : child.sids) {
                if (sid.isMarked())
                    s << '.' << sid.value();
            }
            s << '\n';
        }

        s << "\nNameCount " << v.second.names.size() << '\n';
        if (v.second.names.empty())
            continue;

        boost::io::ios_flags_saver ifs(s);
        s << std::hex;

        for (auto& ref : v.second.names) {
            for (auto r = &ref; r; r = r->next.get()) {
                if (!r->name)
                    break;

                ::App::StringID::IndexID prefixid;
                prefixid.id = 0;
                IndexedName idx(r->name.dataBytes());
                bool printName = true;
                if (idx) {
                    auto key = QByteArray::fromRawData(idx.getType(), qstrlen(idx.getType()));
                    auto it = postfixMap.find(key);
                    if (it != postfixMap.end()) {
                        s << ':' << it->second << '.' << idx.getIndex();
                        printName = false;
                    }
                }
                else {
                    prefixid = ::App::StringID::fromString(r->name.dataBytes());
                    if (prefixid.id) {
                        for (auto& sid : r->sids) {
                            if (sid.isMarked() && sid.value() == prefixid.id) {
                                s << '$';
                                s.write(r->name.dataBytes().constData(), r->name.dataBytes().size());
                                printName = false;
                                break;
                            }
                        }
                        if (printName)
                            prefixid.id = 0;
                    }
                }
                if (printName) {
                    s << ';';
                    s.write(r->name.dataBytes().constData(), r->name.dataBytes().size());
                }

                const QByteArray& postfix = r->name.postfixBytes();
                if (postfix.isEmpty())
                    s << ".0";
                else {
                    auto it = postfixMap.find(postfix);
                    assert(it != postfixMap.end());
                    s << '.' << it->second;
                }
                for (auto& sid : r->sids) {
                    if (sid.isMarked() && sid.value() != prefixid.id)
                        s << '.' << sid.value();
                }

                s << ' ';
            }
            s << "0\n";
        }
    }
    s << "\nEndMap\n";
}

void ElementMap::save(std::ostream& s) const
{
    std::map<const ElementMap*, int> childMapSet;
    std::vector<const ElementMap*> childMaps;
    std::map<QByteArray, int> postfixMap;
    std::vector<QByteArray> postfixes;

    collectChildMaps(childMapSet, childMaps, postfixMap, postfixes);

    s << this->_id << " PostfixCount " << postfixes.size() << '\n';
    for (auto& p : postfixes){
        s.write(p.constData(), p.size());
        s << '\n';
    }
    int index = 0;
    s << "\nMapCount " << childMaps.size() << '\n';
    for (auto& elementMap : childMaps)
        elementMap->save(s, ++index, childMapSet, postfixMap);
}

ElementMapPtr ElementMap::restore(::App::StringHasherRef hasher, std::istream& s)
{
    const char* msg = "Invalid element map";

    unsigned id;
    int count = 0;
    std::string tmp;
    if (!(s >> id >> tmp >> count) || tmp != "PostfixCount")
        FC_THROWM(Base::RuntimeError, msg);

    auto& map = _IdToElementMap[id];
    if (map)
        return map;

    std::vector<std::string> postfixes;
    postfixes.reserve(count);
    for (int i = 0; i < count; ++i) {
        postfixes.emplace_back();
        s >> postfixes.back();
    }

    std::vector<ElementMapPtr> childMaps;
    count = 0;
    if (!(s >> tmp >> count) || tmp != "MapCount" || count == 0)
        FC_THROWM(Base::RuntimeError, msg);
    childMaps.reserve(count - 1);
    for (int i = 0; i < count - 1; ++i) {
        childMaps.push_back(
            std::make_shared<ElementMap>()->restore(hasher, s, childMaps, postfixes));
    }

    return restore(hasher, s, childMaps, postfixes);
}

ElementMapPtr ElementMap::restore(::App::StringHasherRef hasher, std::istream& s,
                      std::vector<ElementMapPtr>& childMaps,
                      const std::vector<std::string>& postfixes)
{
    const char* msg = "Invalid element map";
    std::string tmp;
    int index = 0;
    int typeCount = 0;
    unsigned id = 0;
    if (!(s >> tmp >> index >> id >> typeCount) || tmp != "ElementMap")
        FC_THROWM(Base::RuntimeError, msg);

    auto& map = _IdToElementMap[id];
    if (map) {
        do {
            if (!std::getline(s, tmp))
                FC_THROWM(Base::RuntimeError, "unexpected end of child element map");
        } while (tmp != "EndMap");
        return map;
    }
    map = shared_from_this();

    const char* hasherWarn = nullptr;
    const char* hasherIDWarn = nullptr;
    const char* postfixWarn = nullptr;
    const char* childSIDWarn = nullptr;
    std::vector<std::string> tokens;

    for (int i = 0; i < typeCount; ++i) {
        int count;
        if (!(s >> tmp))
            FC_THROWM(Base::RuntimeError, "missing element type");
        IndexedName idx(tmp.c_str(), 1);

        if (!(s >> tmp >> count) || tmp != "ChildCount")
            FC_THROWM(Base::RuntimeError, "missing element child count");

        auto& indices = this->indexedNames[idx.getType()];
        for (int j = 0; j < count; ++j) {
            int cindex;
            int offset;
            int count;
            long tag;
            int mapIndex;
            if (!(s >> cindex >> offset >> count >> tag >> mapIndex >> tmp))
                FC_THROWM(Base::RuntimeError, "Invalid element child");
            if (cindex < 0)
                FC_THROWM(Base::RuntimeError, "Invalid element child index");
            if (offset < 0)
                FC_THROWM(Base::RuntimeError, "Invalid element child offset");
            if (mapIndex >= index || mapIndex < 0 || mapIndex > (int)childMaps.size())
                FC_THROWM(Base::RuntimeError, "Invalid element child map index");
            auto& child = indices.children[cindex + offset + count];
            child.indexedName = IndexedName::fromConst(idx.getType(), cindex);
            child.offset = offset;
            child.count = count;
            child.tag = tag;
            if (mapIndex > 0)
                child.elementMap = childMaps[mapIndex - 1];
            else
                child.elementMap = nullptr;
            child.postfix = tmp.c_str();
            this->childElements[child.postfix].childMap = &child;
            this->childElementSize += child.count;

            if (!(s >> tmp))
                FC_THROWM(Base::RuntimeError, "Invalid element child string id");

            tokens.clear();
            boost::split(tokens, tmp, boost::is_any_of("."));
            if (tokens.size() > 1) {
                child.sids.reserve(tokens.size() - 1);
                for (unsigned k = 1; k < tokens.size(); ++k) {
                    // The element child string ID is saved as decimal
                    // instead of hex by accident. To simplify maintenance
                    // of backward compatibility, it is not corrected, and
                    // just restored as decimal here.
                    //
                    // long n = strtol(tokens[k].c_str(), nullptr, 16);
                    long n = strtol(tokens[k].c_str(), nullptr, 10);
                    auto sid = hasher->getID(n);
                    if (!sid)
                        childSIDWarn = "Missing element child string id";
                    else
                        child.sids.push_back(sid);
                }
            }
        }

        if (!(s >> tmp >> count) || tmp != "NameCount")
            FC_THROWM(Base::RuntimeError, "missing element name count");

        boost::io::ios_flags_saver ifs(s);
        s >> std::hex;

        indices.names.resize(count);
        for (int j = 0; j < count; ++j) {
            idx.setIndex(j);
            auto* ref = &indices.names[j];
            int k = 0;
            while (1) {
                if (!(s >> tmp))
                    FC_THROWM(Base::RuntimeError, "Failed to read element name");
                if (tmp == "0")
                    break;
                if (k++ != 0) {
                    ref->next.reset(new MappedNameRef);
                    ref = ref->next.get();
                }
                tokens.clear();
                boost::split(tokens, tmp, boost::is_any_of("."));
                if (tokens.size() < 2)
                    FC_THROWM(Base::RuntimeError, "Invalid element entry");

                int offset = 1;
                ::App::StringID::IndexID prefixid;
                prefixid.id = 0;

                switch (tokens[0][0]) {
                    case ':': {
                        if (tokens.size() < 3)
                            FC_THROWM(Base::RuntimeError, "Invalid element entry");
                        ++offset;
                        long n = strtol(tokens[0].c_str() + 1, nullptr, 16);
                        if (n <= 0 || n > (int)postfixes.size())
                            FC_THROWM(Base::RuntimeError, "Invalid element name index");
                        long m = strtol(tokens[1].c_str(), nullptr, 16);
                        ref->name = MappedName(IndexedName::fromConst(postfixes[n - 1].c_str(), m));
                        break;
                    }
                    case '$':
                        ref->name = MappedName(tokens[0].c_str() + 1);
                        prefixid = ::App::StringID::fromString(ref->name.dataBytes());
                        break;
                    case ';':
                        ref->name = MappedName(tokens[0].c_str() + 1);
                        break;
                    default:
                        FC_THROWM(Base::RuntimeError, "Invalid element name marker");
                }

                if (tokens[offset] != "0") {
                    long n = strtol(tokens[offset].c_str(), nullptr, 16);
                    if (n <= 0 || n > (int)postfixes.size())
                        postfixWarn = "Invalid element postfix index";
                    else
                        ref->name += postfixes[n - 1];
                }

                this->mappedNames.emplace(ref->name, idx);

                if (!hasher) {
                    if (offset + 1 < (int)tokens.size())
                        hasherWarn = "No hasher";
                    continue;
                }

                ref->sids.reserve(tokens.size() - offset - 1 + prefixid.id ? 1 : 0);
                if (prefixid.id) {
                    auto sid = hasher->getID(prefixid.id);
                    if (!sid)
                        hasherIDWarn = "Missing element name prefix id";
                    else
                        ref->sids.push_back(sid);
                }
                for (int l = offset + 1; l < (int)tokens.size(); ++l) {
                    long id = strtol(tokens[l].c_str(), nullptr, 16);
                    auto sid = hasher->getID(id);
                    if (!sid)
                        hasherIDWarn = "Invalid element name string id";
                    else
                        ref->sids.push_back(sid);
                }
            }
        }
    }
    if (hasherWarn)
        FC_WARN(hasherWarn);
    if (hasherIDWarn)
        FC_WARN(hasherIDWarn);
    if (postfixWarn)
        FC_WARN(postfixWarn);
    if (childSIDWarn)
        FC_WARN(childSIDWarn);

    if (!(s >> tmp) || tmp != "EndMap")
        FC_THROWM(Base::RuntimeError, "unexpected end of child element map");

    return shared_from_this();
}

MappedName ElementMap::addName(MappedName& name, const IndexedName& idx, const ElementIDRefs& sids,
                   bool overwrite, IndexedName* existing)
{
    if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
        if (name.find("#") >= 0 && findTagInElementName(name) < 0) {
            FC_ERR("missing tag postfix " << name);
        }
    }
    do {
        if (overwrite)
            erase(idx);
        auto ret = mappedNames.insert(std::make_pair(name, idx));
        if (ret.second) {
            ret.first->first.compact();
            mappedRef(idx).append(ret.first->first, sids);
            FC_TRACE(idx << " -> " << name);
            return ret.first->first;
        }
        if (ret.first->second == idx) {
            FC_TRACE("duplicate " << idx << " -> " << name);
            return ret.first->first;
        }
        if (!overwrite) {
            if (existing)
                *existing = ret.first->second;
            return MappedName();
        }

        erase(ret.first->first);
    } while (true);
}

bool ElementMap::erase(const MappedName& name)
{
    auto it = this->mappedNames.find(name);
    if (it == this->mappedNames.end())
        return false;
    MappedNameRef* ref = findMappedRef(it->second);
    if (!ref)
        return false;
    ref->erase(name);
    this->mappedNames.erase(it);
    return true;
}

bool ElementMap::erase(const IndexedName& idx)
{
    auto iter = this->indexedNames.find(idx.getType());
    if (iter == this->indexedNames.end())
        return false;
    auto& indices = iter->second;
    if (idx.getIndex() >= (int)indices.names.size())
        return false;
    auto& ref = indices.names[idx.getIndex()];
    for (auto* r = &ref; r; r = r->next.get())
        this->mappedNames.erase(r->name);
    ref.clear();
    return true;
}

IndexedName ElementMap::find(const MappedName& name, ElementIDRefs* sids) const
{
    auto it = mappedNames.find(name);
    if (it == mappedNames.end()) {
        if (childElements.isEmpty())
            return IndexedName();

        int len = 0;
        if (findTagInElementName(
                name, nullptr, &len, nullptr, nullptr, false, false)
            < 0)
            return IndexedName();
        QByteArray key = name.toRawBytes(len);
        auto it = this->childElements.find(key);
        if (it == this->childElements.end())
            return IndexedName();

        const auto& child = *it.value().childMap;
        IndexedName res;

        MappedName childName = MappedName::fromRawData(name, 0, len);
        if (child.elementMap)
            res = child.elementMap->find(childName, sids);
        else
            res = childName.toIndexedName();

        if (res && boost::equals(res.getType(), child.indexedName.getType())
            && child.indexedName.getIndex() <= res.getIndex()
            && child.indexedName.getIndex() + child.count > res.getIndex()) {
            res.setIndex(res.getIndex() + it.value().childMap->offset);
            return res;
        }

        return IndexedName();
    }

    if (sids) {
        const MappedNameRef* ref = findMappedRef(it->second);
        for (; ref; ref = ref->next.get()) {
            if (ref->name == name) {
                if (!sids->size())
                    *sids = ref->sids;
                else
                    *sids += ref->sids;
                break;
            }
        }
    }
    return it->second;
}

MappedName ElementMap::find(const IndexedName& idx, ElementIDRefs* sids) const
{
    if (!idx)
        return MappedName();

    auto iter = this->indexedNames.find(idx.getType());
    if (iter == this->indexedNames.end())
        return MappedName();

    auto& indices = iter->second;
    if (idx.getIndex() < (int)indices.names.size()) {
        const MappedNameRef& ref = indices.names[idx.getIndex()];
        if (ref.name) {
            if (sids) {
                if (!sids->size())
                    *sids = ref.sids;
                else
                    *sids += ref.sids;
            }
            return ref.name;
        }
    }

    auto it = indices.children.upper_bound(idx.getIndex());
    if (it != indices.children.end()
        && it->second.indexedName.getIndex() + it->second.offset <= idx.getIndex()) {
        auto& child = it->second;
        MappedName name;
        IndexedName childIdx(idx.getType(), idx.getIndex() - child.offset);
        if (child.elementMap)
            name = child.elementMap->find(childIdx, sids);
        else
            name = MappedName(childIdx);
        if (name) {
            name += child.postfix;
            return name;
        }
    }
    return MappedName();
}

std::vector<std::pair<MappedName, ElementIDRefs>> ElementMap::findAll(const IndexedName& idx) const
{
    std::vector<std::pair<MappedName, ElementIDRefs>> res;
    if (!idx)
        return res;

    auto iter = this->indexedNames.find(idx.getType());
    if (iter == this->indexedNames.end())
        return res;

    auto& indices = iter->second;
    if (idx.getIndex() < (int)indices.names.size()) {
        const MappedNameRef& ref = indices.names[idx.getIndex()];
        int count = 0;
        for (auto r = &ref; r; r = r->next.get()) {
            if (r->name)
                ++count;
        }
        if (count) {
            res.reserve(count);
            for (auto r = &ref; r; r = r->next.get()) {
                if (r->name)
                    res.emplace_back(r->name, r->sids);
            }
            return res;
        }
    }

    auto it = indices.children.upper_bound(idx.getIndex());
    if (it != indices.children.end()
        && it->second.indexedName.getIndex() + it->second.offset <= idx.getIndex()) {
        auto& child = it->second;
        IndexedName childIdx(idx.getType(), idx.getIndex() - child.offset);
        if (child.elementMap) {
            res = child.elementMap->findAll(childIdx);
            for (auto& v : res)
                v.first += child.postfix;
        }
        else
            res.emplace_back(MappedName(childIdx) + child.postfix, ElementIDRefs());
    }

    return res;
}



unsigned long ElementMap::size() const
{
    return mappedNames.size() + childElementSize;
}

bool ElementMap::empty() const
{
    return mappedNames.empty() && childElementSize == 0;
}

bool ElementMap::hasChildElementMap() const
{
    return !childElements.empty();
}

void ElementMap::hashChildMaps(ComplexGeoData& master)
{
    if (childElements.empty() || !master.Hasher)
        return;
    std::ostringstream ss;
    for (auto& v : this->indexedNames) {
        for (auto& vv : v.second.children) {
            auto& child = vv.second;
            int len = 0;
            long tag;
            int pos = findTagInElementName(
                MappedName::fromRawData(child.postfix), &tag, &len, nullptr, nullptr, false, false);
            if (pos > 10) {
                MappedName postfix = master.hashElementName(
                    MappedName::fromRawData(child.postfix.constData(), pos), child.sids);
                ss.str("");
                ss << MappedChildElements::prefix() << postfix;
                MappedName tmp;
                master.encodeElementName(
                    child.indexedName[0], tmp, ss, nullptr, nullptr, child.tag, true);
                this->childElements.remove(child.postfix);
                child.postfix = tmp.toBytes();
                this->childElements[child.postfix].childMap = &child;
            }
        }
    }
}

void ElementMap::addChildElements(ComplexGeoData& master, const std::vector<MappedChildElements>& children)
{
    std::ostringstream ss;
    ss << std::hex;

    // To avoid possibly very long recursive child map lookup, resulting very
    // long mapped names, we try to resolve the grand child map now.
    std::vector<MappedChildElements> expansion;
    for (auto it = children.begin(); it != children.end(); ++it) {
        auto& child = *it;
        if (!child.elementMap || child.elementMap->childElements.empty()) {
            if (expansion.size())
                expansion.push_back(child);
            continue;
        }
        auto& indices = child.elementMap->indexedNames[child.indexedName.getType()];
        if (indices.children.empty()) {
            if (expansion.size())
                expansion.push_back(child);
            continue;
        }

        // Note that it is allow to have both mapped names and child map. We
        // may have to split the current child mapping into pieces.

        int start = child.indexedName.getIndex();
        int end = start + child.count;
        for (auto iter = indices.children.upper_bound(start); iter != indices.children.end();
             ++iter) {
            auto& grandchild = iter->second;
            int istart = grandchild.indexedName.getIndex() + grandchild.offset;
            int iend = istart + grandchild.count;
            if (end <= istart)
                break;
            if (istart >= end) {
                if (expansion.size()) {
                    expansion.push_back(child);
                    expansion.back().indexedName.setIndex(start);
                    expansion.back().count = end - start;
                }
                break;
            }
            if (expansion.empty()) {
                expansion.reserve(children.size() + 10);
                expansion.insert(expansion.end(), children.begin(), it);
            }
            expansion.push_back(child);
            auto* entry = &expansion.back();
            if (istart > start) {
                entry->indexedName.setIndex(start);
                entry->count = istart - start;

                expansion.push_back(child);
                entry = &expansion.back();
            }
            else
                istart = start;

            if (iend > end)
                iend = end;

            entry->indexedName.setIndex(istart - grandchild.offset);
            entry->count = iend - istart;
            entry->offset += grandchild.offset;
            entry->elementMap = grandchild.elementMap;
            entry->sids += grandchild.sids;
            if (grandchild.postfix.size()) {
                if (entry->postfix.size()
                    && !entry->postfix.startsWith(ComplexGeoData::elementMapPrefix().c_str())) {
                    entry->postfix = grandchild.postfix + ComplexGeoData::elementMapPrefix().c_str()
                        + entry->postfix;
                }
                else
                    entry->postfix = grandchild.postfix + entry->postfix;
            }

            start = iend;
            if (start >= end)
                break;
        }
        if (expansion.size() && start < end) {
            expansion.push_back(child);
            expansion.back().indexedName.setIndex(start);
            expansion.back().count = end - start;
        }
    }

    for (auto& child : expansion.size() ? expansion : children) {
        if (!child.indexedName || !child.count) {
            if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
                FC_ERR("invalid mapped child element");
            continue;
        }

        ss.str("");
        MappedName tmp;

        ChildMapInfo* entry = nullptr;

        // do child mapping only if the child element count >= 5
        if (child.count >= 5 || !child.elementMap) {
            master.encodeElementName(
                child.indexedName[0], tmp, ss, nullptr, child.postfix.constData(), child.tag, true);

            // Perform some disambiguation in case the same shape is mapped
            // multiple times, e.g. draft array.
            entry = &childElements[tmp.toBytes()];
            int mapIndex = entry->mapIndices[child.elementMap.get()]++;
            ++entry->index;
            if (entry->index != 1 && child.elementMap && mapIndex == 0) {
                // This child has duplicated 'tag' and 'postfix', but it
                // has its own element map. We'll expand this map now.
                entry = nullptr;
            }
        }

        if (!entry) {
            IndexedName childIdx(child.indexedName);
            IndexedName idx(childIdx.getType(), childIdx.getIndex() + child.offset);
            for (int i = 0; i < child.count; ++i, ++childIdx, ++idx) {
                ElementIDRefs sids;
                MappedName name = child.elementMap->find(childIdx, &sids);
                if (!name) {
                    if (!child.tag || child.tag == master.Tag) {
                        if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
                            FC_WARN("unmapped element");
                        continue;
                    }
                    name = MappedName(childIdx);
                }
                ss.str("");
                master.encodeElementName(
                    idx[0], name, ss, &sids, child.postfix.constData(), child.tag);
                master.setElementName(idx, name, &sids);
            }
            continue;
        }

        if (entry->index != 1) {
            // There is some ambiguity in child mapping. We need some
            // additional postfix for disambiguation. NOTE: We are not
            // using ComplexGeoData::indexPostfix() so as to not confuse
            // other code that actually uses this postfix for indexing
            // purposes. Here, we just need some postfix for
            // disambiguation. We don't need to extract the index.
            ss.str("");
            ss << ComplexGeoData::elementMapPrefix() << ":C" << entry->index - 1;

            tmp.clear();
            master.encodeElementName(
                child.indexedName[0], tmp, ss, nullptr, child.postfix.constData(), child.tag, true);

            entry = &childElements[tmp.toBytes()];
            if (entry->childMap) {
                FC_ERR("duplicate mapped child element");
                continue;
            }
        }

        auto& indices = this->indexedNames[child.indexedName.getType()];
        auto res = indices.children.emplace(
            child.indexedName.getIndex() + child.offset + child.count, child);
        if (!res.second) {
            if (!entry->childMap)
                this->childElements.remove(tmp.toBytes());
            FC_ERR("duplicate mapped child element");
            continue;
        }

        auto& insertedChild = res.first->second;
        insertedChild.postfix = tmp.toBytes();
        entry->childMap = &insertedChild;
        childElementSize += insertedChild.count;
    }
}

std::vector<MappedChildElements> ElementMap::getChildElements() const
{
    std::vector<MappedChildElements> res;
    res.reserve(this->childElements.size());
    for (auto& v : this->childElements)
        res.push_back(*v.childMap);
    return res;
}

std::vector<MappedElement> ElementMap::getAll() const
{
    std::vector<MappedElement> ret;
    ret.reserve(size());
    for (auto& v : this->mappedNames)
        ret.emplace_back(v.first, v.second);
    for (auto& v : this->childElements) {
        auto& child = *v.childMap;
        IndexedName idx(child.indexedName);
        idx.setIndex(idx.getIndex() + child.offset);
        IndexedName childIdx(child.indexedName);
        for (int i = 0; i < child.count; ++i, ++idx, ++childIdx) {
            MappedName name;
            if (child.elementMap)
                name = child.elementMap->find(childIdx);
            else
                name = MappedName(childIdx);
            if (name) {
                name += child.postfix;
                ret.emplace_back(name, idx);
            }
        }
    }
    return ret;
}


}// Namespace Data
