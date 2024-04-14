#include "Events.h"
#include "Settings.h"
#include "Utility.h"

namespace Events
{
    RE::BSEventNotifyControl LootActivateEvent::ProcessEvent(const RE::TESActivateEvent* eventPtr, RE::BSTEventSource<RE::TESActivateEvent>*)
    {
        if (!eventPtr)
            return RE::BSEventNotifyControl::kContinue;

        auto event = eventPtr;
        auto dead_guy = event->objectActivated->As<RE::Actor>();
        auto settings = Settings::GetSingleton();
        auto util     = Utility::GetSingleton();        

        // Only do stuff when looking at dead actors
        if (dead_guy && dead_guy->IsDead()) {
            auto chance = util->RandomInt(settings->minNumber, settings->maxNumber);
            logger::debug("random chance number is: {}", std::to_string(chance));
            /* check for a random number, so
            1) it doesn't happen too often and
            2) it doesn't happen on the same npc twice in a row (technically still possible but unlikely)*/
            if (chance == settings->compareValue) {
                logger::debug("you looked at {} and the random int was {}", event->objectActivated->GetDisplayFullName(), std::to_string(chance));
                dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnExplosion, false);
                auto dude = dead_guy->AsReference()->PlaceObjectAtMe(settings->SpawnEnemy, false)->AsReference();
                dude->Enable(false);
            }            
        }
        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace Events
