#pragma once

#include <cube/gfx/animation_track.h>
#include <list>
#include <boost/noncopyable.hpp>

namespace cube::gfx
{

template <class AnimationTrack>
class animation_player :
    private boost::noncopyable
{
public:
    using animation_track_type = AnimationTrack;
    using animation_track_pointer = typename animation_track_type::pointer;

private:
    static_assert(is_animation_track<AnimationTrack>::value , "AnimationTrack does not meet the requirements");

    std::list<animation_track_pointer> tracks_;
};

}
