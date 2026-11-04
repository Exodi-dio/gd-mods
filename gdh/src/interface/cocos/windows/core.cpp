#include <Geode/Geode.hpp>
#include "../hacks_tab.hpp"
#include "../../../core/gui.hpp"
#include "../../../core/config.hpp"
#include "../../../core/utils.hpp"

$execute {
    auto& gui = GDH::Gui::get();
    auto& config = Config::get();
    auto& window = gui.getWindow("Core");

    window.setCustomWindowCocos([&config, &gui](cocos2d::CCNode* node) {
        auto tab = static_cast<HacksTab*>(node);
        
        tab->addSeparator();
        tab->prepareNewRow();

        tab->addConfigFloatInput("TPS Value", "invisible.tps::value", 240.f, 1.f, FLT_MAX, [&gui](float value) {
            auto& hack = gui.getWindow("Invisible").findHackByName("TPS");  
            if (hack.getEnabled()) {
                hack.disable();
                hack.enable();
            }
        });
        tab->addHackToggle("TPS Enabled", "invisible.tps");
        
        tab->addSeparator();
        tab->prepareNewRow();

        #if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_ANDROID64) || defined(GEODE_IS_INTEL_MAC)
        
        tab->addConfigFloatInput("Lock Delta Value", "invisible.lock_delta::value", 240.f, 1.f, FLT_MAX, [&config](float value) {
            if (config.get<bool>("invisible.lock_delta::sync_tps", true)) {
                auto value = config.get<float>("invisible.lock_delta::value", 240.f);
                config.set<float>("invisible.tps::value", value);
            }
        });
        tab->addHackToggle("Lock Delta Enabled", "invisible.lock_delta");

        tab->addConfigToggle("Real Time (Lock Delta)", "invisible.lock_delta::real_time", true);
        tab->addConfigToggle("Sync TPS w/Lock DT ", "invisible.lock_delta::sync_tps", true, [&config](bool enabled) {
            if (config.get<bool>("invisible.lock_delta::sync_tps", true)) {
                auto value = config.get<float>("invisible.tps::value", 240.f);
                config.set<float>("invisible.lock_delta::value", value);
            }
        });

        tab->addSeparator();
        tab->prepareNewRow();
        #else

        tab->addConfigToggle("Real Time (Lock Delta)", "invisible.lock_delta::real_time", true);

        tab->addSeparator();
        tab->prepareNewRow();

        #endif

        tab->addConfigFloatInput("Speedhack Value", "invisible.speedhack::value", 1.f, 0.01f, 50.f, [&gui](float) {
            gui.rescanActiveCheats();
        });
        tab->addHackToggle("Speedhack Enabled", "invisible.speedhack", false, [&gui](bool) {
            gui.rescanActiveCheats();
        });
        tab->addHackToggle("Speedhack Audio", "invisible.speedhack_audio");

        tab->addSeparator();
        tab->prepareNewRow();

        tab->addConfigIntInput("Pitch Shifter (-24 to 24)", "invisible.pitch_shifter::value", 1.f, -12, 12, [&config](int value) {
            bool enabled = config.get<bool>("invisible.pitch_shifter", false);
            GDH::Utils::setPitchShifter(enabled ? config.get<int>("invisible.pitch_shifter::value", 0) : 0);
        });
        tab->addHackToggle("Pitch Shifter Enabled", "invisible.pitch_shifter");
    });
}