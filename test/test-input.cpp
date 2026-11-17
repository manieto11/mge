#include "mge/mge.hpp"

#include <iostream>

int main()
{

    mge::Window::Create("test window", 800, 600);
    mge::Input::Initialize();

    mge::Window::SetBackgroundColor(1, 1, 1);

    while (!mge::Window::ShouldClose())
    {
        mge::Input::Update();

        if (mge::Input::KeyPressed(mge::KeyboardKey::ESCAPE))
            mge::Window::Close();
        if (mge::Input::KeyPressed(mge::KeyboardKey::SPACE))
            std::cout << "Key space pressed.\n";
        if (mge::Input::MousePressed(mge::MouseButton::LEFT))
            std::cout << "Mouse button left pressed.\n";

        for (int i = 0; i < 16; ++i)
        {
            if (!mge::Input::GetGamepad(i).IsAvailable())
                continue;

            if (mge::Input::GetGamepad(i).IsButtonPressed(mge::GamepadButton::A))
                std::cout << "Gamepad " << i << " button A pressed.\n";
            if (mge::Input::GetGamepad(i).IsButtonReleased(mge::GamepadButton::B))
                std::cout << "Gamepad " << i << " button B released.\n";
            if (mge::Input::GetGamepad(i).IsButtonPressed(mge::GamepadButton::X))
                std::cout << "Gamepad " << i << " button X pressed.\n";
            if (mge::Input::GetGamepad(i).IsButtonPressed(mge::GamepadButton::Y))
                std::cout << "Gamepad " << i << " button Y pressed.\n";

            if (mge::Input::GetGamepad(i).GetAxis(mge::GamepadAxis::RT) > 0.5f)
                std::cout << "Gamepad " << i << " RT > 0.5f.\n";
        }

        mge::Window::BeginFrame();
        mge::Window::EndFrame();
    }

    mge::Window::Destroy();
    mge::Input::Finalize();
}