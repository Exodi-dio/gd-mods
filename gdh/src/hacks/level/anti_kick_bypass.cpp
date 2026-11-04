#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Anti Kick Bypass", "Prevents kicking from certain levels", false);

class $modify(AntiKickGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Anti Kick Bypass");        
        
        hack.addHookPtr(self.getHook("GJBaseGameLayer::shouldExitHackedLevel").unwrap());
    }

    bool shouldExitHackedLevel() { return false; }
};