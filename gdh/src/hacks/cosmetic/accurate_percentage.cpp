#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../core/utils.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"
#include <imgui-cocos.hpp>

GUI_HACK_CREATE("Cosmetic", "Accurate Percentage", "Shows decimals in level progress", false);

class $modify(AccuratePercentagePlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Accurate Percentage");

        hack.addHookPtr(self.getHook("PlayLayer::updateProgressbar").unwrap());

        hack.setCustomWindowImGui([
            decimalsKey = hack.formatAdditionalSetting("demicals"),
            showMillisecondsKey = hack.formatAdditionalSetting("showMilliseconds"),
            betterCalcKey = hack.formatAdditionalSetting("betterCalculation")
        ]{
            ImGuiWidgetConfig::InputInt("Demicals", decimalsKey, 2);
            ImGuiWidgetConfig::Checkbox("Better Calculation", betterCalcKey, true);
            ImGuiWidgetConfig::Checkbox("Show time in Milliseconds", showMillisecondsKey, true);
        });

        hack.setCustomWindowCocos([
            decimalsKey = hack.formatAdditionalSetting("demicals"),
            showMillisecondsKey = hack.formatAdditionalSetting("showMilliseconds"),
            betterCalcKey = hack.formatAdditionalSetting("betterCalculation")
        ](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            popup->addConfigIntInput("Demicals", decimalsKey, 1, 10, 2);
            popup->addConfigToggle("Better Calculation", betterCalcKey, true);
            popup->addConfigToggle("Show time in Milliseconds", showMillisecondsKey, true);
        });
    }

    double customGetProgress() {
        auto& config = Config::get();
        bool betterCalc = config.get<bool>("cosmetic.accurate_percentage::betterCalculation", true);

        if (betterCalc)
            return GDH::Utils::getRealProgress(this);

        return getCurrentPercent();
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        if (m_percentageLabel == nullptr) return;

        auto& config = Config::get();

        if (m_level->isPlatformer()) {
            bool showMilliseconds = config.get<bool>("cosmetic.accurate_percentage::showMilliseconds", true);
            m_percentageLabel->setString(GDH::Utils::formatTime(m_timePlayed, showMilliseconds).c_str());
        } 
        else {
            double percent = customGetProgress();
            int decimals = config.get<int>("cosmetic.accurate_percentage::demicals", 2);

            if (decimals > 0) {
                m_percentageLabel->setString(fmt::format("{:.{}f}%", percent, decimals).c_str());
            } else {
                m_percentageLabel->setString(fmt::format("{}%", static_cast<int>(std::floor(percent))).c_str());
            }

            bool betterCalc = config.get<bool>("cosmetic.accurate_percentage::betterCalculation", true);
            if (betterCalc && m_progressFill) {
                m_progressFill->setTextureRect({
                    0, 0,
                    static_cast<float>(m_progressWidth * percent / 100.0),
                    m_progressHeight
                });
            }
        }
    }
};