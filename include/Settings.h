#pragma once

class Settings : public Singleton<Settings>
{
public:
    void LoadSettings() noexcept;
    void        LoadForms();

    std::string      FileName;
    std::uint32_t    minNumber;
    std::uint32_t    maxNumber;
    std::uint32_t    compareValue;


    RE::FormID SpawnFormID;
    RE::FormID SpawnExplosionFormID;

    RE::BGSExplosion* SpawnExplosion;
    RE::TESNPC*       SpawnEnemy; // had to be TESNPC* cause Actor* doesn't work

    static RE::FormID ParseFormID(const std::string& str);

    inline static bool debug_logging{};
};
