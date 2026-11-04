#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Exit Dual Effect", "Disables the leave animation for the second player (useful for booting)", false);

class $modify(NoExitDualEffectGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Exit Dual Effect");        
        
        hack.addHookPtr(self.getHook("GJBaseGameLayer::playExitDualEffect").unwrap());
    }

    void playExitDualEffect(PlayerObject *player) {}
};