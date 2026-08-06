#include "mge/mge.h"

#include <iostream>

int main()
{
    if (mge::Window::Create("Window", 800, 600))
    {
        std::cout << "Window created!" << '\n';
    }

    while(!mge::Window::ShouldClose())
    {
        mge::Window::BeginFrame();

        // Draw stuff

        mge::Window::EndFrame();
    }

    mge::Window::Close();
}