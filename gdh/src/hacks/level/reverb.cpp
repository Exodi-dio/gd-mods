#include <Geode/Geode.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../core/utils.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "imgui.h"
#include <imgui-cocos.hpp>
#include "../../interface/cocos/hack_settings_popup.hpp"

GUI_HACK_CREATE("Level", "Reverb Effect", "", false);

$execute {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Level").findHackByName("Reverb Effect");    
    auto& config = Config::get();    
    hack.setEarlyInit(false);

    hack.setHandler([
        &config, valueKey = hack.formatAdditionalSetting("value")
    ](bool enabled) {
        GDH::Utils::setReverb(enabled ? config.get<float>(valueKey, 0.f) : 0.f);
    });

    hack.setCustomWindowImGui([
        &config, valueKey = hack.formatAdditionalSetting("value")]() {
        ImGui::SetNextItemWidth(160.f);
        if (ImGuiWidgetConfig::DragFloat("##reverb_effect::value", "level.reverb_effect::value", 100.f, 100.f, 20000.f, 10000.f, "Decay Time: %.2fms")) {
            GDH::Utils::setReverb(config.get<float>(valueKey, 0.f));
        }
    });

    hack.setCustomWindowCocos([
        &config, valueKey = hack.formatAdditionalSetting("value")
    ](cocos2d::CCNode* popupNode) {
        auto* popup = static_cast<HackSettingsPopup*>(popupNode);
        popup->addConfigFloatInput("Delay Time (100 to 20000)", "level.reverb_effect::value", 100.f, 20000.f, 10000.f, [](float value) {
            GDH::Utils::setReverb(value);
        });
    });
}