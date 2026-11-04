#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Background Flash", "Removes the unpleasant flicker when triggering the portal", false);

class $modify(NoBackgroundFlashGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Background Flash");        
        
        hack.addHookPtr(self.getHook("GJBaseGameLayer::lightningFlash").unwrap());
        hack.addHookPtr(self.getHook("GJBaseGameLayer::teleportPlayer").unwrap());
    }

    void teleportPlayer(TeleportPortalObject *p0, PlayerObject *p1) {
        bool hasNoEffects = p0->m_hasNoEffects;

        p0->m_hasNoEffects = true;
        GJBaseGameLayer::teleportPlayer(p0, p1);
        p0->m_hasNoEffects = hasNoEffects;
    }

    void lightningFlash(cocos2d::CCPoint from, cocos2d::CCPoint to, cocos2d::ccColor3B color, float lineWidth, float duration, int displacement, bool flash, float opacity) {
        flash = false;
        GJBaseGameLayer::lightningFlash(from, to, color, lineWidth, duration, displacement, flash, opacity);
    }
};