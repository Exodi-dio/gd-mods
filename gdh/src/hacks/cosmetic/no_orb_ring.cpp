#include <Geode/Geode.hpp>
#include <Geode/modify/RingObject.hpp>
#include <imgui-cocos.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Orb Ring", "Disables the rings that appear when you touch the orbs", false);

class $modify(NoOrbRingRingObject, RingObject) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Orb Ring");

        hack.addHookPtr(self.getHook("RingObject::spawnCircle").unwrap());
    }

    void spawnCircle() {}
};