#pragma once
#include <Geode/Geode.hpp>
#include "../../core/hacks.hpp"

class HacksTab : public cocos2d::CCMenu {
public:
    static HacksTab* create();
    void addToggle(GDH::Hack& hck);
    
    void addHackToggle(const std::string& label, const std::string& key, bool defaultValue = false, geode::Function<void(bool)> callback = nullptr);
    void addConfigToggle(const std::string& label, const std::string& key, bool defaultValue = false, geode::Function<void(bool)> callback = nullptr);
    void addConfigIntInput(const std::string& label, const std::string& key, int defaultValue = 0, int min = 0, int max = 100, geode::Function<void(int)> callback = nullptr);
    void addConfigFloatInput(const std::string& label, const std::string& key, float defaultValue = 0.f, float min = 0.f, float max = 100.f, geode::Function<void(float)> callback = nullptr);
    void addConfigButton(const std::string& labelText, geode::Function<void()> callback, const std::string& secondLabelText = "", geode::Function<void()> secondCallback = nullptr);
    geode::Label* AddTextToToggle(const char *str, CCMenuItemToggler* toggler, float x_space = 22.f);

    void addText(const std::string& text, float scale = 0.5f);
    void prepareNewRow();

    geode::prelude::ScrollLayer* m_scrollLayer;
    cocos2d::CCMenu* m_currentRow = nullptr;

    void addPadding(float height = 0.f);
    void addSeparator(float height = 1.f);
private:
    HacksTab() = default;
    bool init();
};