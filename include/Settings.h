#pragma once

class Settings : public Singleton<Settings>
{
public:
    void LoadSettings() noexcept;
    void LoadForms() noexcept;

    std::string   FileName;
    std::uint32_t minNumber;
    std::uint32_t maxNumber;
    std::uint32_t compareValue;

    RE::FormID SpawnFormID;
    RE::FormID SpawnExplosionFormID;
    RE::FormID DraugrEnemyFormID;
    RE::FormID DwarvenSpawnFormID;
    RE::FormID ShadeSpawnFormID;
    RE::FormID MemeSoundFormID;

    RE::BGSExplosion* SpawnExplosion;
    RE::TESNPC*       SpawnEnemy; // had to be TESNPC* cause Actor* doesn't work
    RE::TESNPC*       DraugrEnemy;
    RE::TESNPC*       DwarvenEnemy;
    RE::TESNPC*       ShadeEnemy;

    RE::BGSSoundDescriptorForm* MemeSound;

    static RE::FormID ParseFormID(const std::string& str);

    inline static bool debug_logging{};

    inline static bool npc_event_active{ true };
    inline static bool draugr_container_event_active{ true };
    inline static bool dwarven_container_event_active{ true };
    inline static bool shade_container_event_active{ true };
    inline static bool toggle_meme_sound{ false };
};
