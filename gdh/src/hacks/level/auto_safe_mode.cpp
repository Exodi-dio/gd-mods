#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"
#include "../../core/replayEngine.hpp"
#include "../../core/config.hpp"
#include "../../core/labels.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include "../../interface/cocos/hack_settings_popup.hpp"

using namespace geode::prelude;

GUI_HACK_CREATE("Level", "Auto Safe Mode", "Disables progress on levels when enabled unlegit hacks", false);
GUI_HACK_CREATE("Level", "Cheat Indicator", "Disables progress on levels when enabled unlegit hacks", false);

class $modify(AutoSafeModeEndLevelLayer, EndLevelLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Cheat Indicator");        
        
        hack.setCustomWindowImGui([
            indicate_when_cheating = hack.formatAdditionalSetting("indicate_when_cheating")
        ] {
            ImGuiWidgetConfig::Checkbox("Only indicate when cheating", indicate_when_cheating, false);
        });

        hack.setCustomWindowCocos([
            indicate_when_cheating = hack.formatAdditionalSetting("indicate_when_cheating")
        ](cocos2d::CCNode* popupNode) {
            auto* popup = static_cast<HackSettingsPopup*>(popupNode);
            popup->addConfigToggle("Only indicate when cheating", indicate_when_cheating, false);
        });
    }

    void showLayer(bool p0) {
        EndLevelLayer::showLayer(p0);

        auto& config = Config::get();
        bool safe_mode = config.get<bool>("core.safe_mode", false);
        bool auto_safe_mode = config.get<bool>("level.auto_safe_mode", false);
        bool cheat_indicator = config.get<bool>("level.cheat_indicator", false);

        if (!auto_safe_mode && !cheat_indicator)
            return;

        bool& cheatedInCurrentAttempt = GDH::Labels::Manager::get().m_cheatedInCurrentAttempt;
        auto& activeCheats = GDH::Gui::get().getActiveCheats();

        auto& engine = GDH::ReplayEngine::get();
        bool replay_engine = (engine.mode != state::disable);

        std::string text = "";
        cocos2d::ccColor3B textColor = {255, 128, 128};

        if (safe_mode) {
            for (const auto& hackID : activeCheats)
                text += hackID + "\n";

            if (replay_engine)
                text += "Replay Engine\n";

            text += "Safe Mode (GDH)";
            textColor = {255, 165, 0};
        }
        else if (activeCheats.empty() && !cheatedInCurrentAttempt && !replay_engine) {
            text = "Legit (GDH)";
            textColor = {128, 255, 128};
        } 
        else {
            if (activeCheats.empty() && cheatedInCurrentAttempt) 
                text += "Cheats disabled during session\n";

            for (const auto& hackID : activeCheats)
                text += hackID + "\n";

            if (replay_engine)
                text += "Replay Engine\n";
            
            if (auto_safe_mode) {
                text += "Auto Safe Mode (GDH)";
                textColor = {255, 165, 0};
            } else {
                text += "Cheated (GDH)";
            }
        }

        if (auto* label = geode::Label::create(text, "GoogleSans.fnt"_spr)) {
            label->setScale(0.40f);
            label->setAnchorPoint({0.0f, 0.0f});
            label->setPosition({5.0f, 4.0f});
            label->setColor(textColor);
            label->setOpacity(180);

            auto labelSize = label->getScaledContentSize();

            auto* bg = CCLayerColor::create(ccc4(0, 0, 0, 125), labelSize.width + 4.0f, labelSize.height + 2.0f);
            bg->setAnchorPoint({0.0f, 0.0f});
            bg->setPosition({3.0f, 3.0f});

            this->addChild(bg, 9998);
            this->addChild(label, 9999);
        }
    }
};

class $modify(AutoSafeModePlayLayer, PlayLayer) {
    struct Fields { ~Fields() { GDH::Labels::Manager::get().m_cheatedInCurrentAttempt = false; } };

    void checkCheats() {
        bool& cheatedInCurrentAttempt = GDH::Labels::Manager::get().m_cheatedInCurrentAttempt;
        if (!cheatedInCurrentAttempt) {
            auto& config = Config::get();
            auto& engine = GDH::ReplayEngine::get();
            
            bool replay_engine = (engine.mode != state::disable);
            
            auto& activeCheats = GDH::Gui::get().getActiveCheats();
            if (!activeCheats.empty() || replay_engine) {
                cheatedInCurrentAttempt = true;
            }
        }
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontRun) {
        if (!PlayLayer::init(level, useReplay, dontRun)) return false;
        m_fields.self();
        checkCheats();
        return true;
    }

    void updateProgressbar() {
        checkCheats();
        PlayLayer::updateProgressbar();
    }

    void resetLevel() {
        GDH::Labels::Manager::get().m_cheatedInCurrentAttempt = false;
        checkCheats();
        PlayLayer::resetLevel();
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        bool originalTestMode = m_isTestMode;
        bool& cheatedInCurrentAttempt = GDH::Labels::Manager::get().m_cheatedInCurrentAttempt;

        auto& config = Config::get();  
        if (cheatedInCurrentAttempt && config.get<bool>("level.auto_safe_mode", false)) {
            m_isTestMode = true; 
        }

        PlayLayer::destroyPlayer(player, obj);
        m_isTestMode = originalTestMode;
    }

    void levelComplete() {
        bool originalTestMode = m_isTestMode;
        
        auto& config = Config::get();  

        bool& cheatedInCurrentAttempt = GDH::Labels::Manager::get().m_cheatedInCurrentAttempt;
        if (cheatedInCurrentAttempt && config.get<bool>("level.auto_safe_mode", false)) {
            m_isTestMode = true;
        }

        PlayLayer::levelComplete();
        m_isTestMode = originalTestMode;
    }
};