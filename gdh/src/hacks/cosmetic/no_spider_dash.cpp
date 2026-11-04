#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Spider Dash", "Disables spider dash trail when teleporting", false);

class $modify(NoSpiderDashPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Spider Dash");        
        
        hack.addHookPtr(self.getHook("PlayerObject::playSpiderDashEffect").unwrap());
    }

    void playSpiderDashEffect(cocos2d::CCPoint from, cocos2d::CCPoint to) { }
};