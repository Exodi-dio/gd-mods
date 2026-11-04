#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Smooth Editor Trail", "Makes the wave smoother in the editor", false);

class $modify(SmoothEditorTrailLevelEditorLayer, LevelEditorLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Smooth Editor Trail");        
        
        hack.addHookPtr(self.getHook("LevelEditorLayer::postUpdate").unwrap());
    }

    void postUpdate(float dt) {
        m_trailTimer = 0.1f;
        LevelEditorLayer::postUpdate(dt);
    }
};