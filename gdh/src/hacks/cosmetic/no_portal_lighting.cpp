#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Portal Lighting", "Disables lightning when entering mini/big portal", false);

class $modify(NoPortalLightingGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Portal Lighting");        
        
        hack.addHookPtr(self.getHook("GJBaseGameLayer::lightningFlash").unwrap());
    }

    void lightningFlash(cocos2d::CCPoint from, cocos2d::CCPoint to, cocos2d::ccColor3B color, float lineWidth, float duration, int displacement, bool flash, float opacity) {
        auto gm = GameManager::get();
        auto performanceMode = gm->m_performanceMode;

        gm->m_performanceMode = true;
        GJBaseGameLayer::lightningFlash(from, to, color, lineWidth, duration, displacement, flash, opacity);
        gm->m_performanceMode = performanceMode;
    }
};