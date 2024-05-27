#include "Events.h"
#include "Settings.h"
#include "Utility.h"

namespace Events
{
    RE::BSEventNotifyControl MenuEvent::ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
    {
        auto activateManager = Events::LootActivateEvent::GetSingleton();
        auto menu            = Events::MenuEvent::GetSingleton();
        auto lootMenu        = RE::ContainerMenu::MENU_NAME;

        if (!event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        bool active = activateManager->wasActivated;
        if (event->menuName == lootMenu) {
            if (active) {
                menu->CloseMenu(lootMenu);
                active = false;
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    };

    RE::BSEventNotifyControl LootActivateEvent::ProcessEvent(const RE::TESActivateEvent* eventPtr, RE::BSTEventSource<RE::TESActivateEvent>*)
    {
        if (!eventPtr)
            return RE::BSEventNotifyControl::kContinue;

        auto event = eventPtr;

        auto                 settings = Settings::GetSingleton();
        auto                 util     = Utility::GetSingleton();
        RE::PlayerCharacter* player   = Cache::GetPlayerSingleton();

        bool        isLocked   = event->objectActivated->IsLocked();
        std::string nameOfCont = event->objectActivated->GetName();
        bool        isOwned    = util->LocPlayerOwned();

        if (eventPtr) {
            if (event->actionRef->IsPlayerRef()) {
                if (settings->npc_event_active) {
                    RE::Actor* dead_guy = event->objectActivated->As<RE::Actor>();
                    // Only do stuff when looking at dead actors
                    if (dead_guy && dead_guy->IsDead() && !util->ExceptionName(nameOfCont)) {
                        if (dead_guy->IsInFaction(settings->WerewolfFaction)) {
                            auto chance = util->GetRandomChance(settings->minNumber, settings->maxNumber);
                            if (chance == settings->compareValue) {
                                wasActivated = true;

                                RE::TESObjectREFR* deadNPCref = dead_guy->AsReference();
                                DelayedNPCSpawn(deadNPCref, settings->WerewolfEnemy, settings->SpawnExplosion, util->GetTimer());
                            }
                        }
                        else {
                            auto chance = util->GetRandomChance(settings->minNumber, settings->maxNumber);
                            if (chance == settings->compareValue) {
                                wasActivated                  = true;
                                RE::TESObjectREFR* deadNPCref = dead_guy->AsReference();
                                DelayedNPCSpawn(deadNPCref, settings->SpawnEnemy, settings->SpawnExplosion, util->GetTimer());
                            }
                        }
                    }
                }
                if (isContainerEventsActive() && !isOwned && !util->ExceptionName(nameOfCont)) {
                    if (event->objectActivated->GetBaseObject()->GetFormType() == RE::FormType::Container && !isLocked) {
                        if (settings->draugr_container_event_active && nameOfCont.contains("raugr")) {
                            auto chance = util->GetRandomChance(settings->minNumber, settings->maxNumber);
                            if (chance == settings->compareValue) {
                                wasActivated = true;
                                auto obj_ref = event->objectActivated->AsReference();
                                DelayedContainerSpawn(obj_ref, settings->DraugrEnemy, settings->SpawnExplosion, util->GetTimer());
                            }
                        }
                        else if (settings->urn_explosion_event_active && nameOfCont.contains("Urn")) {
                            auto chance = util->GetRandomChance(settings->minNumber, settings->maxNumber);
                            if (chance == settings->compareValue) {
                                wasActivated = true;
                                event->objectActivated->AsReference()->PlaceObjectAtMe(settings->UrnExplosion, false);
                                util->ApplyStress(player);
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1s);
                                    SKSE::GetTaskInterface()->AddTask([=] {
                                        wasActivated = false;
                                        logger::debug("set activated to false");
                                    });
                                }).detach();
                            }
                        }
                        else if (settings->dwarven_container_event_active && util->LocationCheck("LocTypeDwarvenAutomatons")) {
                            auto chance = util->GetRandomChance(settings->minNumber, settings->maxNumber);
                            if (chance == settings->compareValue) {
                                auto obj_ref = event->objectActivated->AsReference();
                                wasActivated = true;
                                DelayedContainerSpawn(obj_ref, settings->DwarvenEnemy, settings->SpawnExplosion, util->GetTimer());
                            }
                        }
                        else if (settings->shade_container_event_active && (util->LocationCheck("LocTypeWarlockLair") || util->LocationCheck("LocTypeVampireLair"))) {
                            auto chance = util->GetRandomChance(settings->minNumber, settings->maxNumber);
                            if (chance == settings->compareValue) {
                                wasActivated = true;
                                auto obj_ref = event->objectActivated->AsReference();
                                DelayedContainerSpawn(obj_ref, settings->ShadeEnemy, settings->SpawnExplosion, util->GetTimer());
                            }
                        }
                        else if (settings->generic_container_event_active) {
                            auto chance = util->GetRandomChance(settings->minNumber, settings->maxNumber);
                            if (chance == settings->compareValue) {
                                wasActivated = true;
                                auto obj_ref = event->objectActivated->AsReference();
                                DelayedContainerSpawn(obj_ref, settings->MimicEnemy, settings->SpawnExplosion, util->GetTimer());
                            }
                        }
                        else
                            return RE::BSEventNotifyControl::kContinue;
                    }
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    void LootActivateEvent::DelayedContainerSpawn(RE::TESObjectREFR* a_eventItem, RE::TESNPC* a_enemyToSpawn, RE::TESBoundObject* a_explosion,
                                                  std::chrono::duration<double> a_threadDelay)
    {
        const Settings*      settings      = Settings::GetSingleton();
        bool                 explVis       = settings->toggle_visual_explosion;
        auto                 util          = Utility::GetSingleton();
        const auto           scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
        const auto           script        = scriptFactory ? scriptFactory->Create() : nullptr;
        RE::PlayerCharacter* player        = Cache::GetPlayerSingleton();
        if (!settings->delayed_explosion) {
            if (explVis) {
                a_eventItem->PlaceObjectAtMe(a_explosion, false);
            }
        }
        std::jthread([=] {
            std::this_thread::sleep_for(a_threadDelay);
            SKSE::GetTaskInterface()->AddTask([=] {
                if (settings->delayed_explosion) {
                    if (explVis) {
                        a_eventItem->PlaceObjectAtMe(a_explosion, false);
                    }
                }
                auto mimic = a_eventItem->PlaceObjectAtMe(a_enemyToSpawn, false)->AsReference();
                script->SetCommand(fmt::format(FMT_STRING("resetai")));
                script->CompileAndRun(mimic); // no idea why this is needed but it fixed my spawn being passive
                mimic->MoveTo(a_eventItem);
                util->RemoveAllItems(a_eventItem, mimic);
                util->PlayMeme(settings->MemeSound);
                util->ApplyStress(player);
                a_eventItem->Disable();
            });
        }).detach();
        std::jthread([=] {
            std::this_thread::sleep_for(a_threadDelay + 10s);
            SKSE::GetTaskInterface()->AddTask([=] { a_eventItem->Enable(false); });
        }).detach();
        std::jthread([=] {
            std::this_thread::sleep_for(1s);
            SKSE::GetTaskInterface()->AddTask([=] { wasActivated = false; });
        }).detach();
    }

    void LootActivateEvent::DelayedNPCSpawn(RE::TESObjectREFR* a_eventItem, RE::TESNPC* a_enemyToSpawn, RE::TESBoundObject* a_explosion,
                                            std::chrono::duration<double> a_threadDelay)
    {
        const Settings*      settings = Settings::GetSingleton();
        bool                 explVis  = settings->toggle_visual_explosion;
        Utility*             util     = Utility::GetSingleton();
        RE::PlayerCharacter* player   = Cache::GetPlayerSingleton();

        if (!settings->delayed_explosion) {
            if (explVis) {
                a_eventItem->PlaceObjectAtMe(a_explosion, false);
            }
        }
        std::jthread([=] {
            std::this_thread::sleep_for(a_threadDelay);
            SKSE::GetTaskInterface()->AddTask([=] {
                if (settings->delayed_explosion) {
                    if (explVis) {
                        a_eventItem->PlaceObjectAtMe(a_explosion, false);
                    }
                }
                auto dude = a_eventItem->PlaceObjectAtMe(a_enemyToSpawn, false)->AsReference();
                dude->MoveTo(a_eventItem);
                util->PlayMeme(settings->MemeSound);
                util->ApplyStress(player);
            });
        }).detach();
        std::jthread([=] {
            std::this_thread::sleep_for(1s);
            SKSE::GetTaskInterface()->AddTask([=] { wasActivated = false; });
        }).detach();
    }
} // namespace Events
