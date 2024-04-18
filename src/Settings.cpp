#include "Settings.h"
#include "Utility.h"

void Settings::LoadSettings() noexcept
{
    logger::info("Loading settings");

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\SurpriseSpawner.ini)");

    maxNumber                      = std::stoi(ini.GetValue("General", "iMaxNumberRand", ""));
    minNumber                      = std::stoi(ini.GetValue("General", "iMinNumberRand", ""));
    compareValue                   = std::stoi(ini.GetValue("General", "iCompareValue", ""));
    npc_event_active               = ini.GetBoolValue("Event Toggles", "bNPCEvent", "");
    npc_event_active               = ini.GetBoolValue("Event Toggles", "bNPCEvent", "");
    draugr_container_event_active  = ini.GetBoolValue("Event Toggles", "bDraugrContainerEvent");
    dwarven_container_event_active = ini.GetBoolValue("Event Toggles", "bDwarvenContainerEvent");
    shade_container_event_active   = ini.GetBoolValue("Event Toggles", "bShadeContainerEvent");

    std::string fileName(ini.GetValue("General", "sModFileName", ""));
    std::string spawnEnemyID(ini.GetValue("Enemies", "CorpseSpawnFormID", ""));
    std::string spawnExplosionID(ini.GetValue("General", "ExplosionFormID", ""));
    std::string DraugrChestSpawnID(ini.GetValue("Enemies", "DraugrChestEnemy", ""));
    std::string DwarvenChestSpawnID(ini.GetValue("Enemies", "DwarvenChestEnemy", ""));
    std::string ShadeChestSpawnID(ini.GetValue("Enemies", "ShadeChestEnemy", ""));
    std::string WerewolfEnemySpawnID(ini.GetValue("Enemies", "WerewolfSpawnEnemy", ""));
    std::string memeSoundID(ini.GetValue("Fun", "MemeSoundFormID", ""));

    toggle_meme_sound = ini.GetBoolValue("Fun", "bMemeSound");
    debug_logging     = ini.GetBoolValue("Log", "Debug");

    if (!spawnEnemyID.empty()) {
        SpawnFormID = ParseFormID(spawnEnemyID);
    };

    if (!spawnExplosionID.empty()) {
        SpawnExplosionFormID = ParseFormID(spawnExplosionID);
    }

    if (!DraugrChestSpawnID.empty()) {
        DraugrEnemyFormID = ParseFormID(DraugrChestSpawnID);
    }

    if (!DwarvenChestSpawnID.empty()) {
        DwarvenSpawnFormID = ParseFormID(DwarvenChestSpawnID);
    }

    if (!ShadeChestSpawnID.empty()) {
        ShadeSpawnFormID = ParseFormID(ShadeChestSpawnID);
    }
    if (!memeSoundID.empty()) {
        MemeSoundFormID = ParseFormID(memeSoundID);
    }
    if (!WerewolfEnemySpawnID.empty()) {
        WerewolfSpawnFormID = ParseFormID(WerewolfEnemySpawnID);
    }

    if (debug_logging) {
        spdlog::get("Global")->set_level(spdlog::level::level_enum::debug);
        logger::debug("Debug logging enabled");
    };
    // Load settings
    FileName = fileName;
    logger::info("Loaded settings");
};

RE::FormID Settings::ParseFormID(const std::string& str)
{
    RE::FormID         result;
    std::istringstream ss{ str };
    ss >> std::hex >> result;
    return result;
}

void Settings::LoadForms() noexcept
{
    auto dataHandler = RE::TESDataHandler::GetSingleton();

    logger::info("Loading forms");
    if (SpawnFormID)
        SpawnEnemy = skyrim_cast<RE::TESNPC*>(dataHandler->LookupForm(SpawnFormID, FileName));

    if (SpawnExplosionFormID)
        SpawnExplosion = skyrim_cast<RE::BGSExplosion*>(dataHandler->LookupForm(SpawnExplosionFormID, FileName));

    if (DraugrEnemyFormID)
        DraugrEnemy = skyrim_cast<RE::TESNPC*>(dataHandler->LookupForm(DraugrEnemyFormID, FileName));

    if (DwarvenSpawnFormID)
        DwarvenEnemy = skyrim_cast<RE::TESNPC*>(dataHandler->LookupForm(DwarvenSpawnFormID, FileName));

    if (ShadeSpawnFormID)
        ShadeEnemy = skyrim_cast<RE::TESNPC*>(dataHandler->LookupForm(ShadeSpawnFormID, FileName));

    if (MemeSoundFormID)
        MemeSound = skyrim_cast<RE::BGSSoundDescriptorForm*>(dataHandler->LookupForm(MemeSoundFormID, FileName));

    if (WerewolfSpawnFormID)
        WerewolfEnemy = skyrim_cast<RE::TESNPC*>(dataHandler->LookupForm(WerewolfSpawnFormID, FileName));

    // Hardcoded Loads

    WerewolfFaction = dataHandler->LookupForm(ParseFormID("0x43594"), "Skyrim.esm")->As<RE::TESFaction>();
    logger::debug("loaded Faction: {}", WerewolfFaction->GetName());

    logger::info("All Forms loaded");

} // Credits: Colinswrath https://github.com/colinswrath/BladeAndBlunt
