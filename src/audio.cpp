#include "mge/audio.hpp"

#include "mge/exceptions.hpp"
#include "miniaudio.h"

void mge::Audio::Initialize()
{
    throw NotImplementedException("mge::Audio::Initialize()");
}

void mge::Audio::Finalize()
{
    throw NotImplementedException("mge::Audio::Finalize()");
}

mge::Sound mge::Audio::LoadSound(const std::string path)
{
    throw NotImplementedException("mge::Audio::LoadSound()");
}

void mge::Audio::UnloadSound(Sound sound)
{
    throw NotImplementedException("mge::Audio::UnloadSound()");
}

void mge::Audio::PlaySound(Sound sound)
{
    throw NotImplementedException("mge::Audio::PlaySound()");
}

void mge::Audio::PauseSound(Sound sound)
{
    throw NotImplementedException("mge::Audio::PauseSound()");
}

void mge::Audio::ResumeSound(Sound sound)
{
    throw NotImplementedException("mge::Audio::ResumeSound()");
}
