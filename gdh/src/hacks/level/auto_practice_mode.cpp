#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Level", "Auto Practice Mode", "Auto-enables practice mode", false);

class $modify(AutoPracticeModePlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Auto Practice Mode");        
        
        hack.addHookPtr(self.getHook("PlayLayer::setupHasCompleted").unwrap());
    }

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        this->togglePracticeMode(true);
    }
};