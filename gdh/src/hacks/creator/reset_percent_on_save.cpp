#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Reset Percent on Save", "", false);

class $modify(ResetPercentOnSaveEditorPauseLayer, EditorPauseLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Reset Percent on Save");        
        
        hack.addHookPtr(self.getHook("EditorPauseLayer::saveLevel").unwrap());
    }

    void saveLevel() {
        if (m_editorLayer->m_level->m_levelType == GJLevelType::Editor)
            m_editorLayer->m_level->m_normalPercent = 0;

        EditorPauseLayer::saveLevel();
    }
};