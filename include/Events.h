#pragma once
#include "Settings.h"

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

        void RegisterActivateEvents()
        {
            logger::info("{:*^30}", "EVENTS");

            if (const auto scripts = RE::ScriptEventSourceHolder::GetSingleton()) {
                scripts->AddEventSink<RE::TESActivateEvent>(this);
                logger::info("Registered {}"sv, typeid(RE::TESActivateEvent).name());
            }
        }

        bool wasActivated{ false };

        bool isContainerEventsActive()
        {
            auto settings = Settings::GetSingleton();
            if (settings->draugr_container_event_active || settings->dwarven_container_event_active || settings->generic_container_event_active
                || settings->urn_explosion_event_active || settings->shade_container_event_active)
            {
                return true;
            }
            else
                return false;
        }
    };

    class MenuEvent : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
        MenuEvent()                            = default;
        MenuEvent(const MenuEvent&)            = delete;
        MenuEvent(MenuEvent&&)                 = delete;
        MenuEvent& operator=(const MenuEvent&) = delete;
        MenuEvent& operator=(MenuEvent&&)      = delete;

    public:
        static MenuEvent* GetSingleton()
        {
            static MenuEvent singleton;
            return &singleton;
        }

        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

        void RegisterMenuEvents()
        {
            if (const auto scripts = RE::UI::GetSingleton()) {
                scripts->AddEventSink<RE::MenuOpenCloseEvent>(this);
                logger::info("Registered {}"sv, typeid(RE::MenuOpenCloseEvent).name());
            }
        }

        inline void CloseMenu(RE::BSFixedString a_menuName)
        {
            if (const auto UIMsgQueue = RE::UIMessageQueue::GetSingleton(); UIMsgQueue) {
                UIMsgQueue->AddMessage(a_menuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
            }
        }
    };
} // namespace Events
