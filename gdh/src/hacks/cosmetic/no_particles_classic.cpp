#include <Geode/Geode.hpp>
#include <Geode/modify/CCParticleSystemQuad.hpp>
#include <imgui-cocos.hpp>
#include "../../core/gui.hpp"
#include "../../interface/imgui/widget_helper.hpp"
#include <string>

GUI_HACK_CREATE("Cosmetic", "No Particles Classic", "Completely disables the particle system", false);

class $modify(NoParticlesClassicCCParticleSystemQuad, CCParticleSystemQuad) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Particles Classic");

        hack.addHookPtr(self.getHook("cocos2d::CCParticleSystemQuad::draw").unwrap());
    }

    void draw() {}
};