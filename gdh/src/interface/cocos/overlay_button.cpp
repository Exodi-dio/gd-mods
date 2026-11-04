#include "overlay_button.hpp"
#include "../../core/config.hpp"
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/GameManager.hpp>

using namespace cocos2d;
using namespace geode::prelude;

OverlayButton* OverlayButton::instance = nullptr;

OverlayButton* OverlayButton::create(const char* file) {
    auto* ret = new OverlayButton();
    if (ret->init(file)) { ret->autorelease(); return ret; }
    delete ret;
    return nullptr;
}

OverlayButton* OverlayButton::get(const char* file) {
    if (!instance) {
        instance = create(file);
    }
    return instance;
}

bool OverlayButton::init(const char* file) {
    if (!CCMenu::init()) return false;

    auto& config = Config::get();
    m_scale = config.get<float>("ui_icon.scale", 0.5f);
    
    m_sprite = CCSprite::create(file);
    m_sprite->setScale(m_scale);

    float m_minOpacity = config.get<float>("ui_icon.minOpacity", 0.80f);
    m_sprite->setOpacity(m_minOpacity * 255);

    m_sprite->setPosition({
        config.get<float>("ui_icon.x", 80.f),
        config.get<float>("ui_icon.y", 80.f)
    });
    m_target = m_sprite->getPosition();
    addChild(m_sprite);

    setZOrder(1000);
    setPosition({0, 0});
    scheduleUpdate();

    bool isVisible = config.get<bool>("ui_icon.visible", true);
    this->setVisible(isVisible);

    return true;
}

void OverlayButton::setSizeScale(float scale) {
    m_scale = scale;
    if (m_sprite) {
        m_sprite->setScale(m_scale);
    }
    Config::get().set<float>("ui_icon.scale", m_scale);
}

void OverlayButton::setVisible(bool visible) {
    if (isVisible() == visible) return;
    
    CCMenu::setVisible(visible);

    if (auto dispatcher = CCDirector::sharedDirector()->getTouchDispatcher()) {
        if (visible) {
            dispatcher->addTargetedDelegate(this, -1000, true);
        } else {
            dispatcher->removeDelegate(this);
        }
    }

    Config::get().set<bool>("ui_icon.visible", visible);
}

void OverlayButton::update(float dt) {
    auto win_size = CCDirector::sharedDirector()->getWinSize();
    float r = radius();
    m_target.x = std::clamp(m_target.x, r, win_size.width - r);
    m_target.y = std::clamp(m_target.y, r, win_size.height - r);

    CCPoint pos = m_sprite->getPosition();
    if (ccpDistance(pos, m_target) < 0.5f) return;
    m_sprite->setPosition(ccpLerp(pos, m_target, 12.f * dt));
}

bool OverlayButton::ccTouchBegan(CCTouch* t, CCEvent*) {
    if (!isVisible()) return false;

    CCPoint tp = convertToNodeSpace(t->getLocation());
    if (ccpDistance(tp, m_sprite->getPosition()) > radius()) return false;

    m_moved = false;
    m_sprite->stopAllActions();
    m_sprite->runAction(CCScaleTo::create(0.08f, m_scale * 0.85f));
    m_sprite->runAction(CCFadeTo::create(0.08f, 255));
    return true;
}

void OverlayButton::ccTouchMoved(CCTouch* t, CCEvent*) {
    CCPoint tp = convertToNodeSpace(t->getLocation());
    if (!m_moved && ccpDistance(tp, m_sprite->getPosition()) < radius()) return;
    m_moved = true;

    auto win_size = CCDirector::sharedDirector()->getWinSize();
    float r = m_sprite->getContentSize().width * m_scale * 0.60f;
    m_target.x = std::clamp(tp.x, r, win_size.width - r);
    m_target.y = std::clamp(tp.y, r, win_size.height - r);
}

void OverlayButton::ccTouchEnded(CCTouch*, CCEvent*) {
    m_sprite->stopAllActions();
    m_sprite->runAction(CCEaseBackOut::create(CCScaleTo::create(0.25f, m_scale)));

    float m_minOpacity = Config::get().get<float>("ui_icon.minOpacity", 0.80f);
    m_sprite->runAction(CCSequence::create(
        CCDelayTime::create(0.8f),
        CCFadeTo::create(0.3f, m_minOpacity * 255),
        nullptr
    ));

    if (!m_moved && m_callback) m_callback();
    m_moved = false;
    savePos();
}

void OverlayButton::savePos() {
    Config::get().set<float>("ui_icon.x", m_target.x);
    Config::get().set<float>("ui_icon.y", m_target.y);
}

void OverlayButton::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -1000, true);
}

#ifdef GEODE_IS_MOBILE
#include "hacks_layer.hpp"      

static bool inited = false;
class $modify(CocosInitMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        if (!inited) {
            inited = true;
            auto button = OverlayButton::get();
            button->setID("gdh.toggle.ui"_spr);
            button->setCallback([]() {
                HacksLayer::isOpened() ? HacksLayer::get()->onClose(nullptr) : HacksLayer::get()->show();
            });
            OverlayManager::get()->addChild(button);
        }
        return true;
    }
};

class $modify(RefreshSpriteHook, GameManager) {
    void reloadAll(bool switchingModes, bool toFullscreen, bool borderless, bool fix, bool unused) {
        OverlayManager::get()->removeChildByID("gdh.toggle.ui"_spr);
        OverlayButton::instance = nullptr;
        inited = false;
        GameManager::reloadAll(switchingModes, toFullscreen, borderless, fix, unused);
    }
};
#endif