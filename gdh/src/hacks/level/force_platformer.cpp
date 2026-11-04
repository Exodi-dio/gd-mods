#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"
#include "Geode/modify/Modify.hpp"

GUI_HACK_CREATE("Level", "Force Platformer", "Enables platformer mode on all levels", false);

class $modify(ForcePlatformerPlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Force Platformer");        
        
        hack.addHookPtr(self.getHook("PlayLayer::init").unwrap());
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (m_player1) m_player1->togglePlatformerMode(true);
        if (m_player2) m_player2->togglePlatformerMode(true);

        #ifdef GEODE_IS_MOBILE
        if (m_uiLayer) m_uiLayer->togglePlatformerMode(true);
        #endif

        return true;
    }
};