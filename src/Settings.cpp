#include "Settings.h"
#include "Utility.h"

void Settings::LoadSettings() noexcept
{
    logger::info("Loading settings");

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\SurpriseSpawner.ini)");

    std::string spawnEnemyID(ini.GetValue("General", "SpawnFormID", ""));
    logger::debug("got enemy from the ini");
    std::string spawnExplosionID(ini.GetValue("General", "ExplosionFormID", ""));
    logger::debug("got explosion from the ini");
    maxNumber = std::stoi(ini.GetValue("General", "iMaxNumberRand", ""));
    logger::debug("got max int from the ini");
    minNumber = std::stoi(ini.GetValue("General", "iMinNumberRand", ""));
    logger::debug("got min int from the ini");
    compareValue = std::stoi(ini.GetValue("General", "iCompareValue", ""));
    logger::debug("got compare int from the ini");
    std::string fileName(ini.GetValue("General", "sModFileName", ""));
    logger::debug("got Modname from the ini");
    debug_logging = ini.GetBoolValue("Log", "Debug");

    logger::debug("got all settings from the ini");

    if (!spawnEnemyID.empty()) {
        SpawnFormID = ParseFormID(spawnEnemyID);
    };

    if (!spawnExplosionID.empty()) {
        SpawnExplosionFormID = ParseFormID(spawnExplosionID);
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
    auto settings    = Settings::GetSingleton();

    logger::info("Loading forms");
    if (SpawnFormID)
        SpawnEnemy = skyrim_cast<RE::TESNPC*>(dataHandler->LookupForm(SpawnFormID, FileName));

    if (SpawnExplosionFormID)
        SpawnExplosion = skyrim_cast<RE::BGSExplosion*>(dataHandler->LookupForm(SpawnExplosionFormID, FileName));

    // Hardcoded loads
    /*settings->SpawnExplosion   = dataHandler->LookupForm(RE::FormID(ParseFormID("0x908")), "SurpriseSpawn.esp")->As<RE::BGSExplosion>();
    settings->SpawnEnemy       =
     * dataHandler->LookupForm(RE::FormID(ParseFormID("0x800")), "SurpriseSpawn.esp")->As<RE::TESNPC>();*/

    logger::debug("All Forms loaded");
    logger::debug("Explosion is {}", std::to_string(settings->SpawnExplosion->GetFormID()));
    logger::debug("Enemy is {}", settings->SpawnEnemy->GetName());
    logger::debug("max number for randomiser is {}", std::to_string(settings->maxNumber));
    logger::debug("min number for randomiser is {}", std::to_string(settings->minNumber));
    logger::debug("compare number for randomiser is {}", std::to_string(settings->compareValue));

} // Credits: Colinswrath https://github.com/colinswrath/BladeAndBlunt
