#if !defined(MGE_GAMEPAD_HPP)
#define MGE_GAMEPAD_HPP

namespace mge
{
    enum class GamepadButton
    {
        A = 0,
        B = 1,
        X = 2,
        Y = 3,
        LEFT_BUMPER = 4,
        RIGHT_BUMPER = 5,
        BACK = 6,
        START = 7,
        GUIDE = 8,
        LEFT_THUMB = 9,
        RIGHT_THUMB = 10,
        DPAD_UP = 11,
        DPAD_RIGHT = 12,
        DPAD_DOWN = 13,
        DPAD_LEFT = 14,

        LB = LEFT_BUMPER,
        RB = RIGHT_BUMPER,
        VIEW = BACK,
        MENU = START,
        LSB = LEFT_THUMB,
        RSB = RIGHT_THUMB,

        CROSS = A,
        CIRCLE = B,
        SQUARE = X,
        TRIANGLE = Y,
        L1 = LEFT_BUMPER,
        R1 = RIGHT_BUMPER,
        SHARE = BACK,
        OPTIONS = START,
        L3 = LEFT_THUMB,
        R3 = RIGHT_THUMB,
    };

    enum class GamepadAxis
    {
        LEFT_X = 0,
        LEFT_Y = 1,
        RIGHT_X = 2,
        RIGHT_Y = 3,
        LEFT_TRIGGER = 4,
        RIGHT_TRIGGER = 5,

        LT = LEFT_TRIGGER,
        RT = RIGHT_TRIGGER,

        L2 = LEFT_TRIGGER,
        R2 = RIGHT_TRIGGER,
    };

    class Gamepad
    {
    private:
        const int index;

    public:
        Gamepad(int index);

        void Initialize() const;
        void Update() const;

        bool IsAvailable() const;
        
        bool IsButtonDown(GamepadButton button) const;
        bool IsButtonPressed(GamepadButton button) const;
        bool IsButtonReleased(GamepadButton button) const;

        float GetAxis(GamepadAxis axis) const;

        int GetIndex() const;
    };
} // namespace mge

#endif