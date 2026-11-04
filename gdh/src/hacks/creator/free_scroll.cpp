#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Free Scroll", "Allows scrolling out of the editor", false);

class $modify(FreeScrollEditorUI, EditorUI) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Free Scroll");        
        
        hack.addHookPtr(self.getHook("EditorUI::constrainGameLayerPosition").unwrap());
    }

    void constrainGameLayerPosition(float width, float height) {}
};