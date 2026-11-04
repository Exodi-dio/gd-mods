#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_ANDROID64)
#include <Geode/Geode.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Creator", "Editor Extension", "Increases the editor length by a factor of 128", false);

$execute {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Creator").findHackByName("Editor Extension");   

    hack.setHandler([](bool enabled) {
        static auto patches = []() {
            auto* mod = geode::Mod::get();
            auto base = geode::base::get();
            
            #if defined(GEODE_IS_WINDOWS)
            return std::array{
                mod->patch((void*)(base + 0x623670), {0x00, 0x60, 0xEA, 0x4B}).unwrapOr(nullptr), // 00 60 6A 48
                mod->patch((void*)(base + 0x623674), {0x00, 0x60, 0xEA, 0x4B}).unwrapOr(nullptr), // 80 67 6A 48
                mod->patch((void*)(base + 0x623B04), {0xFF, 0xFF, 0x7F, 0xFF}).unwrapOr(nullptr), // F0 23 74 C9
                mod->patch((void*)(base + 0x62368C), {0xFF, 0xFF, 0x7F, 0x7F}).unwrapOr(nullptr)  // F0 23 74 49
            };
            #elif defined(GEODE_IS_ANDROID64) 
            return std::array{
                mod->patch((void*)(base + 0x667224), {0x00, 0x60, 0xEA, 0x4B}).unwrapOr(nullptr), // 00 60 6A 48
                mod->patch((void*)(base + 0x68EF18), {0x00, 0x60, 0xEA, 0x4B}).unwrapOr(nullptr), // 00 60 6A 48
                mod->patch((void*)(base + 0x6C04BC), {0x00, 0x60, 0xEA, 0x4B}).unwrapOr(nullptr), // 00 60 6A 48
                
                mod->patch((void*)(base + 0x68f460), {0x00, 0x60, 0xEA, 0x4B}).unwrapOr(nullptr), // 80 67 6A 48
                mod->patch((void*)(base + 0x68fb8c), {0xFF, 0xFF, 0x7F, 0xFF}).unwrapOr(nullptr), // F0 23 74 C9
                mod->patch((void*)(base + 0x68fb88), {0xFF, 0xFF, 0x7F, 0x7F}).unwrapOr(nullptr)  // F0 23 74 49
            };
            #endif
        }();

        for (auto* patch : patches) {
            if (patch) (void)patch->toggle(enabled);
        }
    });
}
#endif