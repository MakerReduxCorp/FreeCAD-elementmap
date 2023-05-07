#pragma once

#include <memory>


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

    const T& get() const { return *handle; }
    T& get() { createLocalCopy(); return *handle; }

    const T& operator*() const { return *handle; }
    T& operator*() { createLocalCopy(); return *handle; }

    const T* operator->() const { return &(*handle); }
    T* operator->() { createLocalCopy(); return &(*handle); }

    operator const T&() const { return *handle; }
    operator T&() { createLocalCopy(); return *handle; }

private:
    std::shared_ptr<T> handle;
    bool owner = false;
};

//TODO test implicit bool conversion
//TODO T and shared_ptr<T> move constructors/assignment