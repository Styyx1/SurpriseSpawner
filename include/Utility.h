#pragma once

class Utility : public Singleton<Utility>
{
public:
    // Credits: powerof3 https://github.com/powerof3/PapyrusExtenderSSE/blob/0d5d48485b444e73b641b43a99db35e7c5dcef4a/include/Papyrus/Functions/Utility.h#L10
    inline uint32_t RandomInt(uint32_t a_min, uint32_t a_max)
    {
        return clib_util::RNG().Generate<std::uint32_t>(a_min, a_max);
    }

};

