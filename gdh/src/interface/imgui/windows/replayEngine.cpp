#ifdef GEODE_IS_DESKTOP
#include <Geode/Geode.hpp>
#include "imgui.h"
#include <imgui_stdlib.h>
#include <imgui-cocos.hpp>
#include "../../../core/config.hpp"
#include "../../../core/gui.hpp"
#include "../../../core/replayEngine.hpp"
#include "../../../core/config.hpp"
#include "../../../core/utils.hpp"

#include "../../../core/recorder/recorder.hpp"

#ifdef GEODE_IS_WINDOWS
#include "../../../core/recorder/subprocess.hpp"
#endif

#include "../widgetH.hpp"
#include "../widget_helper.hpp"
#include "../layout.hpp"

#include "replayEngine.hpp"

#ifdef GEODE_IS_WINDOWS
std::string getFFmpegPath() {
    static std::string cachedPath = "";
    static bool checked = false;

    if (checked) return cachedPath;
    checked = true;

    static geode::Mod* loader = geode::Loader::get()->getLoadedMod("geode.loader");
    bool consoleEnabled = loader->getSettingValue<bool>("show-platform-console");
    GDH::Recorder::get().console = consoleEnabled;

    auto geodePath = geode::dirs::getGameDir() / "ffmpeg.exe";
    if (std::filesystem::exists(geodePath)) {
        cachedPath = geodePath.string();
        return cachedPath;
    }

    subprocess::Popen process("where ffmpeg");
    if (process.close(true) == 0) {
        cachedPath = "ffmpeg";
        return cachedPath;
    }

    cachedPath = "";
    return cachedPath;
}
#endif

void savePreset(const std::filesystem::path& path, const GDH::Recorder& recorder) {
    std::ofstream file(path);
    if (!file.is_open()) return;

    file << "width=" << recorder.width << "\n";
    file << "height=" << recorder.height << "\n";
    file << "fps=" << recorder.fps << "\n";
    file << "bitrate=" << recorder.bitrate << "\n";
    file << "codec=" << recorder.codec << "\n";
    file << "extra_args=" << recorder.extra_args << "\n";
    file << "vf_args=" << recorder.vf_args << "\n";
    file << "audio_args=" << recorder.audio_args << "\n";
    file << "record_audio=" << (recorder.record_audio ? 1 : 0) << "\n";
}

void loadPreset(const std::filesystem::path& path, GDH::Recorder& recorder) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);

        if (key == "width") recorder.width = std::stoi(val);
        else if (key == "height") recorder.height = std::stoi(val);
        else if (key == "fps") recorder.fps = std::stoi(val);
        else if (key == "bitrate") recorder.bitrate = val;
        else if (key == "codec") recorder.codec = val;
        else if (key == "extra_args") recorder.extra_args = val;
        else if (key == "vf_args") recorder.vf_args = val;
        else if (key == "audio_args") recorder.audio_args = val;
        else if (key == "record_audio") recorder.record_audio = (val == "1");
    }
}

void drawRecorderInterface() {
    std::filesystem::path folder = Config::get().get<std::string>("recorder::path", (getFolderPath() / "Showcases").string());

    #ifdef GEODE_IS_WINDOWS
    std::string ffmpegPath = getFFmpegPath();

    if (ffmpegPath.empty()) {
        ImGui::TextColored(ImColor(255, 128, 128).Value, "FFmpeg not found!");
        ImGui::TextWrapped("Please place ffmpeg.exe in your Geometry Dash folder or add it to system PATH to use the recorder");
        
        if (ImGuiH::Button("Download")) {
            geode::utils::web::openLinkInBrowser("https://github.com/AnimMouse/ffmpeg-autobuild/releases/latest");
        } ImGui::SameLine();

        if (ImGuiH::Button("Open GD Folder")) {
            geode::utils::file::openFolder(geode::dirs::getGameDir());
        }
        return;
    }
    #endif

    auto& layout = GDH::Layout::Manager::get();
    auto& recorder = GDH::Recorder::get();
    if (ImGuiH::Checkbox("Record##Recorder", &recorder.enabled)) {
        if (recorder.enabled) {
            if (GDH::Utils::isOnlyAsciiPath(folder)) {
                recorder.compile_command();
                recorder.start();
            }
            else {
                recorder.enabled = false;
                ImGuiH::AddPopup("Invalid path to the showcase folder. Please remove any Cyrillic characters");
            }
        }
        else {
            recorder.stop();
        }
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("##videoname", &recorder.videoName);
    
    ImGuiH::Checkbox("Include Audio", &recorder.record_audio);
    
    #ifdef GEODE_IS_WINDOWS
    ImGui::SameLine();

    static geode::Mod* loader = geode::Loader::get()->getLoadedMod("geode.loader");
    bool consoleEnabled = loader->getSettingValue<bool>("show-platform-console");
    if (ImGuiH::Checkbox("Show Console (Requires Restart)", &consoleEnabled)) {
        loader->setSettingValue("show-platform-console", consoleEnabled);
    }

    #endif
    
    ImGui::Text("Resolution:");
    ImGui::Separator();

    ImGui::PushItemWidth(layout.multipleScale(45.f));
    if (ImGui::InputInt("##width", &recorder.width, 0) && recorder.lock_aspect_ratio) {
        float aspect_ratio = 16.0f / 9.0f;
        recorder.height = static_cast<int>(recorder.width / aspect_ratio);
    }
    ImGui::SameLine(0, 5);

    ImGui::Text("x");
    ImGui::SameLine(0, 5);

    ImGui::PushItemWidth(layout.multipleScale(45.f));
    if (ImGui::InputInt("##height", &recorder.height, 0) && recorder.lock_aspect_ratio) {
        float aspect_ratio = 16.0f / 9.0f;
        recorder.width = static_cast<int>(recorder.height * aspect_ratio);
    }
    ImGui::SameLine(0, 5);

    ImGui::Text("@");
    ImGui::SameLine(0, 5);

    ImGui::PushItemWidth(layout.multipleScale(35.f));
    ImGui::InputInt("##fps", &recorder.fps, 0);

    ImGui::SameLine(0, 5);
    ImGuiH::Checkbox("Lock Aspect Ratio (16:9)", &recorder.lock_aspect_ratio);

    ImGui::Spacing();
    ImGui::Text("Encoding Settings");
    ImGui::Separator(); 

    ImGui::PushItemWidth(layout.multipleScale(50.f));
    ImGui::InputText("Bitrate", &recorder.bitrate); 

    #ifdef GEODE_IS_WINDOWS

    ImGui::SameLine();
    ImGui::PushItemWidth(layout.multipleScale(120.f));
    ImGui::InputText("Codec", &recorder.codec);

    ImGui::PushItemWidth(layout.multipleScale(450.f));
    ImGui::InputText("Extra Arguments", &recorder.extra_args);

    ImGui::PushItemWidth(layout.multipleScale(450.f));
    ImGui::InputText("VF Args", &recorder.vf_args);

    ImGui::PushItemWidth(layout.multipleScale(450.f));
    ImGui::InputText("Audio Merge Args", &recorder.audio_args);

    #endif

    ImGui::Spacing();
    ImGui::Text("Built-in Presets");
    ImGui::Separator();

    if (ImGuiH::Button("HD")) { recorder.width = 1280; recorder.height = 720; recorder.fps = 60; recorder.bitrate = "25M"; }   
    ImGui::SameLine();  
    if (ImGuiH::Button("FULL HD")) { recorder.width = 1920; recorder.height = 1080; recorder.fps = 60; recorder.bitrate = "50M"; }   
    ImGui::SameLine();  
    if (ImGuiH::Button("2K")) { recorder.width = 2560; recorder.height = 1440; recorder.fps = 60; recorder.bitrate = "70M"; }   
    ImGui::SameLine();  
    if (ImGuiH::Button("4K")) { recorder.width = 3840; recorder.height = 2160; recorder.fps = 60; recorder.bitrate = "80M"; }   

    #ifdef GEODE_IS_WINDOWS
    ImGui::SameLine();  
    if (ImGuiH::Button("8K")) { recorder.width = 7680; recorder.height = 4320; recorder.fps = 60; recorder.bitrate = "250M"; }

    if (ImGuiH::Button("CPU x264")) {
        recorder.codec = "libx264";
        recorder.extra_args = "-pix_fmt yuv420p -preset ultrafast";
    } ImGui::SameLine();  

    if (ImGuiH::Button("CPU x265")) {
        recorder.codec = "libx265";
        recorder.extra_args = "-pix_fmt yuv420p -preset ultrafast";
    } ImGui::SameLine();

    if (ImGuiH::Button("CPU AV1 Lossless")) {
        recorder.codec = "libsvtav1";
        recorder.extra_args = "-crf 0 -pix_fmt yuv420p";
    }

    if (ImGuiH::Button("NVIDIA H264")) {
        recorder.codec = "h264_nvenc";
        recorder.extra_args = "-pix_fmt yuv420p -preset p7";
    } ImGui::SameLine();

    if (ImGuiH::Button("NVIDIA H265")) {
        recorder.codec = "hevc_nvenc";
        recorder.extra_args = "-pix_fmt yuv420p -preset p7";
    } ImGui::SameLine();  
    
    if (ImGuiH::Button("NVIDIA AV1 (Only RTX40 Series)")) {
        recorder.codec = "av1_nvenc";
        recorder.extra_args = "-pix_fmt yuv420p -preset p7";
    }

    if (ImGuiH::Button("AMD H264")) {
        recorder.codec = "h264_amf";
        recorder.extra_args = "-pix_fmt yuv420p -quality quality";
    } ImGui::SameLine();

    if (ImGuiH::Button("AMD H265")) {
        recorder.codec = "hevc_amf";
        recorder.extra_args = "-pix_fmt yuv420p -quality quality";
    } ImGui::SameLine();

    if (ImGuiH::Button("AMD AV1 (Only RX 7000 Series)")) {
        recorder.codec = "av1_amf";
        recorder.extra_args = "-pix_fmt yuv420p -quality quality";
    }
    #endif

    ImGui::Spacing();
    ImGui::Text("Custom Presets");
    ImGui::Separator();

    static std::string customPresetName = "";
    static std::vector<std::filesystem::path> presetFiles;
    static bool presetsNeedRefresh = true;

    if (presetsNeedRefresh) {
        presetFiles.clear();
        if (std::filesystem::exists(getFolderPresetsPath())) {
            for (const auto& entry : std::filesystem::directory_iterator(getFolderPresetsPath())) {
                if (entry.is_regular_file() && entry.path().extension() == ".preset") {
                    presetFiles.push_back(entry.path());
                }
            }
        }
        presetsNeedRefresh = false;
    }

    ImGui::SetNextItemWidth(layout.multipleScale(220.f));
    ImGui::InputTextWithHint("##custom_preset_name", "Enter preset name", &customPresetName);
    
    ImGui::SameLine();
    
    if (ImGuiH::Button("Save Preset") && !customPresetName.empty()) {
        std::filesystem::create_directories(getFolderPresetsPath());
        auto path = getFolderPresetsPath() / (customPresetName + ".preset");
        savePreset(path, recorder);
        presetsNeedRefresh = true;
    }

    ImGui::SameLine();

    if (ImGuiH::Button("Load Preset") && !customPresetName.empty()) {
        auto path = getFolderPresetsPath() / (customPresetName + ".preset");
        if (std::filesystem::exists(path)) {
            loadPreset(path, recorder);
        }
    }

    ImGui::SameLine();

    if (ImGuiH::Button("Delete Preset") && !customPresetName.empty()) {
        auto path = getFolderPresetsPath() / (customPresetName + ".preset");
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
            presetsNeedRefresh = true;
        }
    }

    ImGui::Spacing();

    if (!presetFiles.empty()) {
        for (const auto& path : presetFiles) {
            std::string name = path.stem().string();

            if (ImGuiH::Button(name.c_str())) {
                customPresetName = name;
                loadPreset(path, recorder);
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
    }

    ImGui::Spacing();
    ImGui::Text("Folders");
    ImGui::Separator();

    if (ImGuiH::Button("Open Showcase Folder")) {
        geode::utils::file::openFolder(folder);
    }
    ImGui::SameLine();
    if (ImGuiH::Button("Change Showcase Folder")) {
        geode::async::spawn(
            geode::utils::file::pick(
                geode::utils::file::PickMode::OpenFolder,
                {}
            ),
            [&recorder](geode::utils::file::PickResult res) {
                if (!res) return;
                auto pathOpt = std::move(res).unwrapOr(std::nullopt);
                if (!pathOpt.has_value()) return;

                auto folderPath = std::move(pathOpt).value();
                Config::get().set<std::string>("recorder::path", (folderPath).string());
            }
        );
    } ImGui::SameLine();

    if (ImGuiH::Button("Reset Showcase Folder")) {
        Config::get().set<std::string>("recorder::path", (getFolderPath() / "Showcases").string());
    }

    if (ImGuiH::Button("Open Presets Folder")) {
        geode::utils::file::openFolder(getFolderPresetsPath());
    }

    ImGui::Text("Showcase Folder: %s", folder.string().c_str());
}

void GDH::UI::drawMacroEditor() {
    static int selected_frame_type = 1;
    static int selected_index = -1;
    static int prev_frame_type = selected_frame_type;
    static int target_frame_input = 0;
    static int shift_offset_input = 0;
    static bool request_scroll = false;
    const char* frame_types[] = { "Physic Frames", "Input Frames" };

    if (prev_frame_type != selected_frame_type) {
        selected_index = -1;
        prev_frame_type = selected_frame_type;
        request_scroll = true;
    }
    auto& engine = GDH::ReplayEngine::get();
    auto& layout = GDH::Layout::Manager::get();

    // imgui table sucks so making custom table
    auto drawFakeTable = [&](const char* child_id, const char* const* headers, const float* col_frac, int col_count, int row_count, geode::Function<void(int, int)> draw_cell) {
        bool child_open = ImGui::BeginChild(child_id, ImVec2(ImGui::GetContentRegionAvail().x - layout.multipleScale(250.f), 0), true);
        if (child_open) {
            float table_width = ImGui::GetContentRegionAvail().x;
            float table_left_x = ImGui::GetCursorPosX();
            ImVec2 table_origin_screen = ImGui::GetCursorScreenPos();
            float row_height = ImGui::GetTextLineHeightWithSpacing();
            float col_x[16];
            float acc = 0.0f;
            for (int c = 0; c < col_count; c++) {
                col_x[c] = acc * table_width;
                acc += col_frac[c];
            }

            for (int c = 0; c < col_count; c++) {
                if (c > 0) ImGui::SameLine();
                ImGui::SetCursorPosX(table_left_x + col_x[c]);
                ImGui::TextUnformatted(headers[c]);
            }
            ImGui::Separator();

            ImGuiListClipper clipper;
            clipper.Begin(row_count, row_height);
            if (request_scroll && selected_index >= 0 && selected_index < row_count)
                clipper.IncludeItemByIndex(selected_index);

            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                    bool is_selected = (selected_index == i);
                    ImVec2 row_pos = ImGui::GetCursorPos();
                    ImVec2 row_screen_pos = ImGui::GetCursorScreenPos();

                    if (i % 2 == 1) {
                        ImGui::GetWindowDrawList()->AddRectFilled(
                            row_screen_pos,
                            ImVec2(row_screen_pos.x + table_width, row_screen_pos.y + row_height),
                            ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
                    }

                    std::string sel_id = std::string("##row") + child_id + std::to_string(i);
                    if (ImGui::Selectable(sel_id.c_str(), is_selected, ImGuiSelectableFlags_AllowOverlap, ImVec2(table_width, row_height))) {
                        selected_index = i;
                    }
                    if (is_selected && request_scroll) {
                        ImGui::SetScrollHereY(0.5f);
                        request_scroll = false;
                    }

                    ImGui::SetCursorPos(row_pos);
                    for (int c = 0; c < col_count; c++) {
                        if (c > 0) ImGui::SameLine();
                        ImGui::SetCursorPosX(table_left_x + col_x[c]);
                        draw_cell(i, c);
                    }
                }
            }

            float body_bottom_screen_y = ImGui::GetCursorScreenPos().y;
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            for (int c = 1; c < col_count; c++) {
                float x = table_origin_screen.x + col_x[c] - ImGui::GetStyle().ItemSpacing.x * 0.5f;
                draw_list->AddLine(ImVec2(x, table_origin_screen.y), ImVec2(x, body_bottom_screen_y), ImGui::GetColorU32(ImGuiCol_Border));
            }
        }
        ImGui::EndChild();
    };

    if (selected_frame_type == 0) {
        auto& frames = engine.get_physic_frames();
        if (frames.empty()) {
            selected_index = -1;
        } else if (selected_index < 0) {
            selected_index = 0;
        } else if (selected_index >= (int)frames.size()) {
            selected_index = (int)frames.size() - 1;
        }

        static const char* headers[] = { "Frame", "Player", "X", "Y", "Y Accel" };
        static const float fracs[] = { 0.20f, 0.11f, 0.23f, 0.23f, 0.23f };

        drawFakeTable("PhysicFakeTable", headers, fracs, 5, (int)frames.size(), [&](int i, int c) {
            const auto& frame = frames[i];
            switch (c) {
                case 0: ImGui::Text("%llu", (unsigned long long)frame.frame); break;
                case 1: ImGui::TextUnformatted(frame.isPlayer2 ? "P2" : "P1"); break;
                case 2: ImGui::Text("%f", frame.x); break;
                case 3: ImGui::Text("%f", frame.y); break;
                case 4: ImGui::Text("%f", frame.y_accel); break;
            }
        });
    } else {
        auto& frames = engine.get_input_frames();
        if (frames.empty()) {
            selected_index = -1;
        } else if (selected_index < 0) {
            selected_index = 0;
        } else if (selected_index >= (int)frames.size()) {
            selected_index = (int)frames.size() - 1;
        }

        static const char* headers[] = { "Frame", "Player", "Button", "Action" };
        static const float fracs[] = { 0.20f, 0.15f, 0.30f, 0.40f };

        drawFakeTable("InputFakeTable", headers, fracs, 4, (int)frames.size(), [&](int i, int c) {
            const auto& frame = frames[i];
            switch (c) {
                case 0: ImGui::Text("%llu", (unsigned long long)frame.frame); break;
                case 1: ImGui::TextUnformatted(frame.isPlayer2 ? "P2" : "P1"); break;
                case 2: ImGui::TextUnformatted(
                    frame.button == 1 ? "Jump" :
                    frame.button == 2 ? "Left" :
                    frame.button == 3 ? "Right" : "Unknown");
                    break;
                case 3: ImGui::TextUnformatted(frame.down ? "Push" : "Release"); break;
            }
        });
    }

    ImGui::SameLine();
    bool settings_open = ImGui::BeginChild("##EditorSettings", { 0, ImGui::GetContentRegionAvail().y });
    if (settings_open) {
        ImGui::Text("Frame Type:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::Combo("##FrameType", &selected_frame_type, frame_types, IM_ARRAYSIZE(frame_types));
        ImGui::Separator();

        ImGui::Text("Go to Frame:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 70.0f);
        ImGui::InputInt("##TargetFrame", &target_frame_input, 0, 0);
        ImGui::SameLine();
        if (ImGuiH::Button("Go##FindFrame", ImVec2(62.0f, 0))) {
            if (selected_frame_type == 0) {
                auto& frames = engine.get_physic_frames();
                for (size_t i = 0; i < frames.size(); i++) {
                    if (frames[i].frame >= static_cast<uint64_t>(target_frame_input)) {
                        selected_index = static_cast<int>(i);
                        request_scroll = true;
                        break;
                    }
                }
            } else {
                auto& frames = engine.get_input_frames();
                for (size_t i = 0; i < frames.size(); i++) {
                    if (frames[i].frame >= static_cast<uint64_t>(target_frame_input)) {
                        selected_index = static_cast<int>(i);
                        request_scroll = true;
                        break;
                    }
                }
            }
        }
        ImGui::Separator();
        
        if (ImGuiH::Button("Sort by Frame", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            if (selected_frame_type == 0) {
                auto& frames = engine.get_physic_frames();
                std::sort(frames.begin(), frames.end(), [](const physic_data& a, const physic_data& b) {
                    return a.frame < b.frame;
                });
            } else {
                auto& frames = engine.get_input_frames();
                std::sort(frames.begin(), frames.end(), [](const input_data& a, const input_data& b) {
                    return a.frame < b.frame;
                });
            }
        }
        ImGui::Separator();

        ImGui::Text("Shift All Frames:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 70.0f);
        ImGui::InputInt("##ShiftOffset", &shift_offset_input, 0, 0);
        ImGui::SameLine();
        if (ImGuiH::Button("Shift##ApplyShift", ImVec2(62.0f, 0))) {
            if (selected_frame_type == 0) {
                auto& frames = engine.get_physic_frames();
                for (auto& f : frames) {
                    int64_t new_f = static_cast<int64_t>(f.frame) + shift_offset_input;
                    f.frame = static_cast<uint64_t>(std::max<int64_t>(0, new_f));
                }
            } else {
                auto& frames = engine.get_input_frames();
                for (auto& f : frames) {
                    int64_t new_f = static_cast<int64_t>(f.frame) + shift_offset_input;
                    f.frame = static_cast<uint64_t>(std::max<int64_t>(0, new_f));
                }
            }
        }

        float half_w = (ImGui::GetContentRegionAvail().x / 2.0f) - 2.0f;

        if (ImGuiH::Button("Delete ALL P1", ImVec2(half_w, 0))) {
            if (selected_frame_type == 0) {
                auto& frames = engine.get_physic_frames();
                std::erase_if(frames, [](const physic_data& f) { return !f.isPlayer2; });
            } else {
                auto& frames = engine.get_input_frames();
                std::erase_if(frames, [](const input_data& f) { return !f.isPlayer2; });
            }
            selected_index = -1;
        }
        ImGui::SameLine();
        if (ImGuiH::Button("Delete ALL P2", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            if (selected_frame_type == 0) {
                auto& frames = engine.get_physic_frames();
                std::erase_if(frames, [](const physic_data& f) { return f.isPlayer2; });
            } else {
                auto& frames = engine.get_input_frames();
                std::erase_if(frames, [](const input_data& f) { return f.isPlayer2; });
            }
            selected_index = -1;
        }

        if (ImGuiH::Button("Flip All P1/P2", ImVec2(selected_frame_type == 1 ? half_w : ImGui::GetContentRegionAvail().x, 0))) {
            if (selected_frame_type == 0) {
                for (auto& f : engine.get_physic_frames()) f.isPlayer2 = !f.isPlayer2;
            } else {
                for (auto& f : engine.get_input_frames()) f.isPlayer2 = !f.isPlayer2;
            }
        }

        if (selected_frame_type == 1) {
            ImGui::SameLine();
            if (ImGuiH::Button("Flip All Push/Rel", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                for (auto& f : engine.get_input_frames()) f.down = !f.down;
            }
        }

        ImGui::Separator();
        ImGui::Text("Actions:");
        if (selected_frame_type == 0) {
            auto& frames = engine.get_physic_frames();
            bool hasSel = !frames.empty() && selected_index >= 0 && selected_index < (int)frames.size();
            if (ImGuiH::Button("Add Action", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                physic_data new_frame{};
                new_frame.x = 0.0f;
                new_frame.y = 0.0f;
                new_frame.y_accel = 0.0;
                if (hasSel) {
                    new_frame.frame = frames[selected_index].frame + 1;
                    new_frame.isPlayer2 = frames[selected_index].isPlayer2;
                    frames.insert(frames.begin() + selected_index + 1, new_frame);
                    selected_index++;
                } else {
                    new_frame.frame = 0;
                    new_frame.isPlayer2 = false;
                    frames.push_back(new_frame);
                    selected_index = (int)frames.size() - 1;
                }
                request_scroll = true;
            }
            if (hasSel) {
                if (ImGuiH::Button("Remove Action", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                    frames.erase(frames.begin() + selected_index);
                    if (frames.empty()) selected_index = -1;
                    else if (selected_index >= (int)frames.size()) selected_index = (int)frames.size() - 1;
                    request_scroll = true;
                }
                ImGui::BeginDisabled(selected_index <= 0);
                if (ImGuiH::Button("Move Up", ImVec2(half_w, 0))) {
                    std::swap(frames[selected_index], frames[selected_index - 1]);
                    selected_index--;
                    request_scroll = true;
                }
                ImGui::EndDisabled();
                ImGui::SameLine();
                ImGui::BeginDisabled(selected_index >= (int)frames.size() - 1);
                if (ImGuiH::Button("Move Down", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                    std::swap(frames[selected_index], frames[selected_index + 1]);
                    selected_index++;
                    request_scroll = true;
                }
                ImGui::EndDisabled();
                ImGui::Separator();
            }
            if (hasSel) {
                auto& selected_frame = frames[selected_index];
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                int frame_cast = static_cast<int>(selected_frame.frame);
                if (ImGuiH::DragInt("Frame##Drag", &frame_cast, 1.f, 0, INT_MAX, "Frame: %i"))
                    selected_frame.frame = static_cast<uint64_t>(frame_cast);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGuiH::DragFloat("X##Drag", &selected_frame.x, 0.001f, -FLT_MAX, FLT_MAX, "X Position: %f");
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGuiH::DragFloat("Y##Drag", &selected_frame.y, 0.001f, -FLT_MAX, FLT_MAX, "Y Position: %f");
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                float yAccel_cast = static_cast<float>(selected_frame.y_accel);
                if (ImGuiH::DragFloat("YAccel##Drag", &yAccel_cast, 0.001f, -FLT_MAX, FLT_MAX, "Y Accel: %f"))
                    selected_frame.y_accel = static_cast<double>(yAccel_cast);
                ImGuiH::Checkbox("Player 2", &selected_frame.isPlayer2);
            } else {
                ImGui::TextColored(ImColor(255, 128, 128).Value, "No physic frames");
            }
        } else {
            auto& frames = engine.get_input_frames();
            bool hasSel = !frames.empty() && selected_index >= 0 && selected_index < (int)frames.size();
            if (ImGuiH::Button("Add Action", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                input_data new_frame{};
                new_frame.button = 1;
                new_frame.down = false;
                if (hasSel) {
                    new_frame.frame = frames[selected_index].frame + 1;
                    new_frame.isPlayer2 = frames[selected_index].isPlayer2;
                    frames.insert(frames.begin() + selected_index + 1, new_frame);
                    selected_index++;
                } else {
                    new_frame.frame = 0;
                    new_frame.isPlayer2 = false;
                    frames.push_back(new_frame);
                    selected_index = (int)frames.size() - 1;
                }
                request_scroll = true;
            }
            if (hasSel) {
                if (ImGuiH::Button("Remove Action", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                    frames.erase(frames.begin() + selected_index);
                    if (frames.empty()) selected_index = -1;
                    else if (selected_index >= (int)frames.size()) selected_index = (int)frames.size() - 1;
                    request_scroll = true;
                }
                ImGui::BeginDisabled(selected_index <= 0);
                if (ImGuiH::Button("Move Up", ImVec2(half_w, 0))) {
                    std::swap(frames[selected_index], frames[selected_index - 1]);
                    selected_index--;
                    request_scroll = true;
                }
                ImGui::EndDisabled();
                ImGui::SameLine();
                ImGui::BeginDisabled(selected_index >= (int)frames.size() - 1);
                if (ImGuiH::Button("Move Down", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                    std::swap(frames[selected_index], frames[selected_index + 1]);
                    selected_index++;
                    request_scroll = true;
                }
                ImGui::EndDisabled();
                ImGui::Separator();
            }
            if (hasSel) {
                auto& selected_frame = frames[selected_index];
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                int frame_cast = static_cast<int>(selected_frame.frame);
                if (ImGuiH::DragInt("Frame##Drag", &frame_cast, 1, 0, INT_MAX, "Frame: %i"))
                    selected_frame.frame = static_cast<uint64_t>(frame_cast);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGuiH::DragInt("Button##Drag", &selected_frame.button, 1, 1, 3, "Button: %i");
                ImGuiH::Checkbox("Push", &selected_frame.down);
                ImGui::SameLine();
                ImGuiH::Checkbox("Player 2", &selected_frame.isPlayer2);
            } else {
                ImGui::TextColored(ImColor(255, 128, 128).Value, "No input frames");
            }

            ImGuiH::SpaceSeparator();
            if (ImGuiH::Button("Import Plain Text", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                geode::async::spawn(
                    geode::utils::file::pick(
                        geode::utils::file::PickMode::OpenFile,
                        geode::utils::file::FilePickOptions{
                            .filters = { { "Text File", { "*.txt" } } }
                        }
                    ),
                    [](geode::utils::file::PickResult res) {
                        if (!res) return;
                        auto pathOpt = std::move(res).unwrapOr(std::nullopt);
                        if (!pathOpt) return;

                        auto selectedPath = *pathOpt;
                        if (!GDH::Utils::isOnlyAsciiPath(selectedPath)) {
                            ImGuiH::AddPopup("Invalid path. Please remove non-ASCII characters");
                            return;
                        }

                        std::ifstream file(selectedPath);
                        if (!file.is_open()) {
                            ImGuiH::AddPopup("Failed to open file for reading");
                            return;
                        }

                        auto& engine = GDH::ReplayEngine::get();

                        engine.get_physic_frames().clear();
                        auto& inputs = engine.get_input_frames();
                        inputs.clear();

                        std::string line;
                        while (std::getline(file, line)) {
                            if (line.empty()) continue;
                            std::stringstream ss(line);
                            
                            int64_t frame = 0;
                            int push = 0;
                            int player = 0;

                            if (ss >> frame >> push >> player) {
                                input_data data{};
                                data.frame = static_cast<uint64_t>(std::max<int64_t>(0, frame));
                                data.down = (push != 0);
                                data.isPlayer2 = (player != 0);
                                data.button = 1;
                                inputs.push_back(data);
                            }
                        }

                        file.close();
                        selected_index = inputs.empty() ? -1 : 0;
                        request_scroll = true;
                        ImGuiH::AddPopup("Macro imported successfully!");
                    }
                );
            }

            if (ImGuiH::Button("Export Plain Text", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                geode::async::spawn(
                    geode::utils::file::pick(
                        geode::utils::file::PickMode::SaveFile,
                        geode::utils::file::FilePickOptions{
                            .defaultPath = "macro.txt",
                            .filters = { { "Text File", { "*.txt" } } }
                        }
                    ),
                    [](geode::utils::file::PickResult res) {
                        if (!res) return;
                        auto pathOpt = std::move(res).unwrapOr(std::nullopt);
                        if (!pathOpt) return;

                        auto selectedPath = *pathOpt;
                        if (!GDH::Utils::isOnlyAsciiPath(selectedPath)) {
                            ImGuiH::AddPopup("Invalid path. Please remove non-ASCII characters");
                            return;
                        }

                        std::ofstream file(selectedPath);
                        if (!file.is_open()) {
                            ImGuiH::AddPopup("Failed to open file for writing");
                            return;
                        }

                        auto& engine = GDH::ReplayEngine::get();
                        const auto& inputs = engine.get_input_frames();

                        for (const auto& f : inputs) {
                            file << static_cast<int64_t>(f.frame) << " " 
                                << (f.down ? 1 : 0) << " " 
                                << (f.isPlayer2 ? 1 : 0) << "\n";
                        }

                        file.close();
                        ImGuiH::AddPopup("Macro exported successfully!");
                    }
                );
            }

            ImGuiH::SpaceSeparator();

            ImGuiWidgetConfig::Checkbox("Show Outside", "re::show_editor", false);
        }
    }
    ImGui::EndChild();
}

$execute {
    auto& gui = GDH::Gui::get();
    auto& config = Config::get();
    auto& layout = GDH::Layout::Manager::get();
    auto& window = gui.getWindow("Replay Engine");
    auto& engine = GDH::ReplayEngine::get();

    window.setCustomWindowImGui([&config, &gui, &layout, &engine] {
        using namespace GDH;

        static std::vector<std::filesystem::path> replay_list;
        static bool isProSettingsOpen = true;

        if (ImGuiH::RadioButton("Disable", engine.mode == state::disable)) {
            engine.mode = state::disable;
            engine.setupHacks(state::disable);
        }
        ImGui::SameLine();
        
        if (ImGuiH::RadioButton("Record", engine.mode == state::record)) {
            if (engine.mode != state::record) {
                if (engine.get_actions_size() > 0) {
                    ImGui::OpenPopup("Record Overwrite Warning");
                } else {
                    engine.clear();
                    engine.setupHacks(state::record);
                    engine.mode = state::record;

                    auto& hack = GDH::Gui::get().getWindow("Invisible").findHackByName("Lock Delta");
                    if (!hack.getEnabled()) {
                        ImGuiH::AddPopup("Recommended to enable Lock Delta for more accurate macro recording (in Framerate Window)");
                    }
                }
            }
        }
        ImGui::SameLine();
        
        if (ImGuiH::RadioButton("Play", engine.mode == state::play)) {
            engine.setupHacks(state::play);
            engine.mode = state::play;
        }


        ImGui::SetCursorPosX(layout.multipleScale(96.f));

        if (ImGuiH::RadioButton("Continue", engine.mode == state::continue_mode)) {
            engine.setupHacks(state::continue_mode);
            engine.mode = state::continue_mode;

            ImGuiH::AddPopup("Don't forget to load the replay to continue the recording. The level will automatically switch to practice mode (and don't forget to place checkpoints!!), it'll play what you recorded and then switch back to recording mode", 15.f);
        }
        
        ImGui::Separator();
        
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - layout.multipleScale(35.f));
        ImGui::InputTextWithHint("##replay_name", "Enter the macro name", &engine.replay_name);
        ImGui::SameLine();

        if (ImGuiH::ArrowButton("##replay_select", ImGuiDir_Down)) {
            replay_list.clear();
            for (const auto& entry : std::filesystem::directory_iterator(getFolderMacroPath())) {
                if (entry.is_regular_file()) {
                    if (entry.path().extension() == ".re4") {
                        if (GDH::Utils::isOnlyAsciiPath(entry)) 
                            replay_list.push_back(entry);
                    }
                }
            }
            ImGui::OpenPopup("Select Replay");
        }
        
        if (ImGuiH::Button("Save", {ImGui::GetContentRegionAvail().x / 3, 0.f})) {
            std::filesystem::path savePath = getFolderMacroPath() / (engine.replay_name + ".re4");
            if (std::filesystem::exists(savePath)) {
                ImGui::OpenPopup("Save Overwrite Warning");
            } else {
                ImGuiH::AddPopup(engine.save(engine.replay_name));
            }
        }
        ImGui::SameLine();
        
        if (ImGuiH::Button("Load", {ImGui::GetContentRegionAvail().x / 2, 0.f})) {
            if (engine.get_actions_size() > 0) {
                ImGui::OpenPopup("Load Overwrite Warning");
            } else {
                ImGuiH::AddPopup(engine.load(engine.replay_name));
            }
        }
        ImGui::SameLine();
        
        if (ImGuiH::Button("Clear", {ImGui::GetContentRegionAvail().x, 0.f})) {
            if (engine.get_actions_size() > 0) {
                ImGui::OpenPopup("Clear Warning");
            } else {
                engine.clear();
                ImGuiH::AddPopup("Replay has been cleared");
            }
        }

        ImGui::Text("Replay Size: %zu/%zu", engine.get_current_index(), engine.get_actions_size());
        ImGui::Text("Frame: %llu", engine.get_frame());

        ImGui::Separator();
        if (ImGui::Selectable("Pro Settings")) {
            isProSettingsOpen = true;
            ImGui::OpenPopup("Pro Settings");
        }

        if (ImGui::BeginPopupModal("Record Overwrite Warning", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
            auto glow_in = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Warning_In]);
            auto glow_out = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Warning_Out]);

            ImGuiH::GlowWindow(glow_in, glow_out, layout.multipleScale(500.f));
            ImGui::Text("You currently have actions stored in the buffer.\nStarting a new recording will clear your current unsaved active macro.\n\nAre you sure you want to proceed?");
            ImGuiH::SpaceSeparator();
            if (ImGuiH::Button("Yes, Overwrite", {ImGui::GetContentRegionAvail().x / 2, 0})) {
                engine.clear();    
                engine.mode = state::record;
                engine.setupHacks(state::record);

                auto& hack = GDH::Gui::get().getWindow("Invisible").findHackByName("Lock Delta");
                if (!hack.getEnabled()) {
                    ImGuiH::AddPopup("Recommended to enable Lock Delta for more accurate macro recording (in Framerate Window)");
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGuiH::Button("Cancel", {ImGui::GetContentRegionAvail().x, 0})) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("Save Overwrite Warning", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
            auto glow_in = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Warning_In]);
            auto glow_out = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Warning_Out]);

            ImGuiH::GlowWindow(glow_in, glow_out, layout.multipleScale(500.f));
            ImGui::Text("A macro named \"%s.re4\" already exists.\nAre you sure you want to overwrite it?", engine.replay_name.c_str());
            ImGuiH::SpaceSeparator();
            if (ImGuiH::Button("Yes, Overwrite", {ImGui::GetContentRegionAvail().x / 2, 0})) {
                ImGuiH::AddPopup(engine.save(engine.replay_name));
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGuiH::Button("Cancel", {ImGui::GetContentRegionAvail().x, 0 })) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("Load Overwrite Warning", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
            auto glow_in = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Warning_In]);
            auto glow_out = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Warning_Out]);

            ImGuiH::GlowWindow(glow_in, glow_out, layout.multipleScale(500.f));
            ImGui::Text("You currently have recorded actions actively loaded.\nLoading this macro will clear your unsaved working progress.\n\nAre you sure you want to proceed?");
            ImGuiH::SpaceSeparator();
            if (ImGuiH::Button("Yes, Load", {ImGui::GetContentRegionAvail().x / 2, 0})) {
                ImGuiH::AddPopup(engine.load(engine.replay_name));
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGuiH::Button("Cancel", {ImGui::GetContentRegionAvail().x, 0})) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("Clear Warning", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
            auto glow_in = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Warning_In]);
            auto glow_out = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Warning_Out]);

            ImGuiH::GlowWindow(glow_in, glow_out, layout.multipleScale(500.f));
            ImGui::Text("Are you sure you want to completely clear the current macro?\nThis action cannot be undone.");
            ImGuiH::SpaceSeparator();
            if (ImGuiH::Button("Yes, Clear", {ImGui::GetContentRegionAvail().x / 2, 0})) {
                engine.clear();
                ImGuiH::AddPopup("Replay has been cleared");
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGuiH::Button("Cancel", {ImGui::GetContentRegionAvail().x, 0})) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("Select Replay", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
            auto glow_in = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_In]);
            auto glow_out = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Out]);

            ImGuiH::GlowWindow(glow_in, glow_out, layout.multipleScale(250.f));
            if (ImGui::BeginChild("Replay List", {layout.multipleScale(400.f), layout.multipleScale(500.f)})) {
                for (const auto& replay : replay_list)
                {
                    std::string filename_str = replay.filename().string();
                    
                    if (ImGuiH::Button(filename_str.c_str(), {ImGui::GetContentRegionAvail().x, 0.0f}))
                    {
                        engine.replay_name = replay.stem().string();
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndChild();
            }

            if (ImGuiH::Button("Open Folder", {ImGui::GetContentRegionAvail().x, 0.f})) {
                geode::utils::file::openFolder(getFolderMacroPath());
            }
            
            if (ImGuiH::Button("Close", {ImGui::GetContentRegionAvail().x, 0.f})) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        static bool onceResize = false;
        if (!onceResize && ImGui::IsPopupOpen("Pro Settings")) {
            onceResize = true;
            ImGui::SetNextWindowSize({layout.multipleScale(800.f), layout.multipleScale(655.f)});
        }
        
        if (ImGui::BeginPopupModal("Pro Settings", &isProSettingsOpen)) {
            auto glow_in = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_In]);
            auto glow_out = ImGui::ColorConvertFloat4ToU32(ImGuiH::colorTable[ImGuiH::Glow_Popup_Out]);
            ImGuiH::GlowWindow(glow_in, glow_out);

            if (ImGui::BeginTabBar("ProSettingsTabBar")) {
                if (ImGui::BeginTabItem("Settings")) {
                    ImGui::Spacing();

                    ImGuiWidgetConfig::Checkbox("Accuracy Fix", "engine::accuracy_fix", true);
                    ImGui::SameLine();
                    ImGuiWidgetConfig::Checkbox("Y Velocity Fix", "engine::velocity_fix", false);
                    ImGui::SameLine();
                    ImGuiWidgetConfig::Checkbox("Ignore Inputs on Playback", "engine::ignore_inputs", true);

                    ImGuiH::SpaceSeparator();
                    ImGui::Text("Replay Engine v3 to Replay Engine v4 Converter");
                    if (ImGui::TextLink("https://tobyadd.pages.dev/GDH/re3_to_re4")) {
                        geode::utils::web::openLinkInBrowser("https://tobyadd.pages.dev/GDH/re3_to_re4");
                    }
                    ImGuiH::SpaceSeparator();
                    if (ImGuiH::Button("Reset TPS Warning Popup")) {
                        config.set<bool>("invisible.tps_warning_dont_show_again", false);
                    }
                    
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Recorder")) {
                    ImGui::Spacing();
                    drawRecorderInterface();
                    ImGui::EndTabItem();
                }

                if (!config.get<bool>("re::show_editor", false)) {
                    if (ImGui::BeginTabItem("Editor (Beta)")) {
                        ImGui::Spacing();
                        GDH::UI::drawMacroEditor();
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }

            ImGui::EndPopup();
        }
    });
}
#endif