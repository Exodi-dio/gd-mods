#include <Geode/Geode.hpp>
#include <Geode/modify/HardStreak.hpp>
#include <imgui-cocos.hpp>
#include "../../core/config.hpp"
#include "../../core/gui.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"

GUI_HACK_CREATE("Cosmetic", "Wave Pulse Size", "Resizes the wave trail", false);

class $modify(WavePulseSizeHardStreak, HardStreak) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Wave Pulse Size"); 

        hack.addHookPtr(self.getHook("HardStreak::updateStroke").unwrap());

        hack.setCustomWindowImGui([
            multiplyKey = hack.formatAdditionalSetting("multiply"),
            noPulseKey = hack.formatAdditionalSetting("noPulse"),
            valueKey = hack.formatAdditionalSetting("value")
        ]() {
            ImGuiWidgetConfig::Checkbox("No Pulse##Wave", noPulseKey, false);
            ImGuiWidgetConfig::Checkbox("Multiply pulsation##Wave", multiplyKey, true);
            ImGuiWidgetConfig::DragFloat("##WavePulseSizeValue", valueKey, 0.01f, 0, FLT_MAX, 0.5, "Wave Pulse Size: %.2f");
        });

        hack.setCustomWindowCocos([
            multiplyKey = hack.formatAdditionalSetting("multiply"),
            noPulseKey = hack.formatAdditionalSetting("noPulse"),
            valueKey = hack.formatAdditionalSetting("value")
        ](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            popup->addConfigToggle("No Pulse", noPulseKey, false);
            popup->addConfigToggle("Multiply pulsation", multiplyKey, true);
            popup->addConfigFloatInput("Wave Pulse Size", valueKey, 0.01f, FLT_MAX, 0.5f);
        });
    }

    void updateStroke(float p0) {
        auto& config = Config::get();

        bool noPulse = config.get<bool>("cosmetic.wave_pulse_size::noPulse", false);

        if (!noPulse) {
            bool multiply = config.get<bool>("cosmetic.wave_pulse_size::multiply", true);
            float value = config.get<float>("cosmetic.wave_pulse_size::value", 0.5f);

            if (multiply)
                m_pulseSize *= value;
            else 
                m_pulseSize = value;
        }
        else {
            m_pulseSize = config.get<float>("cosmetic.wave_pulse_size::value", 0.5f);
        }

        HardStreak::updateStroke(p0);
    }
};