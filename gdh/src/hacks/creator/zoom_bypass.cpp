#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Zoom Bypass", "Lets you zoom an infinite amount in the editor", false);

class $modify(ZoomBypassEditorUI, EditorUI) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Zoom Bypass");   
        
        #if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_MACOS) 
        (void) self.setHookPriority("EditorUI::zoomIn", geode::Priority::Early); 
        (void) self.setHookPriority("EditorUI::zoomOut", geode::Priority::Early); 
        (void) self.setHookPriority("EditorUI::scrollWheel", geode::Priority::Early); 
        
        hack.addHookPtr(self.getHook("EditorUI::zoomIn").unwrap());
        hack.addHookPtr(self.getHook("EditorUI::zoomOut").unwrap());
        hack.addHookPtr(self.getHook("EditorUI::scrollWheel").unwrap());
        #elif defined(GEODE_IS_ANDROID)
        (void) self.setHookPriority("EditorUI::zoomGameLayer", geode::Priority::Early); 
        hack.addHookPtr(self.getHook("EditorUI::zoomGameLayer").unwrap());
        #endif
    }

    void zoomBypass(bool in) {
        float scale = m_editorLayer->m_groundLayer->getScale();
        float step = (!in && scale <= 0.105f) || (in && scale < 0.095f) ? 0.01f : 0.1f;

        scale += in ? step : -step;
        this->updateZoom(std::max(scale, 0.01f));
    }

    #if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_MACOS) 

    void zoomIn(cocos2d::CCObject* sender) {
        zoomBypass(true);
    }

    void zoomOut(cocos2d::CCObject* sender) {
        zoomBypass(false);
    }

    void scrollWheel(float y, float x) {
        auto scale = m_editorLayer->m_groundLayer->getScale();

        EditorUI::scrollWheel(y, x);

        if (this->m_editorLayer->m_playbackMode != PlaybackMode::Playing && CCKeyboardDispatcher::get()->getControlKeyPressed()) {
            m_editorLayer->m_groundLayer->setScale(scale);

            if (y <= 0.0 && x <= 0.0)
                zoomBypass(true);
            else
                zoomBypass(false);
        }
    }

    #elif defined(GEODE_IS_ANDROID)

    void zoomGameLayer(bool zoomingIn) {
        zoomBypass(zoomingIn);
    }
    
    #endif
};