#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Stop Triggers On Death", "Stops move/rotation triggers on death so you can see what killed you", false);

class $modify(StopTriggersOnDeathGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        (void) self.setHookPriority("GJBaseGameLayer::update", geode::Priority::Last); 

        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Stop Triggers On Death");

        hack.addHookPtr(self.getHook("GJBaseGameLayer::update").unwrap());
    }

    void update(float dt) {
        if (!m_isEditor && m_playerDied) return;
        
        GJBaseGameLayer::update(dt);
    }
};