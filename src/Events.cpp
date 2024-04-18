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

        auto settings = Settings::GetSingleton();
        auto util     = Utility::GetSingleton();
        auto player   = RE::PlayerCharacter::GetSingleton();

        const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
        const auto script        = scriptFactory ? scriptFactory->Create() : nullptr;

        bool        isLocked   = event->objectActivated->IsLocked();
        std::string nameOfCont = event->objectActivated->GetName();

        // Only do stuff when looking at dead actors
        if (eventPtr) {
            if (event->actionRef->IsPlayerRef()) {
                if (settings->npc_event_active) {
                    RE::Actor* dead_guy = event->objectActivated->As<RE::Actor>();
                    if (dead_guy && dead_guy->IsDead()) {
                        if (dead_guy->IsInFaction(settings->WerewolfFaction)) {
                            auto chance = util->RandomInt(settings->minNumber, settings->maxNumber);
                            if (chance == settings->compareValue) {
                                wasActivated = true;
                                dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                                auto  dude  = dead_guy->AsReference()->PlaceObjectAtMe(settings->WerewolfEnemy, false)->AsReference();
                                float scale = 0.001f;
                                script->SetCommand(fmt::format(FMT_STRING("setscale {}"), scale));
                                script->CompileAndRun(dude);
                                dude->MoveTo(dead_guy);
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1.5s);
                                    SKSE::GetTaskInterface()->AddTask([=] {
                                        float size = 1.0f;
                                        script->SetCommand(fmt::format(FMT_STRING("setscale {}"), size));
                                        script->CompileAndRun(dude);
                                        util->PlayMeme(settings->MemeSound);
                                    });
                                }).detach();
                                dude->Enable(false);
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1s);
                                    SKSE::GetTaskInterface()->AddTask([=] { wasActivated = false; });
                                }).detach();
                            }
                        }
                        else {
                            auto chance = util->RandomInt(settings->minNumber, settings->maxNumber);

                            // check for a random number, so
                            // it doesn't happen too often and
                            // it doesn't happen on the same npc twice in a row
                            // technically still possible but unlikely

                            if (chance == settings->compareValue) {
                                wasActivated = true;
                                dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                                auto  dude  = dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnEnemy, false)->AsReference();
                                float scale = 0.001f;
                                script->SetCommand(fmt::format(FMT_STRING("setscale {}"), scale));
                                script->CompileAndRun(dude);
                                dude->MoveTo(dead_guy);
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1.5s);
                                    SKSE::GetTaskInterface()->AddTask([=] {
                                        float size = 1.0f;
                                        script->SetCommand(fmt::format(FMT_STRING("setscale {}"), size));
                                        script->CompileAndRun(dude);
                                        util->PlayMeme(settings->MemeSound);
                                    });
                                }).detach();
                                dude->Enable(false);
                                std::jthread([=] {
                                    std::this_thread::sleep_for(1s);
                                    SKSE::GetTaskInterface()->AddTask([=] { wasActivated = false; });
                                }).detach();
                            }
                        }
                    }
                }
                if (event->objectActivated->GetBaseObject()->GetFormType() == RE::FormType::Container && !isLocked) {
                    if (settings->draugr_container_event_active && nameOfCont.contains("raugr")) {
                        auto chance = util->RandomInt(settings->minNumber, settings->maxNumber);
                        if (chance == settings->compareValue) {
                            wasActivated = true;
                            event->objectActivated->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                            auto mimic = event->objectActivated->AsReference()->PlaceObjectAtMe(settings->DraugrEnemy, false)->AsReference();
                            event->objectActivated->AsReference()->Disable();
                            util->PlayMeme(settings->MemeSound);
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

                    else if (settings->dwarven_container_event_active && util->LocationCheck("LocTypeDwarvenAutomatons")) {
                        auto chance = util->RandomInt(settings->minNumber, settings->maxNumber);
                        if (chance == settings->compareValue) {
                            wasActivated = true;
                            event->objectActivated->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                            auto mimic = event->objectActivated->AsReference()->PlaceObjectAtMe(settings->DwarvenEnemy, false)->AsReference();
                            event->objectActivated->AsReference()->Disable();
                            util->PlayMeme(settings->MemeSound);
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
                        auto chance = util->RandomInt(settings->minNumber, settings->maxNumber);
                        if (chance == settings->compareValue) {
                            wasActivated = true;
                            event->objectActivated->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                            auto mimic = event->objectActivated->AsReference()->PlaceObjectAtMe(settings->ShadeEnemy, false)->AsReference();
                            event->objectActivated->AsReference()->Disable();
                            util->PlayMeme(settings->MemeSound);
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
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace Events
