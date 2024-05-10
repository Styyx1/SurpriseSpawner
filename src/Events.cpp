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

        const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
        const auto script        = scriptFactory ? scriptFactory->Create() : nullptr;

        bool        isLocked   = event->objectActivated->IsLocked();
        std::string nameOfCont = event->objectActivated->GetName();
        bool        isOwned    = util->LocPlayerOwned();
        bool        explVis    = settings->toggle_visual_explosion;

        if (eventPtr) {
            if (event->actionRef->IsPlayerRef()) {
                if (settings->npc_event_active) {
                    RE::Actor* dead_guy = event->objectActivated->As<RE::Actor>();
                    // Only do stuff when looking at dead actors
                    if (dead_guy && dead_guy->IsDead()) {
                        if (dead_guy->IsInFaction(settings->WerewolfFaction)) {
                            auto chance = util->GetRandomChance(settings->minNumber, settings->maxNumber);
                            if (chance == settings->compareValue) {
                                wasActivated = true;
                                if (!settings->delayed_explosion) {
                                    if (explVis) {
                                        dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                                    }
                                }

                                std::jthread([=] {
                                    std::this_thread::sleep_for(settings->thread_delay);
                                    SKSE::GetTaskInterface()->AddTask([=] {
                                        if (settings->delayed_explosion) {
                                            if (explVis) {
                                                dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                                            }
                                        }
                                        auto dude = dead_guy->AsReference()->PlaceObjectAtMe(settings->WerewolfEnemy, false)->AsReference();
                                        dude->MoveTo(dead_guy);
                                        util->PlayMeme(settings->MemeSound);
                                        util->ApplyStress(player);
                                    });
                                }).detach();
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1s);
                                    SKSE::GetTaskInterface()->AddTask([=] { wasActivated = false; });
                                }).detach();
                            }
                        }
                        else {
                            auto chance = util->GetRandomChance(settings->minNumber, settings->maxNumber);

                            // check for a random number, so
                            // it doesn't happen too often and
                            // it doesn't happen on the same npc twice in a row
                            // technically still possible but unlikely

                            if (chance == settings->compareValue) {
                                wasActivated = true;
                                if (!settings->delayed_explosion) {
                                    if (explVis) {
                                        dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                                    }
                                }
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1.5s);
                                    SKSE::GetTaskInterface()->AddTask([=] {
                                        if (settings->delayed_explosion) {
                                            if (explVis) {
                                                dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                                            }
                                        }
                                        auto dude = dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnEnemy, false)->AsReference();
                                        dude->MoveTo(dead_guy);
                                        util->PlayMeme(settings->MemeSound);
                                        util->ApplyStress(player);
                                    });
                                }).detach();
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1s);
                                    SKSE::GetTaskInterface()->AddTask([=] { wasActivated = false; });
                                }).detach();
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
                                if (explVis) {
                                    event->objectActivated->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                                }
                                auto mimic = event->objectActivated->AsReference()->PlaceObjectAtMe(settings->DraugrEnemy, false)->AsReference();
                                event->objectActivated->AsReference()->Disable();
                                std::jthread([=] {
                                    std::this_thread::sleep_for(10s);
                                    SKSE::GetTaskInterface()->AddTask([=] {
                                        obj_ref->Enable(false);
                                        logger::debug("enabled container again");
                                    });
                                }).detach();
                                util->PlayMeme(settings->MemeSound);
                                util->ApplyStress(player);
                                script->SetCommand(fmt::format(FMT_STRING("resetai")));
                                script->CompileAndRun(mimic); // no idea why this is needed but it fixed my spawn being passive
                                util->RemoveAllItems(event->objectActivated->AsReference(), mimic);
                                mimic->PlayAnimation("GetUpStart", "GetUpEnd");
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1s);
                                    SKSE::GetTaskInterface()->AddTask([=] {
                                        wasActivated = false;
                                        logger::debug("set activated to false");
                                    });
                                }).detach();
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
                                if (explVis) {
                                    event->objectActivated->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                                }
                                auto mimic = event->objectActivated->AsReference()->PlaceObjectAtMe(settings->DwarvenEnemy, false)->AsReference();
                                event->objectActivated->AsReference()->Disable();
                                std::jthread([=] {
                                    std::this_thread::sleep_for(10s);
                                    SKSE::GetTaskInterface()->AddTask([=] {
                                        obj_ref->Enable(false);
                                        logger::debug("enabled container again");
                                    });
                                }).detach();
                                util->PlayMeme(settings->MemeSound);
                                util->ApplyStress(player);
                                script->SetCommand(fmt::format(FMT_STRING("resetai")));
                                script->CompileAndRun(mimic); // no idea why this is needed but it fixed my spawn being passive
                                mimic->MoveTo(event->objectActivated->AsReference());
                                util->RemoveAllItems(event->objectActivated->AsReference(), mimic);
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1s);
                                    SKSE::GetTaskInterface()->AddTask([=] { wasActivated = false; });
                                }).detach();
                            }
                        }
                        else if (settings->shade_container_event_active && (util->LocationCheck("LocTypeWarlockLair") || util->LocationCheck("LocTypeVampireLair"))) {
                            auto chance = util->GetRandomChance(settings->minNumber, settings->maxNumber);
                            if (chance == settings->compareValue) {
                                wasActivated = true;
                                auto obj_ref = event->objectActivated->AsReference();
                                if (explVis) {
                                    event->objectActivated->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                                }
                                auto mimic = event->objectActivated->AsReference()->PlaceObjectAtMe(settings->ShadeEnemy, false)->AsReference();
                                event->objectActivated->AsReference()->Disable();
                                std::jthread([=] {
                                    std::this_thread::sleep_for(10s);
                                    SKSE::GetTaskInterface()->AddTask([=] {
                                        obj_ref->Enable(false);
                                        logger::debug("enabled container again");
                                    });
                                }).detach();
                                util->PlayMeme(settings->MemeSound);
                                util->ApplyStress(player);
                                script->SetCommand(fmt::format(FMT_STRING("resetai")));
                                script->CompileAndRun(mimic); // no idea why this is needed but it fixed my spawn being passive
                                mimic->MoveTo(event->objectActivated->AsReference());
                                util->RemoveAllItems(event->objectActivated->AsReference(), mimic);
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1s);
                                    SKSE::GetTaskInterface()->AddTask([=] { wasActivated = false; });
                                }).detach();
                            }
                        }
                        else if (settings->generic_container_event_active) {
                            auto chance = util->GetRandomChance(settings->minNumber, settings->maxNumber);
                            if (chance == settings->compareValue) {
                                wasActivated = true;
                                auto obj_ref = event->objectActivated->AsReference();
                                if (explVis) {
                                    event->objectActivated->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                                }
                                auto mimic = event->objectActivated->AsReference()->PlaceObjectAtMe(settings->MimicEnemy, false)->AsReference();
                                event->objectActivated->AsReference()->Disable();
                                std::jthread([=] {
                                    std::this_thread::sleep_for(10s);
                                    SKSE::GetTaskInterface()->AddTask([=] {
                                        obj_ref->Enable(false);
                                        logger::debug("enabled container again");
                                    });
                                }).detach();
                                util->PlayMeme(settings->MemeSound);
                                util->ApplyStress(player);
                                script->SetCommand(fmt::format(FMT_STRING("resetai")));
                                script->CompileAndRun(mimic); // no idea why this is needed but it fixed my spawn being passive
                                mimic->MoveTo(event->objectActivated->AsReference());
                                util->RemoveAllItems(event->objectActivated->AsReference(), mimic);
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1s);
                                    SKSE::GetTaskInterface()->AddTask([=] { wasActivated = false; });
                                }).detach();
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
} // namespace Events
