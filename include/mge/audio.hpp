#if !defined(MGE_AUDIO_HPP) && !defined(MGE_DISABLE_AUDIO)
#define MGE_AUDIO_HPP

#include <string>

namespace mge
{
    struct Sound
    {
        unsigned int id;
    };    

    class Audio
    {
    public:
        static void Initialize();
        static void Finalize();

        static Sound LoadSound(const std::string path);
        static void UnloadSound(Sound sound);

        static void PlaySound(Sound sound);
        static void PauseSound(Sound sound);
        static void ResumeSound(Sound sound);
    };
} // namespace mge

#endif