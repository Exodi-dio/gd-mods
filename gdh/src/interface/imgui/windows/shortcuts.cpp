#ifdef GEODE_IS_DESKTOP
#include "imgui.h"
#include <Geode/Geode.hpp>
#include "../../../core/gui.hpp"
#include "../layout.hpp"
#include "../../../core/config.hpp"
#include "../../../core/shortcuts.hpp"
#include "../../../core/keybinds.hpp"
#include "../../../core/replayEngine.hpp"
#include "../widgetH.hpp"
#include <imgui-cocos.hpp>
#include "../../cocos/hacks_layer.hpp"

static bool areShortcutsEnabled() {
    return Config::get().get<bool>("shortcuts::enable", false);
}

static bool areFramerateEnabled() {
    return Config::get().get<bool>("framerate::enable", false);
}

static bool areReplayEngineEnabled() {
    return Config::get().get<bool>("keybinds::engine", false);
}

$execute {
    auto& kb = GDH::Keybinds::get();

    kb.addCallback("shortcut::options", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::openOptions(); });
    kb.addCallback("shortcut::graphics", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::openGraphicsSettings(); });
    kb.addCallback("shortcut::quests", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::openQuests(); });
    kb.addCallback("shortcut::mobile_ui", geode::Keybind(), [](bool r) { 
        if (!r && areShortcutsEnabled()) HacksLayer::isOpened() ? HacksLayer::get()->onClose(nullptr) : HacksLayer::get()->show(); 
    });
    kb.addCallback("shortcut::reset_level", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::resetLevel(); });
    kb.addCallback("shortcut::practice_mode", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::togglePracticeMode(); });
    kb.addCallback("shortcut::reset_volume", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::resetVolume(); });
    kb.addCallback("shortcut::uncomplete_level", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::uncompleteLevel(); });
    kb.addCallback("shortcut::resources", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::openResourcesFolder(); });
    kb.addCallback("shortcut::appdata", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::openAppDataFolder(); });
    kb.addCallback("shortcut::gdh_appdata", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::openGDHAppDataFolder(); });
    kb.addCallback("shortcut::crash_game", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::triggerCrash(); });
    kb.addCallback("shortcut::disable_cheat_hacks", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Gui::get().disableCheats(); });
    kb.addCallback("shortcut::restart_game", geode::Keybind(), [](bool r) { if (!r && areShortcutsEnabled()) GDH::Shortcuts::restart(); });

    kb.addCallback("keybinds::speedhack", geode::Keybind(), [](bool r) { if (!r && areFramerateEnabled()) {
        if (r) return;
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Invisible").findHackByName("Speedhack");
        hack.toggle();
    } });

    kb.addCallback("keybinds::speedhack_inc", geode::Keybind(), [](bool r) {
        if (r || !areFramerateEnabled()) return;

        auto& config = Config::get();
        auto value = config.get<float>("invisible.speedhack::value", 1.f);

        float newValue = std::max(0.05f, std::round((value + 0.05f) * 20.f) / 20.f);
        config.set<float>("invisible.speedhack::value", newValue);

        ImGuiH::AddPopup(fmt::format("Speedhack: {:.2f}x", newValue));
    });

    kb.addCallback("keybinds::speedhack_dec", geode::Keybind(), [](bool r) {
        if (r || !areFramerateEnabled()) return;

        auto& config = Config::get();
        auto value = config.get<float>("invisible.speedhack::value", 1.f);

        float newValue = std::max(0.05f, std::round((value - 0.05f) * 20.f) / 20.f);
        config.set<float>("invisible.speedhack::value", newValue);

        ImGuiH::AddPopup(fmt::format("Speedhack: {:.2f}x", newValue));
    });

    kb.addCallback("re::record", geode::Keybind(), [](bool r) {
        if (r || !areReplayEngineEnabled()) return;

        auto& engine = GDH::ReplayEngine::get();
        engine.mode = (engine.mode == state::record) ? state::disable : state::record;
        ImGuiH::AddPopup(fmt::format("Replay Engine: {}", (engine.mode == state::record) ? "Record" : "Disable"));
    });

    kb.addCallback("re::playback", geode::Keybind(), [](bool r) {
        if (r || !areReplayEngineEnabled()) return;

        auto& engine = GDH::ReplayEngine::get();
        engine.mode = (engine.mode == state::play) ? state::disable : state::play;
        ImGuiH::AddPopup(fmt::format("Replay Engine: {}", (engine.mode == state::play) ? "Play" : "Disable"));
    });

    kb.addCallback("re::save_macro_by_name", geode::Keybind(), [](bool r) {
        if (r || !areReplayEngineEnabled()) return;

        auto pl = PlayLayer::get();
        if (!pl) {
            ImGuiH::AddPopup("Can't get a level name to save a macro");
        }
        else {
            std::string level_name = pl->m_level->m_levelName;

            auto& engine = GDH::ReplayEngine::get();
            ImGuiH::AddPopup(engine.save(level_name));
        }
    });

    kb.addCallback("re::load_macro_by_name", geode::Keybind(), [](bool r) {
        if (r || !areReplayEngineEnabled()) return;

        auto pl = PlayLayer::get();
        if (!pl) {
            ImGuiH::AddPopup("Can't get a level name to load a macro");
        }
        else {
            std::string level_name = pl->m_level->m_levelName;

            auto& engine = GDH::ReplayEngine::get();
            ImGuiH::AddPopup(engine.load(level_name));
        }
    });
    
    auto& gui = GDH::Gui::get();
    auto& window = gui.getWindow("Shortcuts");

    window.setCustomWindowImGui([] {
        auto& layout = GDH::Layout::Manager::get();
        auto& kb = GDH::Keybinds::get();
        float width = ImGui::GetContentRegionAvail().x;

        if (ImGuiH::Button("Options", {width, 0.f}))
            GDH::Shortcuts::openOptions();

        if (ImGuiH::Button("Graphics Settings", {width, 0.f}))
            GDH::Shortcuts::openGraphicsSettings();

        if (ImGuiH::Button("Quests Page", {width, 0.f}))
            GDH::Shortcuts::openQuests();

        if (ImGuiH::Button("Open Mobile UI", {width, 0.f}))
            HacksLayer::isOpened() ? HacksLayer::get()->onClose(nullptr) : HacksLayer::get()->show();

        if (ImGuiH::Button("Reset Level", {width, 0.f})) 
            GDH::Shortcuts::resetLevel();

        if (ImGuiH::Button("Practice Mode", {width, 0.f}))
            GDH::Shortcuts::togglePracticeMode();

        if (ImGuiH::Button("Reset Volume", {width, 0.f}))
            GDH::Shortcuts::resetVolume();

        if (ImGuiH::Button("Uncomplete Level", {width, 0.f}))
            GDH::Shortcuts::uncompleteLevel();

        if (ImGuiH::Button("Resources", {width/2, 0.f})) {
            GDH::Shortcuts::openResourcesFolder();
        }
        ImGui::SameLine();
        if (ImGuiH::Button("AppData", {ImGui::GetContentRegionAvail().x, 0.f})) {
            GDH::Shortcuts::openAppDataFolder();
        }

        if (ImGuiH::Button("GDH AppData", {width, 0.f})) {
            GDH::Shortcuts::openGDHAppDataFolder();
        }

        if (ImGuiH::Button("Crash Game", {width, 0.f})) {
            ImGui::OpenPopup("Crash the Game");
        }
        
        if (ImGui::BeginPopupModal("Crash the Game", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
            auto glow_in = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Critical_In]);
            auto glow_out = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Critical_Out]);

            ImGuiH::GlowWindow(glow_in, glow_out, layout.multipleScale(500.f));
            ImGui::Text("Are you sure you want to crash Geometry Dash?");
            ImGui::Text("Unsaved data will be lost");

            if (ImGuiH::Button("Cancel", {ImGui::GetContentRegionAvail().x, 0})) {
                ImGui::CloseCurrentPopup();
            }

            if (ImGuiH::Button("OK", {ImGui::GetContentRegionAvail().x, 0})) {
                GDH::Shortcuts::triggerCrash();
            }

            ImGui::EndPopup();
        }

        if (ImGuiH::Button("Restart Game", {width, 0.f})) {
            GDH::Shortcuts::restart();
        }
    });
}
#endif