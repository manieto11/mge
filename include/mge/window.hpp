#if !defined(MGE_WINDOW_HPP) && !defined(MGE_DISABLE_WINDOW)
#define MGE_WINDOW_HPP

#include "glm/vec2.hpp"

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

        static int GetWidth();
        static void SetWidth(int width);

        static int GetHeight();
        static void SetHeight(int height);

        static glm::ivec2 GetSize();
        static void SetSize(glm::ivec2 size);

        static glm::ivec2 GetPosition();
        static void SetPosition(glm::ivec2 position);
    };
} // namespace mge

#endif