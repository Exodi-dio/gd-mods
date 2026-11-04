#include <Geode/Geode.hpp>
#include <Geode/modify/LevelPage.hpp>
#include <Geode/modify/LevelSelectLayer.hpp>
#include <chrono>
#include "../../core/gui.hpp"

static std::chrono::steady_clock::time_point lastScrollTime;

GUI_HACK_CREATE("Cosmetic", "Accident \"The Tower\" Click", "Removes accidental entry into \"The Tower\" when quickly scrolling through the list of official levels", false);

class $modify(AccidentTowerClickFixLevelPage, LevelPage) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Accident \"The Tower\" Click");        
        hack.addHookPtr(self.getHook("LevelPage::onTheTower").unwrap());
    }

    void onTheTower(cocos2d::CCObject* sender) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastScrollTime).count();

        if (duration < 1) return;

        LevelPage::onTheTower(sender);
    }
};

class $modify(AccidentTowerClickFixLevelSelectLayer, LevelSelectLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Accident \"The Tower\" Click");        
        hack.addHookPtr(self.getHook("LevelSelectLayer::onNext").unwrap());
        hack.addHookPtr(self.getHook("LevelSelectLayer::onPrev").unwrap());
    }

    void onNext(cocos2d::CCObject* sender) {
        lastScrollTime = std::chrono::steady_clock::now();
        LevelSelectLayer::onNext(sender);
    }

    void onPrev(cocos2d::CCObject* sender) {
        lastScrollTime = std::chrono::steady_clock::now();
        LevelSelectLayer::onPrev(sender);
    }
};