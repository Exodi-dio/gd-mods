#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <imgui-cocos.hpp>
#include "../../core/config.hpp"
#include "../../core/gui.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"

GUI_HACK_CREATE("Cosmetic", "Pulse Size", "Changes pulsation of falls, orbs, etc", false);

class $modify(PulseSizePlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Pulse Size");        

        hack.addHookPtr(self.getHook("PlayLayer::updateVisibility").unwrap());

        hack.setCustomWindowImGui([
            multiplyKey = hack.formatAdditionalSetting("multiply"),
            noPulseKey = hack.formatAdditionalSetting("noPulse"),
            valueKey = hack.formatAdditionalSetting("value")
        ]{
            ImGuiWidgetConfig::Checkbox("No Pulse", noPulseKey, false);
            ImGuiWidgetConfig::Checkbox("Multiply pulsation", multiplyKey, true);
            ImGuiWidgetConfig::DragFloat("##PulseSizeValue", valueKey, 0.01f, 0, FLT_MAX, 0.5, "Pulse Size: %.2f");
        });

        hack.setCustomWindowCocos([
            multiplyKey = hack.formatAdditionalSetting("multiply"),
            noPulseKey = hack.formatAdditionalSetting("noPulse"),
            valueKey = hack.formatAdditionalSetting("value")
        ](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            popup->addConfigToggle("No Pulse", noPulseKey, false);
            popup->addConfigToggle("Multiply pulsation", multiplyKey, true);
            popup->addConfigFloatInput("Pulse Size", valueKey, 0.01f, FLT_MAX, 0.5f);
        });
        
    }

    void updateVisibility(float dt) {   
        auto& config = Config::get();

        bool noPulse = config.get<bool>("cosmetic.pulse_size::noPulse", false);        

        if (!noPulse) {
            bool multiply = config.get<bool>("cosmetic.pulse_size::multiply", true);
            float value = config.get<float>("cosmetic.pulse_size::value", 0.5f);

            auto fmod = FMODAudioEngine::get();
            if (multiply) {
                m_audioEffectsLayer->m_audioScale *= value;
                fmod->m_pulse1 *= value;
            } else {
                m_audioEffectsLayer->m_audioScale = value;
                fmod->m_pulse1 = value;
            }
        }
        else {
            auto fmod = FMODAudioEngine::get();
            m_audioEffectsLayer->m_audioScale = config.get<float>("cosmetic.pulse_size::value", 0.5f);
            fmod->m_pulse1 = config.get<float>("cosmetic.pulse_size::value", 0.5f);
        }

        PlayLayer::updateVisibility(dt);
    }
};