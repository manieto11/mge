#ifndef MGE_WINDOW_H
#define MGE_WINDOW_H

#include <string>

namespace mge
{
    class Window
    {
    public:
        Window(const std::string name, int width, int height);
        ~Window();

        static void Create(const std::string name, int width, int height);
        static bool ShouldClose();
        static void Close();
        static void Destroy();

        static void BeginFrame();
        static void EndFrame();

        static void Draw2D();
        static void Draw3D();

        static void SetBackgroundColor(float r, float g, float b);
    };
}

#endif