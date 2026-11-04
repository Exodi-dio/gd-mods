#include <Geode/Geode.hpp>
#include <Geode/modify/ShaderLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Shaders", "Disabling shaders in levels", true);

class $modify(NoShadersShaderLayer, ShaderLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Shaders");        
        
        hack.addHookPtr(self.getHook("ShaderLayer::performCalculations").unwrap());
    }

    void performCalculations() {
        m_state.m_usesShaders = false;
    }
};