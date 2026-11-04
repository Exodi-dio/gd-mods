#ifdef GEODE_IS_DESKTOP
#include "imgui.h"
#include <imgui_internal.h>
#include <imgui-cocos.hpp>
#include <imgui_stdlib.h>
#include "../../../core/gui.hpp"
#include "../../../core/config.hpp"
#include "../../../core/labels.hpp"
#include "../widget_helper.hpp"
#include "../widgetH.hpp"
#include "../layout.hpp"

GDH::Labels::Corner g_editCorner = GDH::Labels::Corner::Top_Left;
GDH::Labels::Corner g_popupCorner;
int g_popupId = 0;
bool g_openPopup = false;
bool g_editPopup = false;
bool g_updateSize = false;

float g_newLabelSize = 0.35f;
std::array<float, 4> g_newLabelColor = { 1.0f, 1.0f, 1.0f, 0.30f };

const std::vector<std::pair<const char*, const char*>> g_labelTemplates = {
    {"Attempts", "Attempt {attempt}"},
    {"Best Run", "Best Run: {best_run}"},
    {"Best Time", "Best Time: {best_time}"},
    {"CPS Counter", "{cps}/{cps_high}/{clicks}"},
    {"CPS Counter (P1)", "{p1::cps}/{p1::cps_high}/{p1::clicks}"},
    {"CPS Counter (P2)", "{p2::cps}/{p2::cps_high}/{p2::clicks}"},
    {"Custom text", "Edit me!"},
    {"Date", "{date}"},
    {"Death Counter", "{deaths} Deaths"},
    {"Dual Mode", "Dual: {is_dual_mode}"},
    {"FPS", "{fps} FPS"},
    {"Frame", "{frame} Frame"},
    {"Level Info", "{level_name} by {level_creator}"},
    {"Level ID", "{level_id}"},
    {"Level progress", "{progress:2}"},
    {"Noclip Accuracy (%)", "{noclip_accuracy}%"},
    {"Normal Percent", "{normal_percent}%"},
    {"Player 1 Information", "{p1::x}{\\n}{p1::y}{\\n}{p1::y_vel}{\\n}P1Flip: {p1::is_upside_down}"},
    {"Player 2 Information", "{p2::x}{\\n}{p2::y}{\\n}{p2::y_vel}{\\n}P2Flip: {p2::is_upside_down}"},
    {"Practice Percent", "{practice_percent}%"},
    {"Rainbow Text", "Rainbow Text!!"},
    {"Replay Engine State", "{re_state}"},
    {"Session time", "{session_time}"},
    {"Testmode", "{testmode}"},
    {"Time (12h)", "{time_12}"},
    {"Time (24h)", "{time_24}"}
};

void drawLabelCornerControls(GDH::Labels::Corner corner) {  
    if (g_updateSize) {
        g_updateSize = false;
        ImGui::SetWindowSize({ImGui::GetWindowSize().x, 0});
    } 

    ImGui::PushID(static_cast<int>(corner));
    ImGui::Spacing();
    
    if (ImGuiH::Button("Add a label")) {
        g_popupCorner = corner;
        g_openPopup = true;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("?");
    ImGuiH::Tooltip("Labels are shown top-to-bottom in their corners as added here.\n"
                    "Text labels support variables that get substituted to different things like current date, level progress, etc.\n"
                    "Spacing labels add pixel spacing between text labels.\n"
                    "'Add a label' button adds a label with a pre-set text, there can be any number of any type of labels.\n"
                    "Every label has its own color and size.\n"
                    "A single text label may contain any number of variables. Add a few examples and change them around to get the feel for it.", ImGui::IsItemHovered());
    
    auto& manager = GDH::Labels::Manager::get();
    auto& currentLabels = manager.labels[corner];

    if (currentLabels.empty()) {
        ImGui::Spacing();
        ImGui::TextDisabled("No labels in this corner");
    } else {
        int id = 0;
        for (auto &label : currentLabels) {
            ImGui::PushID(id++);
            if (ImGuiH::Button("E")) {
                g_editPopup = true;
                g_popupCorner = corner;
                g_popupId = id - 1;
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            
            if (!label.enabled) ImGui::BeginDisabled(true);
            if (label.type == GDH::Labels::LabelType::Text) {
                ImGui::InputText("##TextInput", &label.text);
            } else if (label.type == GDH::Labels::LabelType::CheatIndicator) {
                ImGuiH::DragFloat("##CISize", &label.size, 1.0f, 1.0f, 256.0f, "Indicator Size: %.1fpx");
            } else if (label.type == GDH::Labels::LabelType::Spacing) {
                ImGuiH::DragFloat("##Spacing", &label.size, 1.0f, 1.0f, 256.0f, "Spacing: %.1fpx");
            }
            if (!label.enabled) ImGui::EndDisabled();
            
            ImGui::PopID();
        }
    }
    
    ImGui::PopID();
}

$execute {
    auto& gui = GDH::Gui::get();
    auto& config = Config::get();
    auto& window = gui.getWindow("Labels");
    auto& layout = GDH::Layout::Manager::get();

    window.setCustomWindowImGui([&config, &gui, &layout] {
        auto& manager = GDH::Labels::Manager::get();
        ImGuiWidgetConfig::Checkbox("Disable All", "labels::disable_all", false);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGuiH::DragFloat("##LabelCorPadDrag", &manager.cornerPadding, 1.0f, 0.0f, 256.0f, "Corner Padding: %.1fpx");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGuiH::DragFloat("##LabelMidPadDrag", &manager.midPadding, 1.0f, 0.0f, 256.0f, "Mid-label Padding: %.1fpx");

        static const char* cornerNames[] = {
            "Top Left", "Top Center", "Top Right",
            "Center Left", "Center Center", "Center Right",
            "Bottom Left", "Bottom Center", "Bottom Right"
        };

        int currentCorner = static_cast<int>(g_editCorner);
        if (ImGui::Combo("Corner", &currentCorner, cornerNames, IM_ARRAYSIZE(cornerNames))) {
            g_editCorner = static_cast<GDH::Labels::Corner>(currentCorner);
        }

        ImGui::Separator();
        drawLabelCornerControls(g_editCorner);
        
        if (g_openPopup) {
            ImGui::OpenPopup("Add a label##Modal");
            g_openPopup = false;
        }

        ImVec4 bgColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        bgColor.w = 1.0f;
        ImGui::PushStyleColor(ImGuiCol_WindowBg, bgColor);
        
        if (ImGui::BeginPopupModal("Add a label##Modal", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
            auto glow_in = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_In]);
            auto glow_out = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Out]);
            ImGuiH::GlowWindow(glow_in, glow_out);
            
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit4("##Color", g_newLabelColor.data());
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGuiH::DragFloat("##Size", &g_newLabelSize, 0.1f, 0.1f, 4.0f, "Size: %.1f");
            
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            for (const auto &[name, value] : g_labelTemplates) {
                if (ImGuiH::Button(name, {layout.multipleScale(290.f), 0})) {
                    std::string_view nameView(name);
                    bool isRainbowButton = (nameView == "Rainbow Text");
                    bool isCPSButton = nameView.contains("CPS Counter");
                    bool isNoclip = (nameView == "Death Counter" || nameView == "Noclip Accuracy (%)");

                    if (isRainbowButton) g_newLabelColor[3] = 0.85f;
                    
                    manager.labels[g_popupCorner].push_back(
                        GDH::Labels::Label(value, g_newLabelColor, g_newLabelSize, isRainbowButton, isCPSButton, isNoclip)
                    );

                    if (isRainbowButton) g_newLabelColor[3] = 0.30f;

                    g_updateSize = true;
                    ImGui::CloseCurrentPopup();
                }
            }

            if (ImGuiH::Button("Cheat Indicator", {layout.multipleScale(290.f), 0.f})) {
                manager.labels[g_popupCorner].push_back(GDH::Labels::Label(1.f, GDH::Labels::LabelType::CheatIndicator));

                auto& hack = gui.getWindow("Level").findHackByName("Cheat Indicator");
                if (!hack.getEnabled()) hack.enable();

                g_updateSize = true;
                ImGui::CloseCurrentPopup();
            }

            if (ImGuiH::Button("Spacing", {layout.multipleScale(290.f), 0.f})) {
                manager.labels[g_popupCorner].push_back(GDH::Labels::Label(g_newLabelSize));
                g_updateSize = true;
                ImGui::CloseCurrentPopup();
            }

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            if (ImGuiH::Button("Close", {layout.multipleScale(290.f), 0.f})) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor();

        if (g_editPopup) {
            ImGui::OpenPopup("Editing the label");
            g_editPopup = false;
        }

        if (ImGui::BeginPopup("Editing the label")) {
            auto& targetLabels = manager.labels[g_popupCorner];
            auto& currentLabel = targetLabels[g_popupId];

            ImGuiH::Checkbox("Enabled", &currentLabel.enabled);
            if (!currentLabel.enabled) ImGui::BeginDisabled(true);
            
            const char* typeName = (currentLabel.type == GDH::Labels::LabelType::Text) ? "Text" :
                                (currentLabel.type == GDH::Labels::LabelType::CheatIndicator) ? "Cheat Indicator" : "Spacing";
                
            if (ImGui::BeginCombo("Label Type", typeName)) {
                if (ImGui::Selectable("Text", currentLabel.type == GDH::Labels::LabelType::Text)) {
                    currentLabel.type = GDH::Labels::LabelType::Text;
                }
                if (ImGui::Selectable("Cheat Indicator", currentLabel.type == GDH::Labels::LabelType::CheatIndicator)) {
                    currentLabel.type = GDH::Labels::LabelType::CheatIndicator;
                }
                if (ImGui::Selectable("Spacing", currentLabel.type == GDH::Labels::LabelType::Spacing)) {
                    currentLabel.type = GDH::Labels::LabelType::Spacing;
                }
                ImGui::EndCombo();
            }
            
            if (currentLabel.type == GDH::Labels::LabelType::Text) {
                ImGui::InputText("Label Text", &currentLabel.text);
                ImGui::ColorEdit4("Color", currentLabel.color.data());
                
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                ImGui::Text("Color Action:");
                ImGuiH::Checkbox("Rainbow", &currentLabel.rainbow); ImGui::SameLine();
                ImGuiH::Checkbox("CPS", &currentLabel.cps); ImGui::SameLine();
                ImGuiH::Checkbox("Noclip", &currentLabel.noclip);
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                
                ImGuiH::DragFloat("Size", &currentLabel.size, 0.1f, 0.1f, 4.0f);
            } else if (currentLabel.type == GDH::Labels::LabelType::CheatIndicator) {
                ImGuiH::DragFloat("Size", &currentLabel.size, 0.1f, 0.1f, 4.0f);
                ImGuiH::DragFloat("Opacity", &currentLabel.color[3], 0.01f, 0.f, 1.f);
            } else if (currentLabel.type == GDH::Labels::LabelType::Spacing) {
                ImGuiH::DragFloat("##Spacing", &currentLabel.size, 1.0f, 1.0f, 256.0f, "Spacing: %.1fpx");
            }
            
            if (!currentLabel.enabled) ImGui::EndDisabled();
            
            if (ImGuiH::Button("Delete")) {
                targetLabels.erase(targetLabels.begin() + g_popupId);
                g_updateSize = true;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::SameLine();
            ImGui::BeginDisabled(g_popupId == 0);
            if (ImGuiH::Button("Move up")) {
                std::swap(targetLabels[g_popupId - 1], targetLabels[g_popupId]);
                g_popupId--;
            }
            ImGui::EndDisabled();
            
            ImGui::SameLine();
            ImGui::BeginDisabled(g_popupId == static_cast<int>(targetLabels.size()) - 1);
            if (ImGuiH::Button("Move down")) {
                std::swap(targetLabels[g_popupId], targetLabels[g_popupId + 1]);
                g_popupId++;
            }
            ImGui::EndDisabled();
            
            ImGui::EndPopup();
        }
    });
}
#endif