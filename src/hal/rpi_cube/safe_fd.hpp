#pragma once

#include <unistd.h>

namespace hal::rpi_cube
{

struct safe_fd
{
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
        if (fd != -1)
            ::close(fd);
    }

    operator int() const
    {
        return fd;
    }

    int fd;
};

} // End of namespace
