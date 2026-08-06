#ifndef MGE_PHYSICS_H
#define MGE_PHYSICS_H

// #define DIM2D

namespace mge
{
#ifdef DIM2D
    class Physics2D
    {
    public:
        bool Init();
    };
#else
    class Physics3D
    {
    public:
        bool Init();
    };
#endif
}

#endif