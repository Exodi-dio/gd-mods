#ifdef GEODE_IS_DESKTOP
#include "imgui.h"
#include <Geode/Geode.hpp>
#include "../../../core/gui.hpp"
#include "../../../core/config.hpp"
#include "../../../core/utils.hpp"
#include "../widget_helper.hpp"
#include "../layout.hpp"
#include "../widgetH.hpp"
#include <imgui-cocos.hpp>

$execute {
    auto& gui = GDH::Gui::get();
    auto& config = Config::get();
    auto& layout = GDH::Layout::Manager::get();
    auto& window = gui.getWindow("Framerate");

    window.setCustomWindowImGui([&config, &gui, &layout] {
        static bool showTpsWarning = false;
        static bool hasShownThisSession = false;

        // ImGuiWidgetConfig::DrawValueToggle("fps_value", 1, 1, FLT_MAX, 60.f, "%0.f FPS");
        // ImGuiWidgetConfig::DrawValueToggle("tps_value", 1, 1, FLT_MAX, 240.f, "%0.f TPS");

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - layout.multipleScale(52.f));
        if (ImGuiWidgetConfig::DragFloat("##FPS_Value", "invisible.fps::value", 1, 1, FLT_MAX, 60.f, "%.2f FPS")) {
            auto& hack = gui.getWindow("Invisible").findHackByName("FPS");  
            if (hack.getEnabled()) {
                hack.disable();
                hack.enable();
            }
        }
        ImGui::SameLine();
        ImGuiWidgetConfig::HackCheckbox("##FPS_Enabled", "invisible.fps", false);

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - layout.multipleScale(52.f));
        if (ImGuiWidgetConfig::DragFloat("##TPS_Value", "invisible.tps::value", 1, 1, FLT_MAX, 240.f, "%.2f TPS")) {
            auto tpsValue = config.get<float>("invisible.tps::value", 240.f);
            if (tpsValue < 1.f) {
                tpsValue = 1.f;
                config.set<float>("invisible.tps::value", 1.f);
            }

            bool dontShowSaved = config.get<bool>("invisible.tps_warning_dont_show_again", false);
            
            if (tpsValue < 240.f && !dontShowSaved && !hasShownThisSession) {
                showTpsWarning = true;
                hasShownThisSession = true;
            }

            if (config.get<bool>("invisible.lock_delta::sync_tps", true)) {
                config.set<float>("invisible.lock_delta::value", tpsValue);
            }

            auto& hack = gui.getWindow("Invisible").findHackByName("TPS");  
            if (hack.getEnabled()) {
                hack.disable();
                hack.enable();
            }
        }
        ImGui::SameLine();
        ImGuiWidgetConfig::HackCheckbox("##TPS_Enabled", "invisible.tps", false);

        if (showTpsWarning) {
            ImGui::OpenPopup("TPS Warning##Modal");
            showTpsWarning = false;
        }

        if (ImGui::BeginPopupModal("TPS Warning##Modal", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            auto glow_in = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Critical_In]);
            auto glow_out = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Critical_Out]);
            ImGuiH::GlowWindow(glow_in, glow_out, layout.multipleScale(500.f));

            float oldScale = ImGui::GetFont()->Scale;
            ImGui::GetFont()->Scale = layout.multipleScale(32.0f) / ImGui::GetFontSize();
            ImGui::PushFont(ImGui::GetFont());

            ImGui::TextColored(ImColor(255, 128, 128).Value, "Warning: TPS below 240 breaks physics!");
            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Text("• Geometry Dash physics requires at least 240 TPS to remain stable");
            ImGui::Text("  (RobTop never designed the engine physics for native TPS changes)");
            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Text("• If you need lower visual frame rates: Use 'Lock Delta' or 'FPS Bypass'");
            ImGui::Text("  This lowers visual FPS without breaking game physics");

            ImGui::GetFont()->Scale = oldScale;
            ImGui::PopFont();

            ImGuiH::SpaceSeparator();

            static bool dontShowAgain = false;
            ImGuiH::Checkbox("Don't show this warning again", &dontShowAgain);

            if (ImGuiH::Button("OK", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                if (dontShowAgain) {
                    config.set<bool>("invisible.tps_warning_dont_show_again", true);
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        #if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_ANDROID64) || defined(GEODE_IS_INTEL_MAC)

        ImGuiH::SpaceSeparator();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGuiWidgetConfig::DragFloat("##LockDelta_Value", "invisible.lock_delta::value", 1, 1.f, FLT_MAX, 240.f, "LockDT: %.2f FPS")) {
            if (config.get<bool>("invisible.lock_delta::sync_tps", true)) {
                auto value = config.get<float>("invisible.lock_delta::value", 240.f);
                config.set<float>("invisible.tps::value", value);
            }
        }
        ImGuiWidgetConfig::HackCheckbox("Lock Delta Enabled", "invisible.lock_delta", false);
        ImGuiH::SpaceSeparator();

        #endif
        
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - layout.multipleScale(52.f));
        if (ImGuiWidgetConfig::DragFloat("##Speedhack_Value", "invisible.speedhack::value", 0.01f, 0, 500.f, 1.f, "Speed: %.2fx"))
            gui.rescanActiveCheats();

        ImGui::SameLine();
        if (ImGuiWidgetConfig::HackCheckbox("##Speedhack_Enabled", "invisible.speedhack", false)) {
            gui.rescanActiveCheats();
        }

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - layout.multipleScale(52.f));
        if (ImGuiWidgetConfig::DragInt("##Pitch_Shifter_Value", "invisible.pitch_shifter::value", 1.f, -24, 24, 0, "Pitch: %d ST")) {
            bool enabled = config.get<bool>("invisible.pitch_shifter", false);
            GDH::Utils::setPitchShifter(enabled ? config.get<int>("invisible.pitch_shifter::value", 0) : 0);
        }
        ImGui::SameLine();
        ImGuiWidgetConfig::HackCheckbox("##Pitch_Shifter_Enabled", "invisible.pitch_shifter", false);
        
        ImGuiWidgetConfig::HackCheckbox("Speedhack Audio", "invisible.speedhack_audio", false);
        ImGuiWidgetConfig::Checkbox("Sync With Speedhack", "invisible.speedhack_audio::sync", true);

        ImGuiH::SpaceSeparator();

        ImGuiWidgetConfig::HackCheckbox("Vertical Sync", "invisible.vertical_sync", false);

        ImGuiWidgetConfig::Checkbox("Real Time (Lock Delta)", "invisible.lock_delta::real_time", true);

        #if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_ANDROID64) || defined(GEODE_IS_INTEL_MAC)
        if (ImGuiWidgetConfig::Checkbox("Sync TPS with Lock DT", "invisible.lock_delta::sync_tps", true)) {
            if (config.get<bool>("invisible.lock_delta::sync_tps", true)) {
                auto value = config.get<float>("invisible.tps::value", 240.f);
                config.set<float>("invisible.lock_delta::value", value);
            }
        }
        #endif
    });
}
#endif