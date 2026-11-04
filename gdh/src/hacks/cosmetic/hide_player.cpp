#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Hide Player", "Hides the player icon", false);

class $modify(HidePlayerPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Hide Player");        
        
        hack.addHookPtr(self.getHook("PlayerObject::toggleVisibility").unwrap());
    }

    void toggleVisibility(bool visible) {
        PlayerObject::toggleVisibility(false);
    }
};