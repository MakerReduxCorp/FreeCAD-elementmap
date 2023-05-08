#pragma once

#include <memory>

// The idea is similar to this: https://github.com/bitwizeshift/Lazy
// but insread of deferring object construction we are deferring copy.
// The object T wrapped by this class is referenced through a shared_ptr, 
// so that copies of Lazy just increment the refcount instead of preforming
// a full copy. In case the object T needs to get modified a full copy  
// is performed, to avoid modifying the other copies owned by the shared_ptr.
//
// My hope was that the compiler was smart enough to deduce when it could call
// const methods and when it had no choice but to call the non-const variant.
// It is not. To work around this all access operators have been commented out, 
// leaving just two functions: asConst() and asMutable(). This is way more verbose
// to type, but also makes the behavior of the class extremely explicit and transparent. 
// There's no guessing when a copy will be made.

template <typename T>
class Lazy {
public:
    Lazy() : handle(std::make_shared<T>()), owner(true) {};

    Lazy(const T& resource) : handle(std::make_shared<T>(resource)), owner(true) {}
    Lazy& operator=(const T& resource) 
    {
        handle = std::make_shared<T>(resource);
        owner = true;
        return *this;
    }

    Lazy(const std::shared_ptr<T>& resource) : handle(resource), owner(false) {}
    Lazy& operator=(const std::shared_ptr<T>& resource) 
    {
        handle = resource;
        owner = false;
        return *this;
    }

    Lazy(const Lazy& other) : handle(other.handle), owner(false) {}
    Lazy& operator=(const Lazy& other) 
    {
        handle = other.handle;
        owner = false;
        return *this;
    }

    Lazy(Lazy&& other) noexcept : handle(std::move(other.handle)), owner(other.owner) {}
    Lazy& operator=(Lazy&& other) noexcept
    {
        this->handle = std::move(other.handle);
        this->owner = other.owner;
        return *this;
    }

    ~Lazy() = default;

    void createLocalCopy()
    {
        if (!owner)
        {
            handle = std::make_shared<T>(*handle);
            owner = true;
        }
    }

    bool hasLocalCopy() const { return owner; }

    // Access the wrapped object in a non-modifying fashion. No copy will be made (unless
    // a local copy was already made beforehand)
    const T& asConst() const { return *handle; }

    // Access the wrapped object to modify it. Ensures that a local copy has been
    // made beforehand, so that the other references owned do not get modified.
    T& asMutable() { createLocalCopy(); return *handle; }

/*
    const T& get() const { return *handle; }
    T& get() { createLocalCopy(); return *handle; }

    const T& operator*() const { return *handle; }
    T& operator*() { createLocalCopy(); return *handle; }

    const T* operator->() const { return &(*handle); }
    T* operator->() { createLocalCopy(); return &(*handle); }

    operator const T&() const { return *handle; }
    operator T&() { createLocalCopy(); return *handle; }
*/

private:
    std::shared_ptr<T> handle;
    bool owner = false;
};

//TODO T and shared_ptr<T> move constructors/assignment
//TODO LazyString = const char* does not work, but LazyString(const char*) does