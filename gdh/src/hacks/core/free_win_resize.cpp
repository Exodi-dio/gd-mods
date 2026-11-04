#ifdef GEODE_IS_WINDOWS

#include <Geode/Geode.hpp>
#include <imgui-cocos.hpp>
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "../../core/windowsUtils.hpp"
#include "../../interface/imgui/widget_helper.hpp"

GUI_HACK_CREATE("Core", "Free Window Resize", "Allows free window resizing", false);

$execute {
    auto& config = Config::get();
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Core").findHackByName("Free Window Resize");   
    hack.setHandler([](bool enabled) {
        static auto patches = []() {
            auto* mod = geode::Mod::get();
            auto base = geode::base::getCocos();
            
            return std::array{
                mod->patch((void*)(base + 0xD93CA), {0x90, 0x90, 0x90, 0x90, 0x90}).unwrapOr(nullptr),
                mod->patch((void*)(base + 0xD7589), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}).unwrapOr(nullptr),
                mod->patch((void*)(base + 0xD8A67), {0x48, 0xe9}).unwrapOr(nullptr)
            };
        }();

        for (auto* patch : patches) {
            if (patch) (void)patch->toggle(enabled);
        }
    });

    hack.setCustomWindowImGui([
        maximizeKey = hack.formatAdditionalSetting("maximaze_window")
    ]{
        ImGuiWidgetConfig::Checkbox("Maximize on Startup", maximizeKey, true);
    });

    geode::queueInMainThread([&config]() {
        if (config.get<bool>("core.free_window_resize", false) &&
            config.get<bool>("core.free_window_resize::maximaze_window", true))
            ShowWindow(GDH::WindowUtils::FindMainHWND(), SW_MAXIMIZE);    
    });
}
#endif