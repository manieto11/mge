#include "mge/mge.hpp"

#include <iostream>

int main()
{
    try
    {
        mge::Window::Create("test-window", 800, 600);
        mge::Renderer::Initialize();
        mge::Input::Initialize();

        mge::Renderer::SetBackgroundColor(1, 1, 1);

        while (!mge::Window::ShouldClose())
        {
            mge::Input::Update();

            if (mge::Input::KeyPressed(mge::KeyboardKey::ESCAPE))
                mge::Window::Close();

            if (mge::Input::KeyPressed(mge::KeyboardKey::UP))
                mge::Window::SetHeight(mge::Window::GetHeight() - 10);
            if (mge::Input::KeyPressed(mge::KeyboardKey::DOWN))
                mge::Window::SetHeight(mge::Window::GetHeight() + 10);
            if (mge::Input::KeyPressed(mge::KeyboardKey::LEFT))
                mge::Window::SetWidth(mge::Window::GetWidth() - 10);
            if (mge::Input::KeyPressed(mge::KeyboardKey::RIGHT))
                mge::Window::SetWidth(mge::Window::GetWidth() + 10);

            glm::ivec2 position = mge::Window::GetPosition();

            if (mge::Input::KeyPressed(mge::KeyboardKey::W))
                position.y -= 10;
            if (mge::Input::KeyPressed(mge::KeyboardKey::S))
                position.y += 10;
            if (mge::Input::KeyPressed(mge::KeyboardKey::A))
                position.x -= 10;
            if (mge::Input::KeyPressed(mge::KeyboardKey::D))
                position.x += 10;

            mge::Window::SetPosition(position);

            mge::Renderer::BeginFrame();

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
