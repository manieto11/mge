#include "mge/mge.hpp"

#include <iostream>

int main()
{
    try
    {
        mge::Audio::Initialize();

        mge::Audio::Finalize();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}