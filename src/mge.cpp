#include "mge/mge.hpp"

void mge::InitializeEngine()
{
    Window::Create("mge window", 800, 600);
    Renderer::Initialize();

    Audio::Initialize();
    Input::Initialize();
    Physics::Initialize();
}

void mge::FinalizeEngine()
{
    Audio::Finalize();
    Input::Finalize();
    Physics::Finalize();

    Renderer::Finalize();
    Window::Destroy();
}