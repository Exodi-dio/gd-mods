#include <Geode/Geode.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"

GUI_HACK_CREATE("Invisible", "Speedhack Audio", "", false);

class $modify(SpeedhackAudioFMODAudioEngine, FMODAudioEngine) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Invisible").findHackByName("Speedhack Audio");        
        hack.setHandler([](bool enabled) {
            if (!enabled) {
                auto audioEngine = FMODAudioEngine::get();
                FMOD::ChannelGroup* group;
                
                if (audioEngine->m_system->getMasterChannelGroup(&group) == FMOD_OK) {
                    group->setPitch(1.f);
                }
            }
        });

        hack.addHookPtr(self.getHook("FMODAudioEngine::update").unwrap());
    }

    void update(float dt) {
        FMODAudioEngine::update(dt);
        
        auto& config = Config::get();
        FMOD::ChannelGroup* group;

        if (m_system->getMasterChannelGroup(&group) == FMOD_OK) {
            bool speedhackEnabled = config.get<bool>("invisible.speedhack", false);
            bool syncWithSpeedhack = config.get<bool>("invisible.speedhack_audio::sync", true);
            const float value = config.get<float>("invisible.speedhack::value", 1.f);

            if (syncWithSpeedhack) {
                group->setPitch(speedhackEnabled ? value : 1.f);
            } else {
                group->setPitch(value);
            }
        }
    }
};