#include "mge/mge.hpp"

#include <iostream>
#include <thread>

int main()
{
    try
    {
        mge::Window::Create("test", 800, 600);
        mge::Input::Initialize();

        while(!mge::Window::ShouldClose())
        {
            mge::Input::Update();

            if (mge::Input::KeyPressed(mge::KeyboardKey::ESCAPE))
                mge::Window::Close();

            mge::Window::BeginFrame();

            mge::Window::EndFrame();
        }
        
        mge::Window::Destroy();
        mge::Input::Finalize();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}