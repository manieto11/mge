#include "mge/mge.hpp"

#include <iostream>

int main()
{
    try
    {
        mge::Window::Create("test-renderer", 800, 600);
        mge::Renderer::Initialize();
        mge::Input::Initialize();

        mge::Renderer::SetBackgroundColor(1, 1, 1);

        while (!mge::Window::ShouldClose())
        {
            mge::Input::Update();

            if (mge::Input::KeyPressed(mge::KeyboardKey::ESCAPE))
                mge::Window::Close();

            mge::Renderer::BeginFrame();

            mge::Renderer::Draw3D();

            mge::Renderer::Draw2D();

            mge::Renderer::EndFrame();
        }

        mge::Renderer::Finalize();
        mge::Window::Destroy();
        mge::Input::Finalize();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
