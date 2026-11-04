#ifdef GEODE_IS_WINDOWS
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../core/keybinds.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include <imgui-cocos.hpp>

GUI_HACK_CREATE("Level", "Auto Deafen", "Deafens user in Discord after a certain %", false);

static bool m_autoDeafenMuted = false;

void setDeafen(bool muted) {
    if (m_autoDeafenMuted == muted) return;
    m_autoDeafenMuted = muted;

    auto &kb = GDH::Keybinds::get();
    
    auto bind = kb.getBind("level.auto_deafen::bind");
    if (bind.key != cocos2d::enumKeyCodes::KEY_None) {
        keybd_event(kb.convertToWinAPI(bind.key), 0, 0, 0);
        keybd_event(kb.convertToWinAPI(bind.key), 0, KEYEVENTF_KEYUP, 0);
    }
}

class $modify(AutoDeafenPlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Auto Deafen");    
        
        hack.addHookPtr(self.getHook("PlayLayer::updateProgressbar").unwrap());
        hack.addHookPtr(self.getHook("PlayLayer::pauseGame").unwrap());

        auto& kb = GDH::Keybinds::get();
        auto& config = Config::get();

        hack.setCustomWindowImGui([
            bindKey = hack.formatAdditionalSetting("bind"),
            startKey = hack.formatAdditionalSetting("start"),
            endKey = hack.formatAdditionalSetting("end"),
            inPracticeKey = hack.formatAdditionalSetting("defaen_in_practice"),
            withStartPosKey = hack.formatAdditionalSetting("defaen_with_startpos"),
            undeafenOnPauseKey = hack.formatAdditionalSetting("undeafen_on_pause"),
            
            &kb, &config
        ]() {
            ImGuiWidgetConfig::DrawCustomKeybindButton(bindKey, "Bind key");

            static bool warningText = false;
            if (kb.getBind(bindKey).modifiers != KeyboardModifier::None) {
                kb.changeBind(bindKey, geode::Keybind());
                warningText = true;
            }

            if (warningText) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 128, 128).Value);
                ImGui::Text("You can't bind keys with modifiers. Use only a single key");
                ImGui::PopStyleColor();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGuiWidgetConfig::DragInt("##DeafenStart", startKey, 1.f, 0, 100, 50, "Deafen At: %d%%");
            ImGuiWidgetConfig::DragInt("##DeafenEnd", endKey, 1.f, 0, 100, 100, "Undeafen At: %d%%");

            ImGuiWidgetConfig::Checkbox("Deafen in Practice", inPracticeKey, false);
            ImGuiWidgetConfig::Checkbox("Deafen with StartPos", withStartPosKey, false);
            ImGuiWidgetConfig::Checkbox("Undeafen on Pause", undeafenOnPauseKey, true);
        });
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();

        auto& config = Config::get();
        bool defaen_in_practice = config.get<bool>("level.auto_deafen::defaen_in_practice", false);
        bool defaen_with_startpos = config.get<bool>("level.auto_deafen::defaen_with_startpos", false);

        if (!defaen_in_practice && m_isPracticeMode) return setDeafen(false);
        if (!defaen_with_startpos && m_isTestMode) return setDeafen(false);

        if (m_isPaused || m_levelEndAnimationStarted || m_playerDied)
            return setDeafen(false);

        int percentage = getCurrentPercentInt();
        int start = config.get<int>("level.auto_deafen::start", 50);
        int end = config.get<int>("level.auto_deafen::end", 100);
        setDeafen(percentage >= start && percentage <= end);
    }

    void pauseGame(bool unfocused) {
        PlayLayer::pauseGame(unfocused);

        auto& config = Config::get();
        if (config.get<bool>("level.auto_deafen::undeafen_on_pause", true))
            setDeafen(false);
    }
};
#endif