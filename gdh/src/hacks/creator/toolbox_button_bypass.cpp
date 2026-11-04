#include <Geode/Geode.hpp>
#include <Geode/modify/EditorOptionsLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Toolbox Button Bypass", "Allows for more objects in the editor toolbox", false);

class $modify(ToolboxButtonBypassEditorOptionsLayer, EditorOptionsLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Creator").findHackByName("Toolbox Button Bypass");        
        
        hack.addHookPtr(self.getHook("EditorOptionsLayer::onButtonRows").unwrap());
        hack.addHookPtr(self.getHook("EditorOptionsLayer::onButtonsPerRow").unwrap());
    }

    void onButtonRows(cocos2d::CCObject* sender)  {
        m_buttonRows += sender->getTag() ? 1 : -1;
        m_buttonRows = std::max(1, m_buttonRows);
        m_buttonRowsLabel->setString(fmt::format("{}", m_buttonRows).c_str());
    }

    void onButtonsPerRow(cocos2d::CCObject* sender) {
        m_buttonsPerRow += sender->getTag() ? 1 : -1;
        m_buttonsPerRow = std::max(1, m_buttonsPerRow);
        m_buttonsPerRowLabel->setString(fmt::format("{}", m_buttonsPerRow).c_str());
    }
};