#include "Events.h"
#include "Settings.h"
#include "Utility.h"

namespace Events
{
    RE::BSEventNotifyControl LootActivateEvent::ProcessEvent(const RE::TESActivateEvent* eventPtr, RE::BSTEventSource<RE::TESActivateEvent>*)
    {
        if (!eventPtr)
            return RE::BSEventNotifyControl::kContinue;

        auto event    = eventPtr;
        auto dead_guy = event->objectActivated->As<RE::Actor>();
        auto settings = Settings::GetSingleton();
        auto util     = Utility::GetSingleton();

        const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
        const auto script        = scriptFactory ? scriptFactory->Create() : nullptr;

        // Only do stuff when looking at dead actors
        if (dead_guy && dead_guy->IsDead()) {
            auto chance = util->RandomInt(settings->minNumber, settings->maxNumber);
            logger::debug("random chance number is: {}, but compare value is {}", std::to_string(chance), std::to_string(settings->compareValue));
           
            //check for a random number, so
            //it doesn't happen too often and
            //it doesn't happen on the same npc twice in a row
            // (technically still possible but unlikely)
            //I still haven't figured out how to automatically close the menu
           
            if (chance == settings->compareValue) {
                logger::debug("you looked at {} and the random int was {}", event->objectActivated->GetDisplayFullName(), std::to_string(chance));
                dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                auto  dude  = dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnEnemy, false)->AsReference();
                float scale = 0.001f;
                script->SetCommand(fmt::format(FMT_STRING("setscale {}"), scale));
                script->CompileAndRun(dude);
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
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace Events
