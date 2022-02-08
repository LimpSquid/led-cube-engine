#pragma once

#include <memory>
#include <unistd.h>

namespace cube::core
{

class safe_fd
{
public:
    safe_fd() :
        fd(-1)
    { }

    safe_fd(int descriptor) :
        fd(descriptor)
    { }

    safe_fd(safe_fd && other) :
        fd(other.fd)
    {
        other.fd = -1;
    }

    ~safe_fd()
    {
        reset();
    }

    void reset(int descriptor = -1)
    {
        // Close old fd before assigning new one
        if (fd >= 0)
            ::close(fd);

        fd = descriptor;
    }

    void swap(safe_fd & other)
    {
        std::swap(fd, other.fd);
    }

    operator int() const
    {
        return fd;
    }

    safe_fd & operator=(int descriptor)
    {
        reset(descriptor);
        return *this;
    }

private:
    int fd;
};

using parent_tracker_t = std::shared_ptr<void *>;

inline parent_tracker_t make_parent_tracker()
{
    return std::make_shared<void *>(nullptr);
}

inline parent_tracker_t::weak_type weak(parent_tracker_t tracker)
{
    return {tracker};
}

inline bool parent_in_scope(parent_tracker_t::weak_type weak)
{
    return weak.lock() != nullptr;
}

} // End of namespace
