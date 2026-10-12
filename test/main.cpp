#include "mge/mge.h"

#include <iostream>
#include <thread>

int main()
{
    try
    {
        mge::Window window("test", 800, 600);
        mge::Input input;

        window.SetBackgroundColor(0, 0, 0);

        while (!window.ShouldClose())
        {
            input.Update();

            if (input.KeyPressed(mge::KeyboardKey::ESCAPE))
                window.Close();

            if (input.MousePressed(mge::MouseButton::LEFT))
                input.DisableCursor();
            else if (input.MousePressed(mge::MouseButton::RIGHT))
                input.ShowCursor();

            window.BeginFrame();

            window.EndFrame();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}