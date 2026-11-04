#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <imgui-cocos.hpp>
#include "../../core/config.hpp"
#include "../../core/gui.hpp"
#include "../../core/utils.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"

GUI_HACK_CREATE("Cosmetic", "Rainbow Icon", "", false);

class $modify(RainbowIconGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Rainbow Icon");

        hack.addHookPtr(self.getHook("GJBaseGameLayer::update").unwrap());

        hack.setCustomWindowImGui([
            hue_speed  = hack.formatAdditionalSetting("hue_speed"),
            saturation = hack.formatAdditionalSetting("saturation"),
            value      = hack.formatAdditionalSetting("value"),
            color1     = hack.formatAdditionalSetting("color1"),
            color2     = hack.formatAdditionalSetting("color2"),
            glow       = hack.formatAdditionalSetting("glow"),
            trail      = hack.formatAdditionalSetting("trail"),
            wave       = hack.formatAdditionalSetting("wave")
        ]() {
            ImGuiWidgetConfig::DragFloat("##RainbowSpeedFactor", hue_speed, 0.01f, 0.01f, 5.f, 0.25f, "Speed: %.2f");
            ImGuiWidgetConfig::DragFloat("##RainbowSaturation", saturation, 0.01f, 0.0f, 1.f, 0.7f, "Saturation: %.2f");
            ImGuiWidgetConfig::DragFloat("##RainbowValue", value, 0.01f, 0.0f, 1.f, 1.0f, "Value: %.2f");
            
            ImGui::Text("Apply to");

            ImGuiWidgetConfig::Checkbox("Primary Color", color1, true);
            ImGuiWidgetConfig::Checkbox("Secondary Color", color2, true);
            ImGuiWidgetConfig::Checkbox("Glow Color", glow, true);
            ImGuiWidgetConfig::Checkbox("Regular Trail", trail, true);
            ImGuiWidgetConfig::Checkbox("Wave Trail", wave, true);
        });
        hack.setCustomWindowImGui([
            hue_speed  = hack.formatAdditionalSetting("hue_speed"),
            saturation = hack.formatAdditionalSetting("saturation"),
            value      = hack.formatAdditionalSetting("value"),
            color1     = hack.formatAdditionalSetting("color1"),
            color2     = hack.formatAdditionalSetting("color2"),
            glow       = hack.formatAdditionalSetting("glow"),
            trail      = hack.formatAdditionalSetting("trail"),
            wave       = hack.formatAdditionalSetting("wave")
        ]() {
            ImGuiWidgetConfig::DragFloat("##RainbowSpeedFactor", hue_speed, 0.01f, 0.01f, 5.f, 0.25f, "Speed: %.2f");
            ImGuiWidgetConfig::DragFloat("##RainbowSaturation", saturation, 0.01f, 0.0f, 1.f, 0.7f, "Saturation: %.2f");
            ImGuiWidgetConfig::DragFloat("##RainbowValue", value, 0.01f, 0.0f, 1.f, 1.0f, "Value: %.2f");
            
            ImGui::Text("Apply to");

            ImGuiWidgetConfig::Checkbox("Primary Color", color1, true);
            ImGuiWidgetConfig::Checkbox("Secondary Color", color2, true);
            ImGuiWidgetConfig::Checkbox("Glow Color", glow, true);
            ImGuiWidgetConfig::Checkbox("Regular Trail", trail, true);
            ImGuiWidgetConfig::Checkbox("Wave Trail", wave, true);
        });

        hack.setCustomWindowCocos([
            hue_speed  = hack.formatAdditionalSetting("hue_speed"),
            saturation = hack.formatAdditionalSetting("saturation"),
            value      = hack.formatAdditionalSetting("value"),
            color1     = hack.formatAdditionalSetting("color1"),
            color2     = hack.formatAdditionalSetting("color2"),
            glow       = hack.formatAdditionalSetting("glow"),
            trail      = hack.formatAdditionalSetting("trail"),
            wave       = hack.formatAdditionalSetting("wave")
        ](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            popup->addConfigFloatInput("Speed", hue_speed, 0.01f, 5.f, 0.25f);
            popup->addConfigFloatInput("Saturation", saturation, 0.f, 1.f, 0.7f);
            popup->addConfigFloatInput("Value", value, 0.f, 1.f, 1.0f);
            
            popup->addSeparator();

            popup->addConfigToggle("Primary Color", color1, true);
            popup->addConfigToggle("Secondary Color", color2, true);
            popup->addConfigToggle("Glow Color", glow, true);
            popup->addConfigToggle("Regular Trail", trail, true);
            popup->addConfigToggle("Wave Trail", wave, true);
        });
    }

    void update(float dt) {
        GJBaseGameLayer::update(dt);

        auto& config = Config::get();
        
        auto hue_speed = config.get<float>("cosmetic.rainbow_icon::hue_speed", 0.25f);
        auto saturation = config.get<float>("cosmetic.rainbow_icon::saturation", 0.7f);
        auto value = config.get<float>("cosmetic.rainbow_icon::value", 1.0f);

        auto toggle_color1 = config.get<bool>("cosmetic.rainbow_icon::color1", true);
        auto toggle_color2 = config.get<bool>("cosmetic.rainbow_icon::color2", true);
        auto toggle_glow = config.get<bool>("cosmetic.rainbow_icon::glow", true);
        auto toggle_trail = config.get<bool>("cosmetic.rainbow_icon::trail", true);
        auto toggle_wave = config.get<bool>("cosmetic.rainbow_icon::wave", true);

        static float g_rainbowHue = 0.0f;
        g_rainbowHue += hue_speed * dt;
        if (g_rainbowHue > 1.0f) g_rainbowHue -= 1.0f;

        auto getRgbColor = [&](float hue) -> cocos2d::ccColor3B {
            if (hue > 1.0f) hue -= 1.0f;
            float r, g, b; 
            GDH::Utils::hsvToRgb(hue, saturation, value, r, g, b);
            return { static_cast<GLubyte>(r * 255), static_cast<GLubyte>(g * 255), static_cast<GLubyte>(b * 255) };
        };

        cocos2d::ccColor3B mainColor = getRgbColor(g_rainbowHue);
        cocos2d::ccColor3B invColor = getRgbColor(g_rainbowHue + 0.5f);

        auto rainbowPlayer = [&](auto* player, const cocos2d::ccColor3B& primary, const cocos2d::ccColor3B& secondary) {
            if (!player) return;

            if (toggle_color1) player->setColor(primary);
            if (toggle_color2) player->setSecondColor(secondary);
            
            if (toggle_glow) {
                player->m_glowColor = primary;
            }

            if (toggle_trail && player->m_regularTrail) {
                player->m_regularTrail->setColor(primary);
            }

            if (toggle_wave && player->m_waveTrail) {
                player->m_waveTrail->setColor(primary);
            }
        };

        rainbowPlayer(m_player1, mainColor, invColor);
        
        if (m_gameState.m_isDualMode) {
            rainbowPlayer(m_player2, invColor, mainColor);
        }
    }
};