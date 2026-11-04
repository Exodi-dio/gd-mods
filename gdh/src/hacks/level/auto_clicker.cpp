#include <Geode/Geode.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include <imgui-cocos.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../interface/cocos/hack_settings_popup.hpp"

GUI_HACK_CREATE("Level", "Autoclicker", "", true);

class $modify(AutoclickerGJBaseGameLayer, GJBaseGameLayer) {
    struct Fields {
        int m_length = 0;
        bool m_pushing = false;
    };

    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Autoclicker");        
        
        hack.addHookPtr(self.getHook("GJBaseGameLayer::processQueuedButtons").unwrap());
        hack.setCustomWindowImGui([] {
            ImGuiWidgetConfig::DragInt("##autoclicker::push_length", "level.autoclicker::push_length", 1.f, 1, INT_MAX, 4, "Push Length: %d");
            ImGuiWidgetConfig::DragInt("##autoclicker::release_length", "level.autoclicker::release_length", 1.f, 1, INT_MAX, 4, "Release Length: %d");
            ImGuiWidgetConfig::Checkbox("First Player", "level.autoclicker::p1", true);
            ImGuiWidgetConfig::Checkbox("Second Player", "level.autoclicker::p2", false);
        });

        hack.setCustomWindowCocos([](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            popup->addConfigIntInput("Push Length", "level.autoclicker::push_length", 1, INT_MAX, 4);
            popup->addConfigIntInput("Release Length", "level.autoclicker::release_length", 1, INT_MAX, 4);
            popup->addConfigToggle("First Player", "level.autoclicker::p1", true);
            popup->addConfigToggle("Second Player", "level.autoclicker::p2", false);
        });
    }

    void processQueuedButtons(float dt, bool clearInputQueue) {
        GJBaseGameLayer::processQueuedButtons(dt, clearInputQueue);
        if (m_levelEndAnimationStarted) return;
        
        auto& config = Config::get();
        int push_length = config.get<int>("level.autoclicker::push_length", 4);
        int release_length = config.get<int>("level.autoclicker::release_length", 4);
        auto fields = m_fields.self();

        fields->m_length++;
        
        if ((fields->m_length >= push_length && !fields->m_pushing) || (fields->m_length >= release_length && fields->m_pushing)) {
            fields->m_pushing = !fields->m_pushing;
            fields->m_length = 0;

            if (config.get<bool>("level.autoclicker::p1", true)) this->handleButton(fields->m_pushing, 1, true);
            if (config.get<bool>("level.autoclicker::p2", false)) this->handleButton(fields->m_pushing, 1, false);
        }
    }
};