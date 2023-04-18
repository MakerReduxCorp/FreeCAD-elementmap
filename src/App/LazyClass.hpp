#pragma once

#include <memory>


template <typename T>
class Lazy {
public:
    Lazy() = default;

    explicit Lazy(const T& resource) : handle(std::make_shared<T>(resource)), owner(true) {}
    explicit Lazy(const std::shared_ptr<T>& resource) : handle(resource) {}

    Lazy(const Lazy& other) : handle(other.handle), owner(false) {}
    Lazy& operator=(const Lazy& other) 
    {
        handle = other.handle;
        owner = false;
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
            handle.reset(std::make_shared<T>(*handle));
            owner = true;
        }
    }

    bool hasLocalCopy() const { return owner; }
    
    operator const T&() const { return *handle; }
    operator T&()
    {
        createLocalCopy();
        return *handle;
    }

private:
    std::shared_ptr<T> handle;
    bool owner = false;
};
