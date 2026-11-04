#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <imgui-cocos.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "No Mirror", "Disables level mirroring", true);

class $modify(NoMirrorGJBaseGameLayer, GJBaseGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("No Mirror");

        hack.addHookPtr(self.getHook("GJBaseGameLayer::canBeActivatedByPlayer").unwrap());
    }

    bool canBeActivatedByPlayer(PlayerObject *p0, EffectGameObject *p1) {
        if (p1->m_objectType == GameObjectType::InverseMirrorPortal) return false;

        return GJBaseGameLayer::canBeActivatedByPlayer(p0, p1);
    }
};