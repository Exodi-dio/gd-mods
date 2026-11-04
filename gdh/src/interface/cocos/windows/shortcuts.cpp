#include <Geode/Geode.hpp>
#include "../hacks_tab.hpp"
#include "../../../core/gui.hpp"
#include "../../../core/shortcuts.hpp"

$execute {
    auto& gui = GDH::Gui::get();
    auto& window = gui.getWindow("Shortcuts");

    window.setCustomWindowCocos([&gui](cocos2d::CCNode* node) {
        auto tab = static_cast<HacksTab*>(node);
        
        tab->addConfigButton("Options", []() {
            GDH::Shortcuts::openOptions();
        });

        tab->addConfigButton("Graphics Settings", []() {
            GDH::Shortcuts::openGraphicsSettings();
        },
        "Quests Page", []() {
            GDH::Shortcuts::openQuests();
        });
        
        tab->addConfigButton("Reset Level", []() {
            GDH::Shortcuts::resetLevel();
        });

        tab->addConfigButton("Practice Mode", []() {
            GDH::Shortcuts::togglePracticeMode();
        });

        tab->addConfigButton("Reset Volume", []() {
            GDH::Shortcuts::resetVolume();
        });

        tab->addConfigButton("Uncomplete Level", []() {
            GDH::Shortcuts::uncompleteLevel();
        });

        tab->addConfigButton(
            "AppData", []() {
                GDH::Shortcuts::openAppDataFolder();
            },
            "GDH AppData", []() {
                GDH::Shortcuts::openGDHAppDataFolder();
            }
        );

        tab->addConfigButton("Crash Game", []() {
            GDH::MaterialLayer(
                geode::createQuickPopup("Crash Game", "Are you sure you want to crash Geometry Dash?\n<cr>Unsaved data will be lost</c>",
                "No", "Yes",
                [](auto, bool btn2) {
                    if (btn2) {
                       GDH::Shortcuts::triggerCrash();
                    }
                })
            );
        });
    });
}