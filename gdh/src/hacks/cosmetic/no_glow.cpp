#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <imgui-cocos.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Glow", "Disables glow on objects", false);

class $modify(NoGlowPlayLayer, PlayLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Glow");

        hack.addHookPtr(self.getHook("PlayLayer::addObject").unwrap());
    }

    void addObject(GameObject* obj) {
        obj->m_hasNoGlow = true;
        PlayLayer::addObject(obj);
    }
};