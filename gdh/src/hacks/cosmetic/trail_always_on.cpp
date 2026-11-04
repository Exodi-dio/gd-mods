#include <Geode/Geode.hpp>
#include <Geode/modify/CCMotionStreak.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Trail Always On", "Displays the trail near the player at any location", false);

class $modify(TrailAlwaysOnCCMotionStreak, cocos2d::CCMotionStreak) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Trail Always On");        
        
        hack.addHookPtr(self.getHook("cocos2d::CCMotionStreak::update").unwrap());
    }

    void update(float delta) {
        m_bStroke = true;
        CCMotionStreak::update(delta);
    }
};