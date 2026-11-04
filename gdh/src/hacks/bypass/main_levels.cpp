#include <Geode/Geode.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include <Geode/modify/LevelAreaInnerLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Bypass", "Main Levels", "Unlocks all main levels", false);

class $modify(MainLevelsGameLevelManager, GameLevelManager) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Main Levels");        
        
        hack.addHookPtr(self.getHook("GameLevelManager::getMainLevel").unwrap());
    }

    GJGameLevel* getMainLevel(int levelID, bool dontGetLevelString) {
        auto level = GameLevelManager::getMainLevel(levelID, dontGetLevelString);
        if (level->m_requiredCoins > 0) {
            level->m_requiredCoins = 0;
        }
        return level;
    }
};

class $modify(MainLevelsAreaInnerLayer, LevelAreaInnerLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Main Levels");        
        
        hack.addHookPtr(self.getHook("LevelAreaInnerLayer::init").unwrap());
    }

    bool init(bool returning) {
        static constexpr std::array level_ids = {5001, 5002, 5003, 5004};
        auto gsm = GameStatsManager::get();
        
        std::unordered_map<int, bool> original_completion;
        for (int level : level_ids) {
            original_completion[level] = gsm->hasCompletedMainLevel(level);
            gsm->m_completedLevels->setObject(
                cocos2d::CCBool::create(true), 
                fmt::format("n_{}", level)
            );
        }
        
        if (!LevelAreaInnerLayer::init(returning)) return false;
        
        for (auto [level, was_completed] : original_completion) {
            if (!was_completed) {
                gsm->m_completedLevels->removeObjectForKey(fmt::format("n_{}", level));
            }
        }
        
        return true;
    }
};