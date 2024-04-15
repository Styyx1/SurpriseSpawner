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

        auto                 event     = eventPtr;
        RE::Actor*           dead_guy  = event->objectActivated->As<RE::Actor>();
        RE::ContainerObject* container = event->objectActivated->As<RE::ContainerObject>();

        auto settings = Settings::GetSingleton();
        auto util     = Utility::GetSingleton();

        const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
        const auto script        = scriptFactory ? scriptFactory->Create() : nullptr;

        // Only do stuff when looking at dead actors
        if (eventPtr) {
            logger::debug("form type of activated object is: {}", RE::FormTypeToString(event->objectActivated->GetBaseObject()->GetFormType()));

            if (settings->npc_event_active) {
                if (dead_guy && dead_guy->IsDead()) {
                    auto chance = util->RandomInt(settings->minNumber, settings->maxNumber);
                    logger::debug("random chance number is: {}, but compare value is {}", std::to_string(chance), std::to_string(settings->compareValue));

                    // check for a random number, so
                    // it doesn't happen too often and
                    // it doesn't happen on the same npc twice in a row
                    // technically still possible but unlikely

                    if (chance == settings->compareValue) {
                        logger::debug("you looked at {} and the random int was {}", event->objectActivated->GetDisplayFullName(), std::to_string(chance));
                        wasActivated = true;
                        dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                        auto  dude  = dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnEnemy, false)->AsReference();
                        float scale = 0.001f;
                        script->SetCommand(fmt::format(FMT_STRING("setscale {}"), scale));
                        script->CompileAndRun(dude);
                        dude->MoveTo(dead_guy);
                        logger::debug("shrinked dude");
                        std::jthread([=] {
                            std::this_thread::sleep_for(1.5s);
                            SKSE::GetTaskInterface()->AddTask([=] {
                                float size = 1.0f;
                                script->SetCommand(fmt::format(FMT_STRING("setscale {}"), size));
                                script->CompileAndRun(dude);
                                logger::debug("made dude big");
                            });
                        }).detach();
                        dude->Enable(false);
                        std::jthread([=] {
                            std::this_thread::sleep_for(1s);
                            SKSE::GetTaskInterface()->AddTask([=] {
                                wasActivated = false;
                                logger::debug("set activated to false");
                            });
                        }).detach();
                    }
                }
            }
            if (settings->draugr_container_event_active) {
                if (event->objectActivated->GetBaseObject()->GetFormType() == RE::FormType::Container) {
                    std::string nameOfCont = event->objectActivated->GetName();
                    if (nameOfCont.contains("raugr")) {
                        auto chance = util->RandomInt(settings->minNumber, settings->maxNumber);
                        logger::debug("looked into {} ", event->objectActivated->GetName());
                        logger::debug("random chance number is: {}, but compare value is {}", std::to_string(chance), std::to_string(settings->compareValue));
                        if (chance == settings->compareValue) {
                            wasActivated = true;
                            event->objectActivated->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                            auto  mimic = event->objectActivated->AsReference()->PlaceObjectAtMe(settings->SpawnEnemy, false)->AsReference();
                            float scale = 0.001f;
                            script->SetCommand(fmt::format(FMT_STRING("setscale {}"), scale));
                            script->CompileAndRun(mimic);
                            script->SetCommand(fmt::format(FMT_STRING("setscale {}"), scale));
                            script->CompileAndRun(event->objectActivated->AsReference());
                            mimic->MoveTo(event->objectActivated->AsReference());
                            event->objectActivated->AsReference()->Disable();
                            logger::debug("shrinked dude");
                            std::jthread([=] {
                                std::this_thread::sleep_for(1.5s);
                                SKSE::GetTaskInterface()->AddTask([=] {
                                    float size = 1.0f;
                                    script->SetCommand(fmt::format(FMT_STRING("setscale {}"), size));
                                    script->CompileAndRun(mimic);
                                    logger::debug("made dude big");
                                });
                            }).detach();
                            mimic->Enable(false);
                            std::jthread([=] {
                                std::this_thread::sleep_for(1s);
                                SKSE::GetTaskInterface()->AddTask([=] {
                                    wasActivated = false;
                                    logger::debug("set activated to false");
                                });
                            }).detach();
                        }
                    }
                }
            }            
        }
        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace Events
