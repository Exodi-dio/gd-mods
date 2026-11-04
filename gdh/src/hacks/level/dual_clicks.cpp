#include <Geode/Geode.hpp>
#include "../../core/gui.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include <imgui-cocos.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../interface/cocos/hack_settings_popup.hpp"

GUI_HACK_CREATE("Level", "Dual Clicks", "Allows you to press two buttons at the same time (useful for botting)", true);

$execute {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Level").findHackByName("Dual Clicks");

    hack.setCustomWindowImGui([] {
        ImGuiWidgetConfig::Checkbox("Invert click for the opposite player", "level.dual_clicks::invert_click", false);
    });

    hack.setCustomWindowCocos([](cocos2d::CCNode* popupNode) {
        auto* popup = static_cast<HackSettingsPopup*>(popupNode);
        popup->addConfigToggle("Invert click for the opposite player", "level.dual_clicks::invert_click", false);
    });
}