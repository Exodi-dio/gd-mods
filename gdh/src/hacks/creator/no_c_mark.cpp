#include <Geode/Geode.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "No (C) Mark", "Removes copyright on copied levels", false);

class $modify(NoCMarkEditLevelLayer, EditLevelLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("No (C) Mark");        
        
        hack.addHookPtr(self.getHook("EditLevelLayer::onShare").unwrap());
    }

    void onShare(CCObject* sender) {
        m_level->m_originalLevel = 0;

        EditLevelLayer::onShare(sender);
    }
};