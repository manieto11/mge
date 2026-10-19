#if !defined(MGE_MOUSE_BUTTON_HPP)
#define MGE_MOUSE_BUTTON_HPP

namespace mge
{
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
} // namespace mge


#endif