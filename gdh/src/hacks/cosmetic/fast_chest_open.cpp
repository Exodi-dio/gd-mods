#include <Geode/Geode.hpp>
#include <Geode/modify/RewardUnlockLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Cosmetic", "Fast Chest Open", "Removes the delay for opening chests", false);

class $modify(FastCheckOpenRewardUnlockLayer, RewardUnlockLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Cosmetic").findHackByName("Fast Chest Open");        
        
        hack.addHookPtr(self.getHook("RewardUnlockLayer::init").unwrap());
    }

    bool init(int p0, RewardsPage *p1)
    {
        if (!RewardUnlockLayer::init(p0, p1))
            return false;

        auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
        m_chestSprite->stopAllActions();
        m_chestSprite->setPosition({winSize.width / 2, (winSize.height / 2) - 20});
        m_chestSprite->runAction(
            cocos2d::CCSequence::create(
                cocos2d::CCDelayTime::create(0.4),
                cocos2d::CCCallFunc::create(this, callfunc_selector(RewardUnlockLayer::step3)),
                nullptr
            )
        );

        return true;
    }
};