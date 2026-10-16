#include "mge/mge.hpp"

#include <iostream>
#include <thread>

int main()
{
    try
    {
        mge::Window::Create("test", 800, 600);
        
        mge::Window::Destroy();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}