#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"
#include "Geode/modify/Modify.hpp"

GUI_HACK_CREATE("Level", "Instant Complete", "Instant level completion", true);

class $modify(InstantCompletePlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Instant Complete");        
        
        (void) self.setHookPriority("PlayLayer::resetLevel", Priority::Late); 
        hack.addHookPtr(self.getHook("PlayLayer::resetLevel").unwrap());
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        
        //bypass anticheat
        m_timePlayed = 10.0;
        m_bestAttemptTime = 10.0;

        PlayLayer::playEndAnimationToPos({0, 0});
    }
};