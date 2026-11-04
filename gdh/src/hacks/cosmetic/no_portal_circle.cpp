#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Portal Circle", "Disables the circles inside portals that appear when they are activated", false);

class $modify(NoPortalCirclePlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Portal Circle");        
        
        hack.addHookPtr(self.getHook("PlayerObject::spawnPortalCircle").unwrap());
    }

    void spawnPortalCircle(cocos2d::ccColor3B color, float startRadius) {}
};