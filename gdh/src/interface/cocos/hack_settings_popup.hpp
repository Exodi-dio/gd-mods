#pragma once
#include <Geode/Geode.hpp>
#include "../../core/hacks.hpp"

class HackSettingsPopup : public geode::Popup {
protected:
    GDH::Hack* m_hack;
    bool init(GDH::Hack& hack);
public:
    geode::prelude::ScrollLayer* m_scrollLayer;
    cocos2d::CCMenu* m_currentRow = nullptr;

    static HackSettingsPopup* create(GDH::Hack& hack);

    void prepareNewRow();

    void addConfigToggle(const std::string& labelText, const std::string& key, bool defaultValue = false, geode::Function<void(bool)> callback = nullptr);
    void addConfigIntInput(const std::string& labelText, const std::string& key, int min, int max, int defaultValue = 0, geode::Function<void(int)> callback = nullptr);
    void addConfigFloatInput(const std::string& labelText, const std::string& key, float min, float max, float defaultValue = 0.f, geode::Function<void(float)> callback = nullptr);
    void addConfigColor3Hex(const std::string& labelText, const std::string& key, const std::string& defaultHex);
    void addConfigColor4Hex(const std::string& labelText, const std::string& key, const std::string& defaultHex);
    void addSeparator(float height = 1.f);
};

class CompatibleColorPopup : public geode::ColorPickPopup {
public:
    static CompatibleColorPopup* create(cocos2d::ccColor4B const& color, bool isRGBA) {
        auto ret = new CompatibleColorPopup();
        if (ret->init(color, isRGBA)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};