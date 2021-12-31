#pragma once

#include <memory>

namespace cube::core
{

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
