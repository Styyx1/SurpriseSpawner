#pragma once

namespace Events
{
    class LootActivateEvent : public RE::BSTEventSink<RE::TESActivateEvent>
    {
        LootActivateEvent()                                    = default;
        LootActivateEvent(const LootActivateEvent&)            = delete;
        LootActivateEvent(LootActivateEvent&&)                 = delete;
        LootActivateEvent& operator=(const LootActivateEvent&) = delete;
        LootActivateEvent& operator=(LootActivateEvent&&)      = delete;

    public:
        static LootActivateEvent* GetSingleton()
        {
            static LootActivateEvent singleton;
            return &singleton;
        }

        RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* a_event, RE::BSTEventSource<RE::TESActivateEvent>*);

        void RegisterEvents()
        {
            logger::info("{:*^30}", "EVENTS");

            if (const auto scripts = RE::ScriptEventSourceHolder::GetSingleton()) {
                scripts->AddEventSink<RE::TESActivateEvent>(this);
                logger::info("Registered {}"sv, typeid(RE::TESActivateEvent).name());
            }
        }

        bool wasActivated{ false };
    };
} // namespace Events
