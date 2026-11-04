#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Pause During Complete", "", false);

class $modify(PauseDuringCompletePlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Pause During Complete");

        hack.addHookPtr(self.getHook("PlayLayer::pauseGame").unwrap());
        hack.addHookPtr(self.getHook("PlayLayer::resetLevel").unwrap());
        hack.addHookPtr(self.getHook("PlayLayer::playEndAnimationToPos").unwrap());
        hack.addHookPtr(self.getHook("PlayLayer::playPlatformerEndAnimationToPos").unwrap());
    }

    void pauseGame(bool unfocused) {
        bool orig = m_levelEndAnimationStarted;
        m_levelEndAnimationStarted = false;        
        PlayLayer::pauseGame(unfocused);
        m_levelEndAnimationStarted = orig;
    }

    void resetLevel() {
        if (m_levelEndAnimationStarted) {
            m_player1->stopAllActions();
            m_player2->stopAllActions();
        }
        PlayLayer::resetLevel();
    }

    void playEndAnimationToPos(cocos2d::CCPoint pos) {
        PlayLayer::playEndAnimationToPos(pos);
        m_uiLayer->m_pauseBtn->setEnabled(true);
    }

    void playPlatformerEndAnimationToPos(cocos2d::CCPoint pos, bool instant) {
        PlayLayer::playPlatformerEndAnimationToPos(pos, instant);
        m_uiLayer->m_pauseBtn->setEnabled(true);
    }
};