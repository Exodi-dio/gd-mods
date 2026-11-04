#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/LevelTools.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Level Edit", "Edit any online level", false);

class $modify(LevelEditPauseLayer, PauseLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Level Edit");        
        
        hack.addHookPtr(self.getHook("PauseLayer::customSetup").unwrap());
        hack.addHookPtr(self.getHook("PauseLayer::onTryEdit").unwrap());
    }

    void customSetup() {
        auto pl = PlayLayer::get();
        auto levelType = pl->m_level->m_levelType;

        pl->m_level->m_levelType = GJLevelType::Editor;
        PauseLayer::customSetup();
        pl->m_level->m_levelType = levelType;
    }

    void onTryEdit(cocos2d::CCObject* sender) {
        auto pl = PlayLayer::get();
        auto levelType = pl->m_level->m_levelType;

        pl->m_level->m_levelType = GJLevelType::Editor;
        PauseLayer::onTryEdit(sender);
        pl->m_level->m_levelType = levelType;
    }
};

class $modify(LevelEditEditorUI, EditorUI) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Level Edit");        
        
        hack.addHookPtr(self.getHook("EditorUI::onSettings").unwrap());
    }

    void onSettings(cocos2d::CCObject* sender) {
        auto lel = LevelEditorLayer::get();
        auto levelType = lel->m_level->m_levelType;

        lel->m_level->m_levelType = GJLevelType::Editor;
        EditorUI::onSettings(sender);
        lel->m_level->m_levelType = levelType;
    }
};

class $modify(LevelEditEditorPauseLayer, EditorPauseLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Level Edit");        
        
        hack.addHookPtr(self.getHook("EditorPauseLayer::init").unwrap());
    }

    bool init(LevelEditorLayer* p0) {
        auto lel = LevelEditorLayer::get();
        auto levelType = lel->m_level->m_levelType;

        lel->m_level->m_levelType = GJLevelType::Editor;
        auto ret = EditorPauseLayer::init(p0);
        lel->m_level->m_levelType = levelType;

        return ret;
    }
};

class $modify(LevelEditLevelTools, LevelTools) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Level Edit");        
        
        hack.addHookPtr(self.getHook("LevelTools::verifyLevelIntegrity").unwrap());
    }

    static bool verifyLevelIntegrity(gd::string p0, int p1) {
        if (LevelTools::verifyLevelIntegrity(p0, p1))
            return true;
            
        return true; // always true
    }
};