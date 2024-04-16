#include "Settings.h"
#include "Utility.h"

void Settings::LoadSettings() noexcept
{
    logger::info("Loading settings");

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\SurpriseSpawner.ini)");

    maxNumber                     = std::stoi(ini.GetValue("General", "iMaxNumberRand", ""));
    minNumber                     = std::stoi(ini.GetValue("General", "iMinNumberRand", ""));
    compareValue                  = std::stoi(ini.GetValue("General", "iCompareValue", ""));
    npc_event_active              = ini.GetBoolValue("Event Toggles", "bNPCEvent", "");
    npc_event_active              = ini.GetBoolValue("Event Toggles", "bNPCEvent", "");
    draugr_container_event_active = ini.GetBoolValue("Event Toggles", "bDraugrContainerEvent", "");

    std::string fileName(ini.GetValue("General", "sModFileName", ""));
    std::string spawnEnemyID(ini.GetValue("General", "SpawnFormID", ""));
    std::string spawnExplosionID(ini.GetValue("General", "ExplosionFormID", ""));
    std::string DraugrChestSpawnID(ini.GetValue("General", "DraugrChestEnemy", ""));

    debug_logging = ini.GetBoolValue("Log", "Debug");

    if (!spawnEnemyID.empty()) {
        SpawnFormID = ParseFormID(spawnEnemyID);
    };

    if (!spawnExplosionID.empty()) {
        SpawnExplosionFormID = ParseFormID(spawnExplosionID);
    }

    if (!DraugrChestSpawnID.empty()) {
        DraugrEnemyFormID = ParseFormID(DraugrChestSpawnID);
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

void Settings::LoadForms()
{
    auto dataHandler = RE::TESDataHandler::GetSingleton();

    logger::info("Loading forms");
    if (SpawnFormID)
        SpawnEnemy = skyrim_cast<RE::TESNPC*>(dataHandler->LookupForm(SpawnFormID, FileName));

    if (SpawnExplosionFormID)
        SpawnExplosion = skyrim_cast<RE::BGSExplosion*>(dataHandler->LookupForm(SpawnExplosionFormID, FileName));

    if (DraugrEnemyFormID)
        DraugrEnemy = skyrim_cast<RE::TESNPC*>(dataHandler->LookupForm(DraugrEnemyFormID, FileName));

    logger::info("All Forms loaded");

} // Credits: Colinswrath https://github.com/colinswrath/BladeAndBlunt
