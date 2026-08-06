#ifndef MGE_WINDOW_H
#define MGE_WINDOW_H

namespace mge
{
    class Window
    {
    public:
        static bool Create(const char *name, int width, int height);
        static bool ShouldClose();
        static void Close();

        static void BeginFrame();
        static void EndFrame();

        static void Draw2D();
        static void Draw3D();

        static void SetBackgroundColor(float r, float g, float b);
    };
}

#endif