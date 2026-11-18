#if !defined(MGE_RENDERER_HPP) && !defined(MGE_DISABLE_RENDERER)
#define MGE_RENDERER_HPP

#include <string>

namespace mge
{
    class Renderer
    {
    public:
        Renderer();
        Renderer(void *window);
        ~Renderer();

        static void Initialize();
        static void Initialize(void *window);
        static void Finalize();

        static void BeginFrame();
        static void EndFrame();

        static void Draw2D();
        static void Draw3D();

        static void SetBackgroundColor(float r, float g, float b);
    };
} // namespace mge

#endif