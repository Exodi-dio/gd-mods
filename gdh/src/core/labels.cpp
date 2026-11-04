#include "labels.hpp"
#include "utils.hpp"
#include "replayEngine.hpp"
#include "config.hpp"

#include <json.hpp>

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace GDH::Labels;

static std::chrono::steady_clock::time_point g_sessionStart;
static float g_rainbowHue = 0.0f;

static const std::unordered_set<int> g_robtopLevels = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    5001, 5002, 5003, 5004, 3001
};

struct CornerProps {
    cocos2d::CCPoint anchor;
    geode::Label::Alignment align;
};

static const std::string_view getCornerKey(Corner corner) {
    switch (corner) {
        case Corner::Top_Left: return "labelsTL";
        case Corner::Top_Center: return "labelsTC";
        case Corner::Top_Right: return "labelsTR";
        case Corner::Center_Left: return "labelsCL";
        case Corner::Center_Center: return "labelsCC";
        case Corner::Center_Right: return "labelsCR";
        case Corner::Bottom_Left: return "labelsBL";
        case Corner::Bottom_Center: return "labelsBC";
        case Corner::Bottom_Right: return "labelsBR";
    }
    return "labelsTL";
}

static const CornerProps getCornerProps(Corner corner) {
    switch (corner) {
        case Corner::Top_Left:      return {{0.0f, 1.0f}, geode::Label::Alignment::Left};
        case Corner::Top_Center:    return {{0.5f, 1.0f}, geode::Label::Alignment::Center};
        case Corner::Top_Right:     return {{1.0f, 1.0f}, geode::Label::Alignment::Right};
        case Corner::Center_Left:   return {{0.0f, 1.0f}, geode::Label::Alignment::Left};
        case Corner::Center_Center: return {{0.5f, 1.0f}, geode::Label::Alignment::Center};
        case Corner::Center_Right:  return {{1.0f, 1.0f}, geode::Label::Alignment::Right};
        case Corner::Bottom_Left:   return {{0.0f, 1.0f}, geode::Label::Alignment::Left};
        case Corner::Bottom_Center: return {{0.5f, 1.0f}, geode::Label::Alignment::Center};
        case Corner::Bottom_Right:  return {{1.0f, 1.0f}, geode::Label::Alignment::Right};
    }
    return {{0.0f, 1.0f}, geode::Label::Alignment::Left};
}

static cocos2d::CCPoint getCornerPos(Corner corner, float cornerPadding, const cocos2d::CCSize& winSize, float y, float labelsHeight) {
    switch (corner) {
        case Corner::Top_Left:      return {cornerPadding, winSize.height - cornerPadding - y};
        case Corner::Top_Center:    return {winSize.width / 2.0f, winSize.height - cornerPadding - y};
        case Corner::Top_Right:     return {winSize.width - cornerPadding, winSize.height - cornerPadding - y};
        case Corner::Center_Left:   return {cornerPadding, winSize.height / 2.0f + labelsHeight / 2.0f - y};
        case Corner::Center_Center: return {winSize.width / 2.0f, winSize.height / 2.0f + labelsHeight / 2.0f - y};
        case Corner::Center_Right:  return {winSize.width - cornerPadding, winSize.height / 2.0f + labelsHeight / 2.0f - y};
        case Corner::Bottom_Left:   return {cornerPadding, cornerPadding + labelsHeight - y};
        case Corner::Bottom_Center: return {winSize.width / 2.0f, cornerPadding + labelsHeight - y};
        case Corner::Bottom_Right:  return {winSize.width - cornerPadding, cornerPadding + labelsHeight - y};
    }
    return {0, 0};
}

$execute {
    GDH::Labels::Manager::get().load();
}

bool renderVariable(geode::utils::StringBuffer<> &stream, std::string_view var_name) {
    auto pl = PlayLayer::get();
    bool isDualAnd2P = pl && pl->m_gameState.m_isDualMode && pl->m_level && pl->m_level->m_twoPlayerMode;

    if (var_name == "time_24") {
        stream.append(fmt::format("{:%T}", geode::localtime(std::time(nullptr))));
    } else if (var_name == "time_12") {
        stream.append(fmt::format("{:%I:%M:%S %p}", geode::localtime(std::time(nullptr))));
    } else if (var_name == "date") {
        stream.append(fmt::format("{:%F}", geode::localtime(std::time(nullptr))));
    } else if (var_name == "clicks") {
        stream.append(CpsCounter::get().overall + CpsCounter::get().p2_overall);
    } else if (var_name == "cps") {
        stream.append(CpsCounter::get().cps + CpsCounter::get().p2_cps);
    } else if (var_name == "cps_high") {
        stream.append(CpsCounter::get().highscore + CpsCounter::get().p2_highscore);
    } else if (var_name == "p1::clicks") {
        stream.append(CpsCounter::get().overall);
    } else if (var_name == "p1::cps") {
        stream.append(CpsCounter::get().cps);
    } else if (var_name == "p1::cps_high") {
        stream.append(CpsCounter::get().highscore);
    } else if (var_name == "p2::clicks") {
        if (!isDualAnd2P) return false;
        stream.append(CpsCounter::get().p2_overall);
    } else if (var_name == "p2::cps") {
        if (!isDualAnd2P) return false;
        stream.append(CpsCounter::get().p2_cps);
    } else if (var_name == "p2::cps_high") {
        if (!isDualAnd2P) return false;
        stream.append(CpsCounter::get().p2_highscore);
    } else if (var_name == "testmode") {
        stream.append(pl ? (pl->m_isTestMode ? "Testmode" : "") : "");
    } else if (var_name == "attempt") {
        stream.append(pl ? pl->m_attempts : 0);
    } else if (var_name == "level_name") {
        stream.append(pl ? pl->m_level->m_levelName : "");
    } else if (var_name == "level_creator") {
        if (pl && pl->m_level) {
            if (g_robtopLevels.contains(pl->m_level->m_levelID)) {
                stream.append("RobTop");
            } else {
                stream.append(pl->m_level->m_creatorName);
            }
        }
    } else if (var_name == "level_id") {
        stream.append(pl ? pl->m_level->m_levelID : 0);
    } else if (var_name == "normal_percent") {
        stream.append(pl ? pl->m_level->getNormalPercent() : 0);
    } else if (var_name == "practice_percent") {
        stream.append(pl ? pl->m_level->m_practicePercent : 0);
    } else if (var_name == "noclip_accuracy") {
        stream.append(fmt::format("{:.2f}", NoclipAccuracy::get().getPercentage()));
    } else if (var_name == "deaths") {
        stream.append(NoclipAccuracy::get().deaths_full);
    } else if (var_name == "frame") {
        stream.append(GDH::ReplayEngine::get().get_frame());
    } else if (var_name == "re_state") {
        auto& engine = GDH::ReplayEngine::get();
        stream.append(engine.mode == state::disable ? "Disable" : engine.mode == state::record ? "Record" : "Play");
    } else if (var_name == "session_time") {
        auto dur = std::chrono::steady_clock::now() - g_sessionStart;
        auto hours = std::chrono::duration_cast<std::chrono::hours>(dur);
        auto mins = std::chrono::duration_cast<std::chrono::minutes>(dur) % 60;
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur) % 60;
        stream.append(fmt::format("{:02}:{:02}:{:02}", hours.count(), mins.count(), secs.count()));
    } else if (var_name == "progress" || var_name.starts_with("progress:")) {
        if (pl && pl->m_level && pl->m_level->isPlatformer()) {
            bool showMS = Config::get().get<bool>("cosmetic.accurate_percentage::showMilliseconds", true);
            stream.append(GDH::Utils::formatTime(pl->m_timePlayed, showMS));
        } else {
            var_name.remove_prefix(8);
            int decimal_points = 0;
            if (!var_name.empty()) {
                var_name.remove_prefix(1);
                auto parsed = geode::utils::numFromString<int>(var_name, 10);
                if (parsed.ok()) decimal_points = *parsed;
            }
            stream.append(fmt::format("{:.{}f}%", (pl ? GDH::Utils::getRealProgress(pl) : 0.0f), decimal_points));
        }
    } else if (var_name == "fps" || var_name.starts_with("fps:")) {
        var_name.remove_prefix(3);
        int decimal_points = 0;
        if (!var_name.empty()) {
            var_name.remove_prefix(1);
            auto parsed = geode::utils::numFromString<int>(var_name, 10);
            if (parsed.ok()) decimal_points = *parsed;
        }
        stream.append(fmt::format("{:.{}f}", GDH::Utils::getFps(true), decimal_points));
    } else if (var_name == "is_dual_mode") {
        stream.append(fmt::format("{}", pl ? pl->m_gameState.m_isDualMode : false));
    } else if (var_name == "p1::x") {
        stream.append(fmt::format("P1X: {}", pl ? pl->m_player1->m_position.x : 0.f));
    } else if (var_name == "p1::y") {
        stream.append(fmt::format("P1Y: {}", pl ? pl->m_player1->m_position.y : 0.f));
    } else if (var_name == "p1::y_vel") {
        stream.append(fmt::format("P1YVEL: {}", pl ? pl->m_player1->m_yVelocity : 0.0));
    } else if (var_name == "p1::is_upside_down") {
        stream.append(fmt::format("{}", (pl && pl->m_player1) ? pl->m_player1->m_isUpsideDown : false));
    } else if (var_name == "p2::x") {
        if (!isDualAnd2P || !pl->m_player2) return false;
        stream.append(fmt::format("P2X: {}", pl->m_player2->m_position.x));
    } else if (var_name == "p2::y") {
        if (!isDualAnd2P || !pl->m_player2) return false;
        stream.append(fmt::format("P2Y: {}", pl->m_player2->m_position.y));
    } else if (var_name == "p2::y_vel") {
        if (!isDualAnd2P || !pl->m_player2) return false;
        stream.append(fmt::format("P2YVEL: {}", pl->m_player2->m_yVelocity));
    } else if (var_name == "p2::is_upside_down") {
        if (!isDualAnd2P || !pl->m_player2) return false;
        stream.append(fmt::format("{}", pl->m_player2->m_isUpsideDown));
    } else if (var_name == "best_run" || var_name.starts_with("best_run:")) {
        if (pl && pl->m_level && pl->m_level->isPlatformer()) {
            return false;
        }

        int decimal_points = 0;
        if (var_name.starts_with("best_run:")) {
            var_name.remove_prefix(9);
            auto parsed = geode::utils::numFromString<int>(var_name, 10);
            if (parsed.ok()) decimal_points = *parsed;
        }

        auto& mgr = GDH::Labels::Manager::get();
        if (mgr.m_bestRuns.empty()) {
            stream.append("N/A");
        } else {
            for (size_t i = 0; i < mgr.m_bestRuns.size(); ++i) {
                const auto& run = mgr.m_bestRuns[i];
                stream.append(fmt::format("{:.{}f}%-{:.{}f}%", run.start, decimal_points, run.end, decimal_points));
                if (i + 1 < mgr.m_bestRuns.size()) {
                    stream.append(", ");
                }
            }
        }
    } else if (var_name == "best_time") {
        if (pl && pl->m_level && !pl->m_level->isPlatformer()) {
            return false;
        }

        auto& mgr = GDH::Labels::Manager::get();
        if (mgr.m_bestTime <= 0.0) {
            stream.append("N/A");
        } else {
            bool showMS = Config::get().get<bool>("cosmetic.accurate_percentage::showMilliseconds", true);
            stream.append(GDH::Utils::formatTime(mgr.m_bestTime, showMS));
        }
    } else if (var_name == "\\n") {
        stream.append("\n");
    } else {
        return false;
    }
    return true;
}

std::string GDH::Labels::Label::render() const {
    geode::utils::StringBuffer<> stream;
    std::string_view raw = text;
    
    while (!raw.empty()) {
        if (raw[0] == '{') {
            std::string_view::size_type ccurly = raw.find("}");
            if (ccurly == std::string_view::npos) {
                stream.append('{');
                raw.remove_prefix(1);
                continue;
            }
            
            std::string_view var = raw.substr(1, ccurly - 1);
            
            if (!renderVariable(stream, var)) {
                return "";
            }
            
            raw.remove_prefix(ccurly + 1);
        } else {
            stream.append(raw[0]);
            raw.remove_prefix(1);
        }
    }
    return stream.str();
}

void Manager::save() {
    nlohmann::json obj;
    obj["cornerPadding"] = cornerPadding;
    obj["midPadding"] = midPadding;
    for (const auto &[corner, labelsCorner] : labels) {
        std::string cornerIndice(getCornerKey(corner));
        for (const auto &label : labelsCorner) {
            nlohmann::json labelObj;
            labelObj["enabled"] = label.enabled;
            labelObj["type"] = label.type;
            switch (label.type) {
                case LabelType::Text: {
                    labelObj["text"] = label.text;
                    labelObj["color"] = { label.color[0], label.color[1], label.color[2], label.color[3] };
                    labelObj["rainbow"] = label.rainbow;
                    labelObj["cps"] = label.cps;
                    labelObj["noclip"] = label.noclip;
                    labelObj["size"] = label.size;
                } break;
                case LabelType::CheatIndicator: {
                    labelObj["size"] = label.size;
                    labelObj["opacity"] = label.color[3];
                } break;
                case LabelType::Spacing: {
                    labelObj["spacing"] = label.size;
                } break;
            }
            obj[cornerIndice].push_back(labelObj);
        }
    }
    std::ofstream outFile(getLabelsDataPath());
    if (outFile.is_open()) {
        outFile << obj.dump(4);
    }
}

void Manager::load() {
    std::ifstream inFile(getLabelsDataPath());
    if (!inFile.is_open()) return;

    labels.clear();
    labels[Corner::Top_Left] = {};
    labels[Corner::Top_Center] = {};
    labels[Corner::Top_Right] = {};
    labels[Corner::Center_Left] = {};
    labels[Corner::Center_Center] = {};
    labels[Corner::Center_Right] = {};
    labels[Corner::Bottom_Left] = {};
    labels[Corner::Bottom_Center] = {};
    labels[Corner::Bottom_Right] = {};
    
    std::string file_contents((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
    
    nlohmann::json obj = nlohmann::json::parse(file_contents, nullptr, false);
    if (obj.is_discarded()) {
        return;
    }
    
    cornerPadding = obj.value("cornerPadding", 4.0f);
    midPadding = obj.value("midPadding", 3.0f);

    for (auto &[corner, labelsCorner] : labels) {
        std::string cornerIndice(getCornerKey(corner));
        if (!obj.contains(cornerIndice)) continue;

        for (const auto &i : obj[cornerIndice]) {
            bool enabled = i.value("enabled", true);

            switch (i["type"].get<LabelType>()) {
                case LabelType::Text: {
                    std::array<float, 4> color = { i["color"][0], i["color"][1], i["color"][2], i["color"][3] };
                    bool rainbow = i.value("rainbow", false);
                    bool cps = i.value("cps", false);
                    bool noclip = i.value("noclip", false);
                    
                    auto& lbl = labelsCorner.emplace_back(i["text"], color, i["size"], rainbow, cps, noclip);
                    lbl.enabled = enabled;
                } break;
                case LabelType::CheatIndicator: {
                    float opacity = i.value("opacity", 1.0f);
                    auto& lbl = labelsCorner.emplace_back(i["size"].get<float>(), LabelType::CheatIndicator, opacity);
                    lbl.enabled = enabled;
                } break;
                case LabelType::Spacing: {
                    auto& lbl = labelsCorner.emplace_back(i["spacing"].get<float>());
                    lbl.enabled = enabled;
                } break;
            }
        }
    }
}

class $modify(LabelsPlayLayer, PlayLayer) {
    struct Fields {
        cocos2d::CCMenu* labelsMenu = nullptr;
        std::unordered_map<Corner, std::vector<geode::Label*>> labelObjs;
        float startPercent = 0.0f;

        ~Fields() {
            GDH::Labels::Manager::get().resetBestRun();
        }
    };

    static void onModify(auto& self) {
        (void) self.setHookPriority("PlayLayer::destroyPlayer", -35);
    }

    bool init(GJGameLevel *level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto fields = m_fields.self();

        fields->labelsMenu = cocos2d::CCMenu::create();
        fields->labelsMenu->setPosition({0, 0});
        fields->labelsMenu->setID("labels-menu"_spr);
        if (m_uiLayer) {
            m_uiLayer->addChild(fields->labelsMenu, 999);
        }

        fields->labelObjs[Corner::Top_Left] = {};
        fields->labelObjs[Corner::Top_Center] = {};
        fields->labelObjs[Corner::Top_Right] = {};
        fields->labelObjs[Corner::Center_Left] = {};
        fields->labelObjs[Corner::Center_Center] = {};
        fields->labelObjs[Corner::Center_Right] = {};
        fields->labelObjs[Corner::Bottom_Left] = {};
        fields->labelObjs[Corner::Bottom_Center] = {};
        fields->labelObjs[Corner::Bottom_Right] = {};

        g_sessionStart = std::chrono::steady_clock::now();
        fields->startPercent = GDH::Utils::getRealProgress(this);

        return true;
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (m_anticheatSpike && obj == m_anticheatSpike)
            return PlayLayer::destroyPlayer(player, obj);

        auto fields = m_fields.self();
        if (!m_isPracticeMode && (!m_level || !m_level->isPlatformer())) {
            float currentEnd = GDH::Utils::getRealProgress(this);
            GDH::Labels::Manager::get().updateBestRun(fields->startPercent, currentEnd);
        }

        auto& config = Config::get();
        if (config.get<bool>("core.noclip", false))
            NoclipAccuracy::get().handle_death();

        PlayLayer::destroyPlayer(player, obj);
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        CpsCounter::get().reset();
        NoclipAccuracy::get().handle_reset();

        auto fields = m_fields.self();
        auto& mgr = GDH::Labels::Manager::get();

        if (!m_level || !m_level->isPlatformer()) {
            fields->startPercent = GDH::Utils::getRealProgress(this);
        }
    }

    void levelComplete() {
        auto fields = m_fields.self();

        if (!m_isPracticeMode) {
            if (m_level && m_level->isPlatformer()) {
                GDH::Labels::Manager::get().updateBestTime(m_timePlayed);
            } else {
                GDH::Labels::Manager::get().updateBestRun(fields->startPercent, 100.0f);
            }
        }
        PlayLayer::levelComplete();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        g_rainbowHue += 0.125f * dt;
        if (g_rainbowHue > 1.0f) g_rainbowHue -= 1.0f;

        CpsCounter::get().update(dt);
    }
    
    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        
        auto& config = Config::get();
        auto& manager = GDH::Labels::Manager::get();
        auto fields = m_fields.self();

        bool disable_all = config.get<bool>("labels::disable_all", false);
        bool noclip_enabled = config.get<bool>("core.noclip", false);
        bool safe_mode = config.get<bool>("core.safe_mode", false);
        bool auto_safe_mode = config.get<bool>("level.auto_safe_mode", false);

        bool indicate_when_cheating = config.get<bool>("level.cheat_indicator::indicate_when_cheating", false);
        bool is_cheating = manager.m_cheatedInCurrentAttempt;

        bool cheat_indicator_enabled = config.get<bool>("level.cheat_indicator", false) && (!indicate_when_cheating || is_cheating || safe_mode);

        static bool was_disabled = false;

        if (disable_all != was_disabled) {
            was_disabled = disable_all;
            for (auto &[corner, labelVec] : fields->labelObjs) {
                for (auto* label : labelVec) {
                    if (label) label->setVisible(!disable_all);
                }
            }
        }

        if (disable_all) return;

        auto windowSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
        
        for (auto &[corner, labelVec] : fields->labelObjs) {
            float y = 0.0f, labelsHeight = 0.0f; int i = 0;
            auto cornerProps = getCornerProps(corner);

            for (const auto &labelObj : manager.labels[corner]) {
                if (!labelObj.enabled) continue;
                if (labelObj.noclip && !noclip_enabled) continue;

                if (labelObj.type == LabelType::Text) {
                    std::string renderedText = labelObj.render();
                    if (renderedText.empty()) continue; 

                    geode::Label *label = nullptr;
                    if (i < labelVec.size()) {
                        label = labelVec[i];
                    } else {
                        label = geode::Label::create("", "bigFont.fnt");
                        label->setAnchorPoint(cornerProps.anchor);
                        label->setAlignment(cornerProps.align);
                        if (fields->labelsMenu) {
                            fields->labelsMenu->addChild(label);
                        }
                        labelVec.push_back(label);
                    }

                    i++;
                
                    label->setScale(labelObj.size);
                    label->setText(renderedText);

                    float labelWidth = label->getContentWidth() * labelObj.size;
                    float maxWidth = windowSize.width - (manager.cornerPadding * 2.0f);
                    if (labelWidth > maxWidth && labelWidth > 0.0f) {
                        float autoScale = (maxWidth / labelWidth) * labelObj.size;
                        label->setScale(autoScale);
                    }

                    if (labelObj.rainbow) {
                        float r, g, b; GDH::Utils::hsvToRgb(g_rainbowHue, 1.0f, 1.0f, r, g, b);
                        label->setColor({static_cast<GLubyte>(r * 255), static_cast<GLubyte>(g * 255), static_cast<GLubyte>(b * 255)});
                    } 
                    else if (labelObj.cps) {
                        bool isPressed = false;

                        if (labelObj.text.find("p2::") != std::string::npos) {
                            isPressed = CpsCounter::get().p2_push;
                        } else if (labelObj.text.find("p1::") != std::string::npos) {
                            isPressed = CpsCounter::get().p1_push;
                        } else {
                            isPressed = CpsCounter::get().p1_push || CpsCounter::get().p2_push;
                        }

                        if (isPressed) {
                            label->setColor({0, 255, 0});
                        } else {
                            label->setColor({
                                static_cast<GLubyte>(labelObj.color[0] * 255),
                                static_cast<GLubyte>(labelObj.color[1] * 255),
                                static_cast<GLubyte>(labelObj.color[2] * 255)
                            });
                        }
                    }
                    else if (labelObj.noclip) {
                        NoclipAccuracy::get().prevDied ? label->setColor({255, 0, 0}) 
                        : label->setColor({static_cast<GLubyte>(labelObj.color[0] * 255),
                            static_cast<GLubyte>(labelObj.color[1] * 255),static_cast<GLubyte>(labelObj.color[2] * 255)});
                    }
                    else {
                        label->setColor({static_cast<GLubyte>(labelObj.color[0] * 255),
                            static_cast<GLubyte>(labelObj.color[1] * 255),static_cast<GLubyte>(labelObj.color[2] * 255)});
                    }
                    label->setOpacity(static_cast<GLubyte>(labelObj.color[3] * 255));

                    y += label->getContentHeight() * label->getScaleY() + manager.midPadding;
                } 
                else if (labelObj.type == LabelType::CheatIndicator) {
                    if (!cheat_indicator_enabled) continue;
                    
                    geode::Label *label = nullptr;
                    if (i < labelVec.size()) {
                        label = labelVec[i];
                    } else {
                        label = geode::Label::create(".", "bigFont.fnt");
                        label->setAnchorPoint(cornerProps.anchor);
                        label->setAlignment(cornerProps.align);
                        if (fields->labelsMenu) {
                            fields->labelsMenu->addChild(label);
                        }
                        labelVec.push_back(label);
                    }

                    i++;

                    label->setScale(labelObj.size);
                    label->setText(".");
                    label->setContentHeight(12.5f);

                    if (safe_mode) {
                        label->setColor({255, 128, 0});
                    } 
                    else if (is_cheating) {
                        if (auto_safe_mode)
                            label->setColor({255, 128, 0});
                        else
                            label->setColor({255, 0, 0});
                    } 
                    else {
                        label->setColor({0, 255, 0});
                    }

                    label->setOpacity(static_cast<GLubyte>(labelObj.color[3] * 255));

                    y += (label->getContentHeight()) * labelObj.size + manager.midPadding;
                } 
                else if (labelObj.type == LabelType::Spacing) {
                    y += labelObj.size + manager.midPadding;
                }
            }

            for (; i < labelVec.size(); i++) {
                labelVec[i]->setOpacity(0);
            }

            labelsHeight = y - manager.midPadding;
            y = 0.0f;
            i = 0;

            for (const auto &labelObj : manager.labels[corner]) {
                if (!labelObj.enabled) continue;
                if (labelObj.noclip && !noclip_enabled) continue;
                
                if (labelObj.type == LabelType::Text) {
                    if (labelObj.render().empty()) continue; 
                    geode::Label *label = labelVec[i++];
                    label->setPosition(getCornerPos(corner, manager.cornerPadding, windowSize, y, labelsHeight));
                    y += label->getContentHeight() * label->getScaleY() + manager.midPadding;
                } 
                else if (labelObj.type == LabelType::CheatIndicator) {
                    if (!cheat_indicator_enabled) continue;
                    geode::Label *label = labelVec[i++];
                    label->setPosition(getCornerPos(corner, manager.cornerPadding, windowSize, y, labelsHeight));
                    y += (label->getContentHeight()) * labelObj.size + manager.midPadding;
                } 
                else if (labelObj.type == LabelType::Spacing) {
                    y += labelObj.size + manager.midPadding;
                }
            }
        }
    }
};

class $modify(LabelsGJBaseGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        GJBaseGameLayer::handleButton(down, button, isPlayer1);

        if (button == 1) {
            bool swapControls = GameManager::sharedState()->getGameVariable("0010");
            bool isDualAnd2P = m_gameState.m_isDualMode && m_level && m_level->m_twoPlayerMode;
            bool isP1 = isDualAnd2P ? (swapControls ? !isPlayer1 : isPlayer1) : true;

            CpsCounter::get().click(down, isP1);
        }
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {       
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
        NoclipAccuracy::get().handle_update(this, dt);
    }
};

class $modify(FPSCounterCCScheduler, cocos2d::CCScheduler) {
    static void onModify(auto& self) {
        (void) self.setHookPriority("cocos2d::CCScheduler::update", geode::Priority::First); 
    }

    void update(float dt) {
        CCScheduler::update(dt);
        GDH::Utils::getFps();
    }
};