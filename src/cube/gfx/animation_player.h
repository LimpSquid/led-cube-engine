#pragma once

#include <gfx/animation_track.h>
#include <core/engine.h>
#include <list>
#include <type_traits>
#include <boost/noncopyable.hpp>

namespace cube::gfx
{

template <class AnimationTrack>
class animation_player : private boost::noncopyable
{
private:
template<class, class = void>
struct is_animation_track : std::false_type { };

template<class T>
struct is_animation_track<T, std::void_t<decltype(
    std::declval<typename T::pointer>(),
    std::declval<T>().start(),
    std::declval<T>().stop(),
    std::declval<T>().pause(),
    std::declval<T>().is_stopped(),
    std::declval<T>().is_running(),
    std::declval<T>().is_paused(),
    std::declval<T>().is_finished()
)>> : std::true_type { };

public:
    using animation_track_type = AnimationTrack;
    using animation_track_pointer = typename animation_track_type::pointer;

    animation_player() = default;
    ~animation_player() = default;

private:
    static_assert(is_animation_track<AnimationTrack>::value , "AnimationTrack does not meet the requirements");

    std::list<animation_track_pointer> tracks_;
};

}