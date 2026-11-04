#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Ignore ESC", "Prevents exiting the level", false);

class $modify(IgnoreESCPauseLayer, PauseLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Ignore ESC");  
        
        (void) self.setHookPriority("PauseLayer::onQuit", Priority::VeryLate); 
        hack.addHookPtr(self.getHook("PauseLayer::onQuit").unwrap());
    }
    
    void onQuit(cocos2d::CCObject* sender) {}
};