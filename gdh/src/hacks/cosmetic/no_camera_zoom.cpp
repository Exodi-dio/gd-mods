#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Camera Zoom", "Disables camera zooming via trigger", true);

class $modify(NoCameraZoomGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Camera Zoom");        
        
        hack.addHookPtr(self.getHook("GJBaseGameLayer::updateZoom").unwrap());
    }

    void updateZoom(float p0, float p1, int p2, float p3, int p4, int p5) {}
};