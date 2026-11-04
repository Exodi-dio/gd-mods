#include <Geode/Geode.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Verify Hack", "Publish a level without verification", false);

class $modify(VerifyHackEditLevelLayer, EditLevelLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Verify Hack");        
        
        hack.addHookPtr(self.getHook("EditLevelLayer::init").unwrap());
    }

    bool init(GJGameLevel *gjgl) {
        gjgl->m_isVerified = true;
        return EditLevelLayer::init(gjgl);
    }
};