#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Auto Song Download", "Automatic downloading of song when you enter an online level", false);

class $modify(AutoSongDownloadLevelInfoLayer, LevelInfoLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Auto Song Download");        
        
        hack.addHookPtr(self.getHook("LevelInfoLayer::init").unwrap());
    }

    bool init(GJGameLevel* p0, bool p1) {
        if (!LevelInfoLayer::init(p0, p1))
            return false;

        if (m_songWidget->m_downloadBtn->isVisible()) {
            GameManager::sharedState()->setGameVariable("0016", true); // Bypass TOS
            m_songWidget->m_downloadBtn->activate();
        }            

        return true;
    }
};