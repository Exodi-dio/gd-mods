#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No New Best Popup", "Disable the new best popup", false);

static bool needCreateAction = false;

class $modify(NoNewBestPopupPlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No New Best Popup");
        hack.addHookPtr(self.getHook("PlayLayer::showNewBest").unwrap());
        hack.addHookPtr(self.getHook("PlayLayer::destroyPlayer").unwrap());
    }

    void showNewBest(bool p0, int p1, int p2, bool p3, bool p4, bool p5) {
        needCreateAction = !p4;
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        PlayLayer::destroyPlayer(player, obj);
        if (needCreateAction) {
            auto* newSequence = cocos2d::CCSequence::create(
                cocos2d::CCDelayTime::create(1.f),
                cocos2d::CCCallFunc::create(this, callfunc_selector(PlayLayer::delayedResetLevel)),
                nullptr
            );
            newSequence->setTag(0x10);
            runAction(newSequence);
        }
    }
};