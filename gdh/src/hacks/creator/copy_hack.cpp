#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Copy Hack", "Copy any online level without a password", false);

class $modify(CopyHackLevelInfoLayer, LevelInfoLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Copy Hack");        
        
        hack.addHookPtr(self.getHook("LevelInfoLayer::init").unwrap());
        hack.addHookPtr(self.getHook("LevelInfoLayer::levelDownloadFinished").unwrap());
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        level->m_password = 1;

        return true;
    }

    void levelDownloadFinished(GJGameLevel *level) {
        level->m_password = 1;
        LevelInfoLayer::levelDownloadFinished(level);
    }
};