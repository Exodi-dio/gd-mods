#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/CurrencyRewardLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Hide Complete Window", "", false);

class $modify(HideCompleteWindowEndLevelLayer, EndLevelLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Hide Complete Window");        
        
        hack.addHookPtr(self.getHook("EndLevelLayer::showLayer").unwrap());
    }

    void showLayer(bool instant) {
        auto gm = GameManager::get();
        bool fastMenu = gm->getGameVariable(GameVar::FastMenu);
        gm->setGameVariable(GameVar::FastMenu, true);

        EndLevelLayer::showLayer(instant);
        gm->setGameVariable(GameVar::FastMenu, fastMenu);

        this->m_mainLayer->setPosition(m_startPosition);
        this->setOpacity(0);

        m_hidden = true;
    }
};

class $modify(HideCompleteWindowCurrencyRewardLayer, CurrencyRewardLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Hide Complete Window");    
        
        hack.addHookPtr(self.getHook("CurrencyRewardLayer::init").unwrap());
    }

    bool init(int p0, int p1, int p2, int p3, CurrencySpriteType p4, int p5, CurrencySpriteType p6, int p7, cocos2d::CCPoint p8, CurrencyRewardType p9, float p10, float p11) {
        if (!CurrencyRewardLayer::init(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)) return false;

        this->setVisible(false);

        return true;
    }
};