#include <Geode/Geode.hpp>
#include <Geode/binding/AppDelegate.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"

$execute {
    auto& config = Config::get();
    auto& gui = GDH::Gui::get();

    auto& hack = gui.getWindow("Level").findHackByName("Click Between Steps");   
    hack.setGameVariableID(GameVar::ClickBetweenSteps);
    hack.setDesc("Toggles substeps on, allowing you to click in-between the default normal steps. This may affect performance");
    hack.setEarlyInit(false);
    hack.setHandler([](bool enabled) {
        if (auto gjbgl = GJBaseGameLayer::get()) {
            gjbgl->m_clickBetweenSteps = enabled;
        }
    });
    
    auto& hack2 = gui.getWindow("Level").findHackByName("Click On Steps");   
    hack2.setGameVariableID(GameVar::ClickOnSteps);
    hack2.setDesc("Allows you to click on every step, regardless of your refresh rate. This is beneficial for users below 240 Hz");
    hack2.setEarlyInit(false);
    hack2.setHandler([](bool enabled) {
        if (auto gjbgl = GJBaseGameLayer::get()) {
            gjbgl->m_clickOnSteps = enabled;
        }
    });

    auto& hack3 = gui.getWindow("Level").findHackByName("Click Between Frames");
    hack3.setDesc("Enables the in-game \"Click Between Steps\" setting");
    hack3.setEarlyInit(false);
    hack3.setHandler([](bool enabled) {
        auto cbf = geode::Loader::get()->getLoadedMod("syzzi.click_between_frames");
        if (!cbf) return;

        cbf->setSettingValue<bool>("soft-toggle", !enabled);
    });
}

$on_game(ModsLoaded) {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Level").findHackByName("Click Between Frames");

    auto cbf = geode::Loader::get()->getLoadedMod("syzzi.click_between_frames");
    if (!cbf) {
        hack.setDisabled(true);
        return;
    }

    hack.setEnabled(!cbf->getSettingValue<bool>("soft-toggle"));
}