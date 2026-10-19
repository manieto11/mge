#if !defined(MGE_INPUT_HPP)
#define MGE_INPUT_HPP

#include "glm/vec2.hpp"
#include "input/keyboardKey.hpp"
#include "input/mouseButton.hpp"

namespace mge
{
    class Input
    {
    public:
        Input();
        Input(void *window);
        ~Input();

        static void Initialize();
        static void Initialize(void *window);
        static void Update();
        static void Finalize();

        static bool KeyDown(KeyboardKey key);
        static bool KeyPressed(KeyboardKey key);
        static bool KeyReleased(KeyboardKey key);

        static bool MouseDown(MouseButton button);
        static bool MousePressed(MouseButton button);
        static bool MouseReleased(MouseButton button);

        static float MouseDeltaX();
        static float MouseDeltaY();
        static glm::vec2 MouseDelta();

        static float MousePositionX();
        static float MousePositionY();
        static glm::vec2 MousePosition();

        static void ShowCursor();
        static void HideCursor();
        static void DisableCursor();
    };
} // namespace mge

#endif