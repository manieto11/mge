#ifndef MGE_INPUT_H
#define MGE_INPUT_H

#include "glm/vec2.hpp"

namespace mge
{
    enum class KeyboardKey;
    enum class MouseButton;

    class Input
    {
    public:
        Input();
        ~Input();

        static void Initialize();
        static void Update();
        static void Finalize();

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

        static bool KeyDown(KeyboardKey key);
        static bool KeyPressed(KeyboardKey key);
        static bool KeyReleased(KeyboardKey key);
    };

    enum class MouseButton
    {
        BUTTON_0,
        BUTTON_1,
        BUTTON_2,
        BUTTON_3,
        BUTTON_4,
        BUTTON_5,
        BUTTON_6,
        BUTTON_7,
        SIZE,
        LEFT = BUTTON_0,
        RIGHT = BUTTON_1,
        MIDDLE = BUTTON_2,
    };

    enum class KeyboardKey
    {
        SPACE = 32,
        APOSTROPHE = 39,
        COMMA = 44,
        MINUS,
        PERIOD,
        SLASH,
        N_0,
        N_1,
        N_2,
        N_3,
        N_4,
        N_5,
        N_6,
        N_7,
        N_8,
        N_9,
        SEMICOLON = 59,
        EQUAL = 61,
        A = 65,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        LEFT_BRACKET,
        BACKSLASH,
        RIGHT_BRACKET,
        GRAVE_ACCENT = 96,
        WORLD1 = 161,
        WORLD2,
        ESCAPE = 256,
        ENTER,
        TAB,
        BACKSPACE,
        INSERT,
        DELETE,
        RIGHT,
        LEFT,
        DOWN,
        UP,
        PAGE_UP,
        PAGE_DOWN,
        HOME,
        END,
        CAPS_LOCK = 280,
        SCROLL_LOCK,
        NUM_LOCK,
        PRINT_SCREEN,
        PAUSE,
        F1 = 290,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,
        KP_0 = 320,
        KP_1,
        KP_2,
        KP_3,
        KP_4,
        KP_5,
        KP_6,
        KP_7,
        KP_8,
        KP_9,
        KP_DECIMAL,
        KP_DIVIDE,
        KP_MULTIPLY,
        KP_SUBTRACT,
        KP_ADD,
        KP_ENTER,
        KP_EQUAL,
        LEFT_SHIFT = 340,
        LEFT_CONTROL,
        LEFT_ALT,
        LEFT_SUPER,
        RIGHT_SHIFT,
        RIGHT_CONTROL,
        RIGHT_ALT,
        RIGHT_SUPER,
        MENU,
        SIZE,
    };
} // namespace mge

#endif