#pragma once

#include <cube/core/math.hpp>
#include <ctime>
#include <cstdlib>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <3rdparty/simplex/simplex.hpp>
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

// https://github.com/simongeilfus/SimplexNoise/blob/master/NoiseGallery.jpg

namespace cube::gfx
{
    constexpr core::range noise_range{-1.0f, 1.0f};
    constexpr core::range ridged_noise_range{0.0f, 1.0f};
    constexpr core::range ridged_mf_noise_range{0.0f, 1.0f};

    inline void noise_reseed() { Simplex::seed(static_cast<uint32_t>(std::time(nullptr))); }
    inline float noise(float x) { return Simplex::noise(x); }
    inline float noise(glm::vec2 const x) { return Simplex::noise(x); }
    inline float noise(glm::vec3 const x) { return Simplex::noise(x); }
    inline float noise(glm::vec4 const x) { return Simplex::noise(x); }
    inline float ridged_noise(float x) { return Simplex::ridgedNoise(x); }
    inline float ridged_noise(glm::vec2 const x) { return Simplex::ridgedNoise(x); }
    inline float ridged_noise(glm::vec3 const x) { return Simplex::ridgedNoise(x); }
    inline float ridged_noise(glm::vec4 const x) { return Simplex::ridgedNoise(x); }
    inline float ridged_mf_noise(float x) { return Simplex::ridgedMF(x); }
    inline float ridged_mf_noise(glm::vec2 const x) { return Simplex::ridgedMF(x); }
    inline float ridged_mf_noise(glm::vec3 const x) { return Simplex::ridgedMF(x); }
    inline float ridged_mf_noise(glm::vec4 const x) { return Simplex::ridgedMF(x); }
}
