#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Jump Hack", "Removes the barrier to jump gravity", true);

class $modify(JumpHackGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Jump Hack");        
        
        hack.addHookPtr(self.getHook("GJBaseGameLayer::update").unwrap());
    }

    void update(float dt) {
        if (m_player1) m_player1->m_isOnGround = true;
        if (m_player2) m_player2->m_isOnGround = true;
        GJBaseGameLayer::update(dt);
    }
};