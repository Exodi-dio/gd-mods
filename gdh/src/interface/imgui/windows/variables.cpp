#ifdef GEODE_IS_DESKTOP
#include "imgui.h"
#include <Geode/Geode.hpp>
#include "../../../core/gui.hpp"
#include <imgui_stdlib.h>
#include <imgui-cocos.hpp>
#include "../widgetH.hpp"

$execute {
    auto& gui = GDH::Gui::get();
    auto& window = gui.getWindow("Variables");

    window.setCustomWindowImGui([] {
        static int type_index = 0;
        static int player_index = 0;
        static int creator_index = 0;
        static std::string value;

        const char* types[] = {"Creator", "Player"};
        const char* player_items[] = {
            "Attempts", "Jumps", "Normal %", "Position X", "Position Y", 
            "Practice %", "Song ID", "Speed", "Level ID"
        };
        const char* creator_items[] = {"Object ID"};

        const float width = ImGui::GetContentRegionAvail().x;

        ImGui::SetNextItemWidth(width);
        ImGui::Combo("##TypeVars", &type_index, types, IM_ARRAYSIZE(types));

        ImGui::SetNextItemWidth(width);
        if (type_index == 0) {
            ImGui::Combo("##CreatorVars", &creator_index, creator_items, IM_ARRAYSIZE(creator_items));
        } else {
            ImGui::Combo("##PlayerVars", &player_index, player_items, IM_ARRAYSIZE(player_items));
        }

        ImGui::SetNextItemWidth(width);
        ImGui::InputText("##ValueVar", &value);

        auto processInt = [](auto& target, bool isSet) {
            if (isSet) {
                if (auto res = geode::utils::numFromString<int>(value)) {
                    target = res.unwrap();
                }
            } else {
                value = std::to_string(static_cast<int>(target));
            }
        };

        auto processFloat = [](float& target, bool isSet) {
            if (isSet) {
                if (auto res = geode::utils::numFromString<float>(value)) {
                    target = res.unwrap();
                }
            } else {
                value = std::to_string(target);
            }
        };

        auto executeGetSet = [&](bool isSet) {
            if (type_index == 0) {
                if (auto editor = EditorUI::get()) {
                    processInt(editor->m_selectedObjectIndex, isSet);
                }
            } else {
                if (auto pl = PlayLayer::get()) {
                    if (!pl->m_level || !pl->m_player1) return;
                    auto& level = *pl->m_level;
                    auto& player = *pl->m_player1;

                    switch (player_index) {
                        case 0: processInt(level.m_attempts, isSet); break;
                        case 1: processInt(level.m_jumps, isSet); break;
                        case 2: processInt(level.m_normalPercent, isSet); break;
                        case 3: processFloat(player.m_position.x, isSet); break;
                        case 4: processFloat(player.m_position.y, isSet); break;
                        case 5: processInt(level.m_practicePercent, isSet); break;
                        case 6: processInt(level.m_songID, isSet); break;
                        case 7: processFloat(player.m_playerSpeed, isSet); break;
                        case 8: processInt(level.m_levelID, isSet); break;
                    }
                }
            }
        };

        if (ImGuiH::Button("Get", {width / 2.f, 0.0f})) {
            executeGetSet(false);
        }

        ImGui::SameLine();

        if (ImGuiH::Button("Set", {ImGui::GetContentRegionAvail().x, 0.0f})) {
            executeGetSet(true);
        }
    });
}
#endif
