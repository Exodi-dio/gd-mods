#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Hide UI", "Hides the editor Ul while building", false);

class $modify(HideUIEditorUI, EditorUI) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Hide UI");  
        
        hack.addHookPtr(self.getHook("EditorUI::init").unwrap());
        hack.setHandler([](bool enabled) {
            if (auto LevelEditorLayer = LevelEditorLayer::get()) {
                auto EditorUILayer = LevelEditorLayer->m_editorUI;
                if (EditorUILayer) EditorUILayer->setVisible(!enabled);
            }
        });
    }
    
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        this->setVisible(false);

        return true;
    }
};