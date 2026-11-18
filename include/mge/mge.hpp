#if !defined(MGE_HPP)
#define MGE_HPP

#include "mge/audio.hpp"
#include "mge/exceptions.hpp"
#include "mge/input.hpp"
#include "mge/physics.hpp"
#include "mge/renderer.hpp"
#include "mge/window.hpp"

namespace mge
{
    void InitializeEngine();
    void FinalizeEngine();
} // namespace mge


#endif