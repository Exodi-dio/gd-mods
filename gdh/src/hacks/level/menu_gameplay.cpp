#include <Geode/Geode.hpp>
#include <Geode/modify/MenuGameLayer.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include "../../core/gui.hpp"
#include "../../core/keybinds.hpp"

GUI_HACK_CREATE("Level", "Menu Gameplay", "Manual control of the player on the main menu", false);

static bool isHolding = false;

class $modify(MenuGameplayMenuGameLayer, MenuGameLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Menu Gameplay");        
        
        hack.addHookPtr(self.getHook("MenuGameLayer::tryJump").unwrap());
        hack.addHookPtr(self.getHook("MenuGameLayer::update").unwrap());
        hack.addHookPtr(self.getHook("MenuGameLayer::ccTouchBegan").unwrap());
        hack.addHookPtr(self.getHook("MenuGameLayer::resetPlayer").unwrap());
    }

    void tryJump(float dt) {}

    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override {
        return true;
    }

    void update(float dt) override {
        if (m_playerObject && !m_playerObject->m_isSpider) {
            auto& kb = GDH::Keybinds::get();
            
            bool currentInput = isHolding ||  kb.isKeyDown(cocos2d::enumKeyCodes::KEY_W) || kb.isKeyDown(cocos2d::enumKeyCodes::KEY_Up);

            static bool wasHolding = false;

            if (currentInput && !wasHolding) {
                wasHolding = true;
                m_playerObject->pushButton(PlayerButton::Jump);
            } 
            else if (!currentInput && wasHolding) {
                wasHolding = false;
                m_playerObject->releaseButton(PlayerButton::Jump);
            }
        }

        MenuGameLayer::update(dt);
    }  
    
    void resetPlayer() {
        MenuGameLayer::resetPlayer();
        if (m_playerObject) {
            m_playerObject->setPositionY(105.f);
        }
    }
};

class $modify(MenuGameplayCCTouchDispatcher, CCTouchDispatcher) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Level").findHackByName("Menu Gameplay");        
        
        hack.addHookPtr(self.getHook("cocos2d::CCTouchDispatcher::touches").unwrap());
    }

    void touches(CCSet* touches, CCEvent* event, unsigned int type) {
        if (type == CCTOUCHBEGAN) {
            isHolding = true;
        }
        else if (type == CCTOUCHENDED || type == CCTOUCHCANCELLED) {
            isHolding = false;
        }

        CCTouchDispatcher::touches(touches, event, type);
    }
};