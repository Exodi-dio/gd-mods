#include <Geode/Geode.hpp>
#include "../hacks_tab.hpp"
#include "../../../core/gui.hpp"
#include "../../../core/config.hpp"
#include "../../../core/replayEngine.hpp"
#include "../overlay_button.hpp"
#include "../hacks_layer.hpp"

$execute {
    auto& gui = GDH::Gui::get();
    auto& config = Config::get();
    auto& window = gui.getWindow("Settings");

    window.setCustomWindowCocos([&config, &gui](cocos2d::CCNode* node) {
        auto tab = static_cast<HacksTab*>(node);
        tab->addConfigFloatInput("Icon Size (0.5 - 3)", "ui_icon.scale", 0.5f, 0.25f, 3.f, [](float val) {
            OverlayButton::get()->setSizeScale(val);
        });
        tab->addConfigFloatInput("Min Opacity (0.25 - 1.f)", "ui_icon.minOpacity", 0.8f);
        
        tab->addSeparator();
        tab->prepareNewRow();

        tab->addConfigToggle("Hide icon on game", "ui_icon.hide_on_game", true);
        tab->addConfigToggle("Hide icon on editor", "ui_icon.hide_on_editor", false);

        tab->addSeparator();
        tab->prepareNewRow();

        tab->addConfigToggle("Notify about updates", "ui.notify_updates", true, [](bool enabled) {
            if (!enabled) {
                GDH::MaterialLayer(FLAlertLayer::create("Important", "Even if you turn off update popup, please keep GDH updated\n\nIt's still in beta, so updating helps avoid bugs and issues from older versions :(", "OK"))->show();
            }
        });

        tab->addSeparator();

        tab->addConfigButton("Disable cheating hacks", []() {
            GDH::MaterialLayer(
                geode::createQuickPopup("Confimation", "Are you sure you want to turn off cheating hacks?",
                "No", "Yes",
                [](auto, bool btn2) {
                    if (btn2) {
                        GDH::Gui::get().disableCheats();
                        if (HacksLayer::isOpened()) {
                            HacksLayer::get()->onClose(nullptr);
                        }

                        geode::queueInMainThread([]() {
                            HacksLayer::get()->show();
                        });
                    }
                })
            );
        });

        tab->addConfigButton("Show Cheat Hack List", []() {
            auto& activeCheats = GDH::Gui::get().getActiveCheats();
            bool replay_engine = (GDH::ReplayEngine::get().mode != state::disable);
            
            std::string text = "";
            int index = 0;

            for (const auto& hackID : activeCheats) {
                if (!text.empty()) text += ", ";
                
                std::string color = (index % 2 == 0) ? "<cr>" : "<cy>";
                text += color + hackID + "</c>";
                index++;
            }

            if (replay_engine) {
                if (!text.empty()) text += ", ";
                
                std::string color = (index % 2 == 0) ? "<cr>" : "<cy>";
                text += color + "Replay Engine</c>";
            }

            if (text.empty())
                text = "<cg>No cheat hacks enabled</c>";

            GDH::MaterialLayer(FLAlertLayer::create("Cheat Hack Lists", text.c_str(), "OK"))->show();
        });
    });
}