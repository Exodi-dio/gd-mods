#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"
#include "Geode/modify/Modify.hpp"

GUI_HACK_CREATE("Level", "Zero Practice Complete", "", false);

class $modify(ZeroPracticeCompletePlayLayer, PlayLayer) {
    struct Fields {
        bool fromStart = true;
    };

    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Zero Practice Complete");        
        
        hack.addHookPtr(self.getHook("PlayLayer::levelComplete").unwrap());
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        m_fields->fromStart = (m_gameState.m_currentProgress == 0);
    }

    void levelComplete() {
        if (m_fields->fromStart && m_isPracticeMode) {
            m_isPracticeMode = false;
            if (m_uiLayer) {
                m_uiLayer->toggleCheckpointsMenu(false);
            }
        }
        PlayLayer::levelComplete();
    }
};