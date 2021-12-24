#pragma once

#include <memory>

namespace cube::core
{

using parent_tracker = std::shared_ptr<void *>;

inline parent_tracker make_parent_tracker()
{
    return std::make_shared<void *>(nullptr);
}

inline parent_tracker::weak_type weak(parent_tracker tracker)
{
    return {tracker};
}

inline bool parent_in_scope(parent_tracker::weak_type weak)
{
    return weak.lock() != nullptr;
}

} // End of namespace
