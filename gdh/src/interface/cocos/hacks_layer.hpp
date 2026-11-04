#pragma once
#include <Geode/Geode.hpp>
#include "hacks_tab.hpp"

class HacksLayer : public geode::Popup {
private:
    bool init();
    static HacksLayer* instance;

    std::vector<HacksTab*> m_tabs;
    std::vector<CCMenuItemSpriteExtra*> m_buttonTabs;
    int m_index = 0;
    int m_lastIndexScroll = -1;

    geode::ScrollLayer* m_tabsScrollLayer = nullptr;

    void switchTab(int newIndex);
public:
    ~HacksLayer();

    static HacksLayer* create();
    static HacksLayer* get();

    static bool isOpened();
    void onClose(CCObject* object);
};