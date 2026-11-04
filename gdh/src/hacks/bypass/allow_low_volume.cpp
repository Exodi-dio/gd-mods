#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Bypass", "Allow Low Volume", "Removes the limit on minimum volume percentage", false);

class $modify(AllowLowVolumePauseLayer, PauseLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Allow Low Volume");        
        
        hack.addHookPtr(self.getHook("PauseLayer::musicSliderChanged").unwrap());
        hack.addHookPtr(self.getHook("PauseLayer::sfxSliderChanged").unwrap());
    }

    void musicSliderChanged(cocos2d::CCObject* sender) {
        PauseLayer::musicSliderChanged(sender);
        
        auto value = geode::cast::typeinfo_cast<SliderThumb*>(sender)->getValue();
        auto* audio_engine = FMODAudioEngine::sharedEngine();
        if (value < 0.03f)
            audio_engine->setBackgroundMusicVolume(value);
    }
    
    void sfxSliderChanged(cocos2d::CCObject* sender) {
        PauseLayer::sfxSliderChanged(sender);
        
        auto value = geode::cast::typeinfo_cast<SliderThumb*>(sender)->getValue();
        auto* audio_engine = FMODAudioEngine::sharedEngine();
        if (value < 0.03f)
            audio_engine->setEffectsVolume(value);
    }
};

class $modify(AllowLowVolumeOptionsLayer, OptionsLayer) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Allow Low Volume");        
        
        hack.addHookPtr(self.getHook("OptionsLayer::musicSliderChanged").unwrap());
        hack.addHookPtr(self.getHook("OptionsLayer::sfxSliderChanged").unwrap());
    }

    void musicSliderChanged(cocos2d::CCObject* sender) {
        OptionsLayer::musicSliderChanged(sender);
        
        auto value = geode::cast::typeinfo_cast<SliderThumb*>(sender)->getValue();
        auto* audio_engine = FMODAudioEngine::sharedEngine();
        if (value < 0.03f)
            audio_engine->setBackgroundMusicVolume(value);
    }

    void sfxSliderChanged(cocos2d::CCObject* sender) {
        OptionsLayer::sfxSliderChanged(sender);

        auto value = geode::cast::typeinfo_cast<SliderThumb*>(sender)->getValue();
        auto* audio_engine = FMODAudioEngine::sharedEngine();
        if (value < 0.03f)
            audio_engine->setEffectsVolume(value);
    }
};