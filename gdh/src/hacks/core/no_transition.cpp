#include <Geode/Geode.hpp>
#include <Geode/modify/CCTransitionFade.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Core", "No Transition Old", "Fast transition between scenes (legacy)", false);

class $modify(NoTransitionCCTransitionFade, cocos2d::CCTransitionFade) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Core").findHackByName("No Transition Old");        
        
        hack.addHookPtr(self.getHook("cocos2d::CCTransitionFade::create").unwrap());
    }

    static cocos2d::CCTransitionFade* create(float t, cocos2d::CCScene *scene) {
        return cocos2d::CCTransitionFade::create(0, scene);
    }
};