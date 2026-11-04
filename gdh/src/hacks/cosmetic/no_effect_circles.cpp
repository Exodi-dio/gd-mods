#include <Geode/Geode.hpp>
#include <Geode/modify/CCCircleWave.hpp>
#include <imgui-cocos.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Circles", "Removes all circles effects", false);

class $modify(NoEffectCirclesCCCircleWave, CCCircleWave) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Effect Circles");

        hack.addHookPtr(self.getHook("CCCircleWave::draw").unwrap());
    }

    void draw() {}
};