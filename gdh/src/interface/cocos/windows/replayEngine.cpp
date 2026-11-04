#include <Geode/Geode.hpp>
#include "../hacks_tab.hpp"
#include "../../../core/config.hpp"
#include "../../../core/gui.hpp"
#include "../../../core/utils.hpp"
#include "../../../core/replayEngine.hpp"
#include "../../../core/recorder/recorder.hpp"

class LabelInfoScheduler : public geode::Label {
public:
    void customUpdate(float dt) {
        auto& engine = GDH::ReplayEngine::get();
        
        this->setText(fmt::format(
            "Frame: {}\nReplay Size: {}/{}", 
            engine.get_frame(), 
            engine.get_current_index(), 
            engine.get_actions_size()
        ));
    }
};

class RecorderLayer : public geode::Popup {
protected:
    geode::TextInput* nameInput;
    geode::TextInput* widthInput;
    geode::TextInput* heightInput;
    geode::TextInput* fpsInput;
    geode::TextInput* bitrateInput;

    bool tryStartRecorder(const std::filesystem::path& path) {
        auto& recorder = GDH::Recorder::get();
        if (!GDH::Utils::isOnlyAsciiPath(path)) {
            recorder.enabled = false;
            GDH::MaterialLayer(FLAlertLayer::create(
                "Recorder",
                "Invalid path. Please remove any Cyrillic characters.",
                "OK"
            ))->show();
            return false;
        }
        Config::get().set<std::string>("recorder::path", path.string());
        recorder.start();
        return true;
    }

    bool init() {
        if (!geode::Popup::init(360.f, 230.f, "GDH_square.png"_spr)) return false;

        auto& recorder = GDH::Recorder::get();
        auto winSize = m_mainLayer->getContentSize();

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto title = geode::Label::create("Recorder Settings", "GoogleSans.fnt"_spr);
        title->setPosition({winSize.width / 2, winSize.height - 16.f});
        title->setScale(0.7f);
        m_mainLayer->addChild(title);

        auto closeBtn = CCSprite::create("GDH_closeBtn.png"_spr);
        closeBtn->setScale(0.75f);
        m_closeBtn->setSprite(closeBtn);

        auto createLabel = [&menu](const std::string& text, const cocos2d::CCPoint& position, float scale = 0.6f) {
            auto label = geode::Label::create(text, "GoogleSans.fnt"_spr);
            label->setAnchorPoint({0.f, 0.5f});
            label->setPosition(position);
            label->setScale(scale);
            menu->addChild(label);
            return label;
        };

        auto recordToggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f,
            [this, &recorder](CCMenuItemToggler* sender) {
            auto pl = PlayLayer::get();
            recorder.enabled = !sender->isOn();

            if (pl && pl->m_hasCompletedLevel) {
                GDH::MaterialLayer(FLAlertLayer::create("Recorder", "Restart level to start recording", "OK"))->show();
                recorder.enabled = false;
                sender->toggle(true);
                return;
            }

            if (!recorder.enabled) {
                recorder.stop();
                return;
            }
            
            auto showError = [](const char* msg) {
                GDH::MaterialLayer(FLAlertLayer::create("Recorder", msg, "OK"))->show();
            };

            std::string question =
                "Do you want to set a custom directory to save the showcase?\n\n"
                "Yes - Select folder manually\n"
                "No  - Keep the default directory";

            auto popup = geode::createQuickPopup(
                "Recorder",
                question,
                "No", "Yes",
                [&](auto, bool btn2) {
                    if (btn2) {
                        geode::async::spawn(
                            geode::utils::file::pick(
                                geode::utils::file::PickMode::OpenFolder, {}
                            ),
                            [&](geode::utils::file::PickResult res) {
                                if (!res) return;
                                auto pathOpt = std::move(res).unwrapOr(std::nullopt);
                                if (!pathOpt) return;
                                tryStartRecorder(std::move(*pathOpt));
                            }
                        );
                    } else {
                        tryStartRecorder(getFolderShowcasesPath());
                    }
                }
            );

            GDH::MaterialLayer(popup);
        });
        
        recordToggle->toggle(recorder.enabled);
        recordToggle->setPosition({45.f, 175.f});
        menu->addChild(recordToggle);
        
        createLabel("Record", {recordToggle->getPositionX() - 10.f + recordToggle->getScaledContentWidth(), recordToggle->getPositionY()}, 0.7f);

        auto includeAudioToggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f,
            [this, &recorder](CCMenuItemToggler* sender) {
            recorder.record_audio = !sender->isOn();
        });
        
        includeAudioToggle->toggle(recorder.record_audio);
        includeAudioToggle->setPosition({153.f, 175.f});
        menu->addChild(includeAudioToggle);
        
        createLabel("Audio", {includeAudioToggle->getPositionX() - 10.f + includeAudioToggle->getScaledContentWidth(), includeAudioToggle->getPositionY()}, 0.7f);

        auto ultraFastToggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f,
            [this, &recorder](CCMenuItemToggler* sender) {
            recorder.ultrafast = !sender->isOn();
        });
        
        ultraFastToggle->toggle(recorder.ultrafast);
        ultraFastToggle->setPosition({260.f, 175.f});
        menu->addChild(ultraFastToggle);
        
        createLabel("Ultrafast", {ultraFastToggle->getPositionX() - 10.f + ultraFastToggle->getScaledContentWidth(), ultraFastToggle->getPositionY()}, 0.7f);

        createLabel("Video Name:", {30.f, 140.f});
        nameInput = geode::TextInput::create(170.f, "video_name", "GoogleSans.fnt"_spr);
        nameInput->setPosition({245.f, 140.f});
        nameInput->setString(recorder.videoName);
        nameInput->setFilter("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.");
        nameInput->setCallback([&recorder](const std::string& text) { recorder.videoName = text; });
        menu->addChild(nameInput);

        createLabel("Resolution / FPS:", {30.f, 105.f});

        widthInput = geode::TextInput::create(45.f, "W", "GoogleSans.fnt"_spr);
        widthInput->setPosition({182.f, 105.f}); 
        widthInput->setString(std::to_string(recorder.width));
        widthInput->setFilter("1234567890");
        widthInput->setCallback([&recorder](const std::string& text) { 
            auto value = geode::utils::numFromString<int>(text);
            if (!value.isErr()) recorder.width = value.unwrap();
        });
        menu->addChild(widthInput);

        createLabel("x", {210.f, 105.f}, 0.5f);

        heightInput = geode::TextInput::create(45.f, "H", "GoogleSans.fnt"_spr);
        heightInput->setPosition({240.f, 105.f});
        heightInput->setString(std::to_string(recorder.height));
        heightInput->setFilter("1234567890");
        heightInput->setCallback([&recorder](const std::string& text) { 
            auto value = geode::utils::numFromString<int>(text);
            if (!value.isErr()) recorder.height = value.unwrap();
        });
        menu->addChild(heightInput);

        createLabel("@", {267.f, 105.f}, 0.5f);

        fpsInput = geode::TextInput::create(40.f, "FPS", "GoogleSans.fnt"_spr);
        fpsInput->setPosition({300.f, 105.f});
        fpsInput->setString(std::to_string(recorder.fps));
        fpsInput->setFilter("1234567890");
        fpsInput->setCallback([&recorder](const std::string& text) { 
            auto value = geode::utils::numFromString<int>(text);
            if (!value.isErr()) recorder.fps = value.unwrap();
        });
        menu->addChild(fpsInput);

        createLabel("Bitrate:", {30.f, 70.f});
        bitrateInput = geode::TextInput::create(80.f, "e.g. 50M", "GoogleSans.fnt"_spr);
        bitrateInput->setPosition({200.f, 70.f});
        bitrateInput->setFilter("0123456789MKmk");
        bitrateInput->setString(recorder.bitrate);
        bitrateInput->setCallback([&recorder](const std::string& text) { recorder.bitrate = text; });
        menu->addChild(bitrateInput);

        auto createPresetBtn = [this, &recorder](const std::string& text, float x, int w, int h, int fps, const std::string& bitrate) {
            auto spr = ButtonSprite::create(text.c_str(), 45, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 25.f, 0.4f);
            auto btn = CCMenuItemExt::createSpriteExtra(spr, [this, &recorder, w, h, fps, bitrate](auto) {
                recorder.width = w;
                recorder.height = h;
                recorder.fps = fps;
                recorder.bitrate = bitrate;

                widthInput->setString(std::to_string(w));
                heightInput->setString(std::to_string(h));
                fpsInput->setString(std::to_string(fps));
                bitrateInput->setString(bitrate);
            });
            btn->setPosition({x, 30.f});
            return btn;
        };

        menu->addChild(createPresetBtn("HD", 60.f, 1280, 720, 60, "25M"));
        menu->addChild(createPresetBtn("FULL HD", 140.f, 1920, 1080, 60, "50M"));
        menu->addChild(createPresetBtn("2K", 220.f, 2560, 1440, 60, "70M"));
        menu->addChild(createPresetBtn("4K", 300.f, 3840, 2160, 60, "80M"));

        auto applyStyle = [](geode::TextInput* input) {
            if (auto* bg = input->getChildByType<geode::NineSlice>(0)) { 
                bg->setColor({71, 71, 131}); 
                bg->setOpacity(255); 
            }
        };
        applyStyle(nameInput);
        applyStyle(widthInput);
        applyStyle(heightInput);
        applyStyle(fpsInput);
        applyStyle(bitrateInput);

        return true;
    }
public:
    static RecorderLayer* create() {
        auto ret = new RecorderLayer();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class ReplaySelectLayer : public geode::Popup {
protected:
    geode::TextInput* input;

    bool init() {
        if (!geode::Popup::init(300.f, 220.f, "GDH_square.png"_spr)) return false;

        auto winSize = m_mainLayer->getContentSize();

        auto title = geode::Label::create("Select Replay", "GoogleSans.fnt"_spr);
        title->setPosition({winSize.width / 2, winSize.height - 18.f});
        title->setScale(0.7f);
        m_mainLayer->addChild(title);

        auto closeBtn = CCSprite::create("GDH_closeBtn.png"_spr);
        closeBtn->setScale(0.75f);
        m_closeBtn->setSprite(closeBtn);

        auto scroll = ScrollLayer::create({600.f, 180.f});
        scroll->setPosition({winSize.width / 2 - 300.f, 10.f});
        scroll->m_contentLayer->setLayout(
            ColumnLayout::create()
                ->setAutoScale(false)
                ->setAxisReverse(true)
                ->setAutoGrowAxis(true)
                ->setGap(5.f)
        );
        m_mainLayer->addChild(scroll);

        scroll->m_contentLayer->addChild(CCNode::create());

        for (const auto& entry : std::filesystem::directory_iterator(getFolderMacroPath())) {
            if (entry.is_regular_file() && entry.path().extension() == ".re4") {
                if (GDH::Utils::isOnlyAsciiPath(entry)) {
                    std::string macroName = entry.path().stem().string();
                    std::filesystem::path macroPath = entry.path();
    
                    auto menu = CCMenu::create();
                    menu->setContentSize({600.f, 25.f});
    
                    auto btnSprite = ButtonSprite::create(macroName.c_str(), 180, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 26.f, 0.5f);
                    auto btn = CCMenuItemExt::createSpriteExtra(btnSprite, [this, macroPath](auto) {
                        auto& engine = GDH::ReplayEngine::get();
    
                        engine.replay_name = macroPath.stem().string();
                        input->setString(engine.replay_name);
    
                        this->m_closeBtn->activate();
                    });
    
                    btn->setPosition({300.f, 12.5f});
                    menu->addChild(btn);
                    scroll->m_contentLayer->addChild(menu);
                }
            }
        }

        scroll->m_contentLayer->addChild(CCNode::create());

        scroll->m_contentLayer->updateLayout();
        scroll->updateLayout();
        scroll->moveToTop();
        return true;
    }
public:
    static ReplaySelectLayer* create(geode::TextInput* textInput) {
        auto ret = new ReplaySelectLayer();
        if (ret->init()) { ret->autorelease(); ret->input = textInput; return ret; }
        delete ret;
        return nullptr;
    }
};

class MacroEditorLayer : public geode::Popup {
protected:
    static const int page_size = 6;
    
    int m_frameType = 1;
    int m_selectedIndex = -1;
    int m_page = 0;
    std::string m_goToValue = "";
    cocos2d::CCNode* m_contentLayer = nullptr;

    bool init() {
        if (!geode::Popup::init(410.f, 300.f, "GDH_square.png"_spr)) return false;

        auto closeBtn = CCSprite::create("GDH_closeBtn.png"_spr);
        closeBtn->setScale(0.75f);
        m_closeBtn->setSprite(closeBtn);

        m_contentLayer = cocos2d::CCNode::create();
        m_contentLayer->setContentSize(m_mainLayer->getContentSize());
        m_mainLayer->addChild(m_contentLayer);

        refresh();
        return true;
    }

    template <typename Frames>
    static int pageCount(const Frames& frames) {
        return std::max(1, (int)((frames.size() + page_size - 1) / page_size));
    }

    void refresh() {
        m_contentLayer->removeAllChildren();

        auto& engine = GDH::ReplayEngine::get();

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_contentLayer->addChild(menu);

        const char* typeNames[] = { "Physic Frames", "Input Frames" };

        auto typeLeftArrow = CCSprite::create("GDH_arrow.png"_spr);
        typeLeftArrow->setScale(0.55f);
        auto typeLeftBtn = CCMenuItemExt::createSpriteExtra(typeLeftArrow, [this](auto) {
            m_frameType = m_frameType == 0 ? 1 : 0;
            m_selectedIndex = -1;
            m_page = 0;
            refresh();
        });
        typeLeftBtn->setPosition({25.f, 275.f});
        menu->addChild(typeLeftBtn);

        auto typeLabel = geode::Label::create(typeNames[m_frameType], "GoogleSans.fnt"_spr);
        typeLabel->setScale(0.5f);
        typeLabel->setPosition({100.f, 275.f});
        menu->addChild(typeLabel);

        auto typeRightArrow = CCSprite::create("GDH_arrow.png"_spr);
        typeRightArrow->setScale(0.55f);
        typeRightArrow->setFlipX(true);
        auto typeRightBtn = CCMenuItemExt::createSpriteExtra(typeRightArrow, [this](auto) {
            m_frameType = m_frameType == 0 ? 1 : 0;
            m_selectedIndex = -1;
            m_page = 0;
            refresh();
        });
        typeRightBtn->setPosition({175.f, 275.f});
        menu->addChild(typeRightBtn);

        if (m_frameType == 0) {
            buildPhysicPage(menu, engine);
        } else {
            buildInputPage(menu, engine);
        }
    }

    geode::TextInput* createStyledInput(float width, const std::string& placeholder, const std::string& value, const std::string& filter, float x, float y, std::function<void(const std::string&)> cb) {
        auto input = geode::TextInput::create(width, placeholder, "GoogleSans.fnt"_spr);
        if (auto* bg = input->getChildByType<geode::NineSlice>(0)) {
            bg->setColor({71, 71, 131});
            bg->setOpacity(255);
        }
        input->setPosition({x, y});
        input->setString(value);
        if (!filter.empty()) input->setFilter(filter);
        input->setCallback(cb);
        m_contentLayer->addChild(input);
        return input;
    }

    void buildPhysicPage(CCMenu* menu, GDH::ReplayEngine& engine) {
        auto& frames = engine.get_physic_frames();

        if (frames.empty()) m_selectedIndex = -1;
        else if (m_selectedIndex >= (int)frames.size()) m_selectedIndex = (int)frames.size() - 1;

        int totalPages = pageCount(frames);
        m_page = std::clamp(m_page, 0, totalPages - 1);

        if (!frames.empty()) {
            auto headerLabel = geode::Label::create("Sel                    Frame                    Player                          X                                          Y                                   YAccel", "GoogleSans.fnt"_spr);
            headerLabel->setAnchorPoint({0.f, 0.5f});
            headerLabel->setScale(0.4f);
            headerLabel->setPosition({25.f, 253.f});
            headerLabel->setColor({180, 180, 220});
            menu->addChild(headerLabel);
        }

        int base = m_page * page_size;

        for (int row = 0; row < page_size; row++) {
            int idx = base + row;
            if (idx >= (int)frames.size()) break;
            auto& frame = frames[idx];
            bool isSel = (idx == m_selectedIndex);

            float rowY = 230.f - (row * 35.f);

            auto rowMenu = CCMenu::create();
            rowMenu->setContentSize({430.f, 30.f});
            rowMenu->setPosition({15.f, rowY});
            rowMenu->setAnchorPoint({0.f, 0.5f});
            rowMenu->setLayout(
                RowLayout::create()
                    ->setAxisAlignment(AxisAlignment::Start)
                    ->setCrossAxisAlignment(AxisAlignment::Center)
                    ->setGap(8.f)
            );

            auto selSpr = ButtonSprite::create(">", 18, true, "GoogleSans.fnt"_spr, isSel ? "GDH_button_02.png"_spr : "GDH_button_01.png"_spr, 22.f, 0.4f);
            selSpr->m_label->setColor(isSel ? ccColor3B({33, 33, 78}) : ccColor3B({255, 255, 255}));
            auto selBtn = CCMenuItemExt::createSpriteExtra(selSpr, [this, idx](auto) {
                m_selectedIndex = idx;
                refresh();
            });
            rowMenu->addChild(selBtn);

            auto frameInput = createStyledInput(55.f, "Frame", std::to_string(frame.frame), "0123456789", 0, 0, [&frame](const std::string& text) {
                if (auto val = geode::utils::numFromString<int>(text)) frame.frame = static_cast<uint64_t>(val.unwrap());
            });

            frameInput->removeFromParent();
            rowMenu->addChild(frameInput);

            auto pBtnSpr = ButtonSprite::create(frame.isPlayer2 ? "P2" : "P1", 24, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 22.f, 0.4f);
            auto pBtn = CCMenuItemExt::createSpriteExtra(pBtnSpr, [this, &frame](auto) {
                frame.isPlayer2 = !frame.isPlayer2;
                refresh();
            });
            rowMenu->addChild(pBtn);

            auto xInput = createStyledInput(70.f, "X", fmt::format("{:.6f}", frame.x), "-0123456789.", 0, 0, [&frame](const std::string& text) {
                if (auto val = geode::utils::numFromString<float>(text)) frame.x = val.unwrap();
            });
            xInput->removeFromParent();
            rowMenu->addChild(xInput);

            auto yInput = createStyledInput(70.f, "Y", fmt::format("{:.6f}", frame.y), "-0123456789.", 0, 0, [&frame](const std::string& text) {
                if (auto val = geode::utils::numFromString<float>(text)) frame.y = val.unwrap();
            });
            yInput->removeFromParent();
            rowMenu->addChild(yInput);

            auto yAccelInput = createStyledInput(70.f, "YAccel", fmt::format("{:.6f}", frame.y_accel), "-0123456789.", 0, 0, [&frame](const std::string& text) {
                if (auto val = geode::utils::numFromString<double>(text)) frame.y_accel = static_cast<double>(val.unwrap());
            });
            yAccelInput->removeFromParent();
            rowMenu->addChild(yAccelInput);

            rowMenu->updateLayout();
            m_contentLayer->addChild(rowMenu);
        }

        if (frames.empty()) {
            auto emptyLabel = geode::Label::create("No physic frames", "GoogleSans.fnt"_spr);
            emptyLabel->setScale(0.45f);
            emptyLabel->setOpacity(120);
            emptyLabel->setPosition({205.f, 150.f});
            menu->addChild(emptyLabel);
        }

        buildBottomControls(menu, frames, [this, &frames]() {
            physic_data newFrame{};
            if (!frames.empty() && m_selectedIndex >= 0 && m_selectedIndex < (int)frames.size()) {
                newFrame.frame = frames[m_selectedIndex].frame + 1;
                newFrame.isPlayer2 = frames[m_selectedIndex].isPlayer2;
                frames.insert(frames.begin() + m_selectedIndex + 1, newFrame);
                m_selectedIndex++;
            } else {
                newFrame.frame = 0;
                frames.push_back(newFrame);
                m_selectedIndex = (int)frames.size() - 1;
            }
            m_page = m_selectedIndex / page_size;
        });
    }

    void buildInputPage(CCMenu* menu, GDH::ReplayEngine& engine) {
        auto& frames = engine.get_input_frames();

        if (frames.empty()) m_selectedIndex = -1;
        else if (m_selectedIndex >= (int)frames.size()) m_selectedIndex = (int)frames.size() - 1;

        int totalPages = pageCount(frames);
        m_page = std::clamp(m_page, 0, totalPages - 1);

        if (!frames.empty()) {
            auto headerLabel = geode::Label::create("Sel                         Frame                         Player                            Button                                    Action", "GoogleSans.fnt"_spr);
            headerLabel->setAnchorPoint({0.f, 0.5f});
            headerLabel->setScale(0.4f);
            headerLabel->setPosition({25.f, 253.f});
            headerLabel->setColor({180, 180, 220});
            menu->addChild(headerLabel);
        }

        int base = m_page * page_size;

        for (int row = 0; row < page_size; row++) {
            int idx = base + row;
            if (idx >= (int)frames.size()) break;
            auto& frame = frames[idx];
            bool isSel = (idx == m_selectedIndex);

            float rowY = 230.f - (row * 35.f);

            auto rowMenu = CCMenu::create();
            rowMenu->setContentSize({400.f, 30.f});
            rowMenu->setPosition({15.f, rowY});
            rowMenu->setAnchorPoint({0.f, 0.5f});
            rowMenu->setLayout(
                RowLayout::create()
                    ->setAxisAlignment(AxisAlignment::Start)
                    ->setCrossAxisAlignment(AxisAlignment::Center)
                    ->setGap(12.f)
            );

            auto selSpr = ButtonSprite::create(">", 18, true, "GoogleSans.fnt"_spr, isSel ? "GDH_button_02.png"_spr : "GDH_button_01.png"_spr, 22.f, 0.4f);
            selSpr->m_label->setColor(isSel ? ccColor3B({33, 33, 78}) : ccColor3B({255, 255, 255}));
            auto selBtn = CCMenuItemExt::createSpriteExtra(selSpr, [this, idx](auto) {
                m_selectedIndex = idx;
                refresh();
            });
            rowMenu->addChild(selBtn);

            auto frameInput = createStyledInput(65.f, "Frame", std::to_string(frame.frame), "0123456789", 0, 0, [&frame](const std::string& text) {
                if (auto val = geode::utils::numFromString<int>(text)) frame.frame = static_cast<uint64_t>(val.unwrap());
            });
            frameInput->removeFromParent();
            rowMenu->addChild(frameInput);

            auto pBtnSpr = ButtonSprite::create(frame.isPlayer2 ? "P2" : "P1", 28, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 22.f, 0.4f);
            auto pBtn = CCMenuItemExt::createSpriteExtra(pBtnSpr, [this, &frame](auto) {
                frame.isPlayer2 = !frame.isPlayer2;
                refresh();
            });
            rowMenu->addChild(pBtn);

            const char* btnName = frame.button == 1 ? "Jump" : frame.button == 2 ? "Left" : frame.button == 3 ? "Right" : "Unknown";
            auto btnSpr = ButtonSprite::create(btnName, 60, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 22.f, 0.4f);
            auto btnItem = CCMenuItemExt::createSpriteExtra(btnSpr, [this, &frame](auto) {
                frame.button = frame.button < 3 ? frame.button + 1 : 1;
                refresh();
            });
            rowMenu->addChild(btnItem);

            auto actSpr = ButtonSprite::create(frame.down ? "Push" : "Release", 60, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 22.f, 0.4f);
            auto actBtn = CCMenuItemExt::createSpriteExtra(actSpr, [this, &frame](auto) {
                frame.down = !frame.down;
                refresh();
            });
            rowMenu->addChild(actBtn);

            rowMenu->updateLayout();
            m_contentLayer->addChild(rowMenu);
        }

        if (frames.empty()) {
            auto emptyLabel = geode::Label::create("No input frames", "GoogleSans.fnt"_spr);
            emptyLabel->setScale(0.45f);
            emptyLabel->setOpacity(120);
            emptyLabel->setPosition({205.f, 150.f});
            menu->addChild(emptyLabel);
        }

        buildBottomControls(menu, frames, [this, &frames]() {
            input_data newFrame{};
            newFrame.button = 1;
            if (!frames.empty() && m_selectedIndex >= 0 && m_selectedIndex < (int)frames.size()) {
                newFrame.frame = frames[m_selectedIndex].frame + 1;
                newFrame.isPlayer2 = frames[m_selectedIndex].isPlayer2;
                frames.insert(frames.begin() + m_selectedIndex + 1, newFrame);
                m_selectedIndex++;
            } else {
                newFrame.frame = 0;
                frames.push_back(newFrame);
                m_selectedIndex = (int)frames.size() - 1;
            }
            m_page = m_selectedIndex / page_size;
        });
    }

    template <typename Frames>
    void buildBottomControls(CCMenu* menu, Frames& frames, std::function<void()> makeNew) {
        int totalPages = pageCount(frames);

        auto prevArrow = CCSprite::create("GDH_arrow.png"_spr);
        prevArrow->setScale(0.45f);
        auto prevBtn = CCMenuItemExt::createSpriteExtra(prevArrow, [this](auto) {
            if (m_page > 0) { m_page--; refresh(); }
        });
        prevBtn->setPosition({20.f, 18.f});
        prevBtn->setEnabled(m_page > 0);
        if (m_page <= 0) prevBtn->setOpacity(100);
        menu->addChild(prevBtn);

        auto pageLabel = geode::Label::create(fmt::format("{}/{}", m_page + 1, totalPages), "GoogleSans.fnt"_spr);
        pageLabel->setScale(0.4f);
        pageLabel->setPosition({50.f, 18.f});
        menu->addChild(pageLabel);

        auto nextArrow = CCSprite::create("GDH_arrow.png"_spr);
        nextArrow->setScale(0.45f);
        nextArrow->setFlipX(true);
        auto nextBtn = CCMenuItemExt::createSpriteExtra(nextArrow, [this, totalPages](auto) {
            if (m_page < totalPages - 1) { m_page++; refresh(); }
        });
        nextBtn->setPosition({80.f, 18.f});
        nextBtn->setEnabled(m_page < totalPages - 1);
        if (m_page >= totalPages - 1) nextBtn->setOpacity(100);
        menu->addChild(nextBtn);

        createStyledInput(60.f, "Go Frame", m_goToValue, "0123456789", 315.f, 275.f, [this](const std::string& text) {
            m_goToValue = text;
        });

        auto goBtnSpr = ButtonSprite::create("Go", 35, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 22.f, 0.4f);
        auto goBtn = CCMenuItemExt::createSpriteExtra(goBtnSpr, [this, &frames](auto) {
            if (auto val = geode::utils::numFromString<uint64_t>(m_goToValue)) {
                uint64_t target = val.unwrap();
                for (size_t i = 0; i < frames.size(); i++) {
                    if (frames[i].frame >= target) {
                        m_selectedIndex = (int)i;
                        m_page = (int)(i / page_size);
                        refresh();
                        break;
                    }
                }
            }
        });
        goBtn->setPosition({375.f, 275.f});
        menu->addChild(goBtn);

        auto addSpr = ButtonSprite::create("Add", 55, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 22.f, 0.4f);
        auto addBtn = CCMenuItemExt::createSpriteExtra(addSpr, [this, makeNew](auto) {
            makeNew();
            refresh();
        });
        addBtn->setPosition({140.f, 18.f});
        menu->addChild(addBtn);

        bool hasSel = !frames.empty() && m_selectedIndex >= 0 && m_selectedIndex < (int)frames.size();

        auto remSpr = ButtonSprite::create("Remove", 65, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 22.f, 0.4f);
        if (!hasSel) remSpr->setColor({150, 150, 150});
        auto remBtn = CCMenuItemExt::createSpriteExtra(remSpr, [this, &frames](auto) {
            frames.erase(frames.begin() + m_selectedIndex);
            if (frames.empty()) m_selectedIndex = -1;
            else if (m_selectedIndex >= (int)frames.size()) m_selectedIndex = (int)frames.size() - 1;
            refresh();
        });
        remBtn->setPosition({220.f, 18.f});
        remBtn->setEnabled(hasSel);
        menu->addChild(remBtn);

        auto upSpr = ButtonSprite::create("Up", 45, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 22.f, 0.4f);
        if (!hasSel || m_selectedIndex <= 0) upSpr->setColor({150, 150, 150});
        auto upBtn = CCMenuItemExt::createSpriteExtra(upSpr, [this, &frames](auto) {
            if (m_selectedIndex > 0) {
                std::swap(frames[m_selectedIndex], frames[m_selectedIndex - 1]);
                m_selectedIndex--;
                m_page = m_selectedIndex / page_size;
                refresh();
            }
        });
        upBtn->setPosition({295.f, 18.f});
        upBtn->setEnabled(hasSel && m_selectedIndex > 0);
        menu->addChild(upBtn);

        auto downSpr = ButtonSprite::create("Down", 55, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 22.f, 0.4f);
        if (!hasSel || m_selectedIndex >= (int)frames.size() - 1) downSpr->setColor({150, 150, 150});
        auto downBtn = CCMenuItemExt::createSpriteExtra(downSpr, [this, &frames](auto) {
            if (m_selectedIndex < (int)frames.size() - 1) {
                std::swap(frames[m_selectedIndex], frames[m_selectedIndex + 1]);
                m_selectedIndex++;
                m_page = m_selectedIndex / page_size;
                refresh();
            }
        });
        downBtn->setPosition({365.f, 18.f});
        downBtn->setEnabled(hasSel && m_selectedIndex < (int)frames.size() - 1);
        menu->addChild(downBtn);
    }

public:
    static MacroEditorLayer* create() {
        auto ret = new MacroEditorLayer();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

$execute {
    auto& gui = GDH::Gui::get();
    auto& window = gui.getWindow("Replay Engine");
    auto& engine = GDH::ReplayEngine::get();
    auto& config = Config::get();

    window.setCustomWindowCocos([&gui, &engine, &config](cocos2d::CCNode* node) {
        auto tab = static_cast<HacksTab*>(node);
        auto engineTab = CCMenu::create();
        engineTab->setContentSize({500.f, 250.f});

        // Info Label
        auto infoLabel = LabelInfoScheduler::create("", "GoogleSans.fnt"_spr);
        infoLabel->setAnchorPoint({0.f, 0.5f});
        infoLabel->setPosition({8, 15});
        infoLabel->setScale(0.5f);
        infoLabel->schedule(schedule_selector(LabelInfoScheduler::customUpdate), 0.0f);
        engineTab->addChild(infoLabel);

        auto turnOffOthers = [engineTab](int currentTag) {
            int tags[] = {6767, 6969, 5252};
            for (int tag : tags) {
                if (tag == currentTag) continue;
                if (auto* other = static_cast<CCMenuItemToggler*>(engineTab->getChildByTag(tag))) {
                    if (other->isOn()) {
                        other->toggle(false);
                    }
                }
            }
        };

        // Record Toggle
        auto recordToggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f,
            [&engine, turnOffOthers](CCMenuItemToggler* sender) {
                if (!sender->isOn()) {
                    turnOffOthers(6767);
                    engine.clear();
                    engine.mode = state::record;
                } else {
                    engine.mode = state::disable;
                }
                engine.setupHacks(engine.mode);
            });
        
        recordToggle->setPosition({25.f, 235.f});
        recordToggle->setTag(6767);
        if (engine.mode == state::record) recordToggle->toggle(true);
        engineTab->addChild(recordToggle);
        engineTab->addChild(tab->AddTextToToggle("Record", recordToggle));

        // Play Toggle
        auto playToggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f,
            [&engine, turnOffOthers](CCMenuItemToggler* sender) {
                if (!sender->isOn()) {
                    turnOffOthers(6969);
                    engine.mode = state::play;
                } else {
                    engine.mode = state::disable;
                }
                engine.setupHacks(engine.mode);
            });

        playToggle->setPosition({123.f, 235.f});
        playToggle->setTag(6969);
        if (engine.mode == state::play) playToggle->toggle(true);
        engineTab->addChild(playToggle);
        engineTab->addChild(tab->AddTextToToggle("Play", playToggle));

        // Continue Toggle
        auto continueToggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f,
            [&engine, turnOffOthers](CCMenuItemToggler* sender) {
                if (!sender->isOn()) {
                    turnOffOthers(5252);
                    engine.mode = state::continue_mode;
                    GDH::MaterialLayer(FLAlertLayer::create(
                        "Replay Engine", 
                        "Don't forget to <cr>load the replay</c> to continue the recording. "
                        "The level will automatically switch to <cg>practice mode</c> "
                        "(and don't forget to <cy>place checkpoints!!</c>), "
                        "it'll play what you recorded and then switch back to recording mode.", 
                        "OK"
                    ))->show();
                } else {
                    engine.mode = state::disable;
                }
                engine.setupHacks(engine.mode);
            });

        continueToggle->setPosition({200.f, 235.f});
        continueToggle->setTag(5252);
        if (engine.mode == state::continue_mode) continueToggle->toggle(true);
        engineTab->addChild(continueToggle);
        engineTab->addChild(tab->AddTextToToggle("Continue", continueToggle));
        
        // Replay Name Input
        auto replayNameInput = geode::TextInput::create(220.f, "Replay Name", "GoogleSans.fnt"_spr);
        if (auto* bg = replayNameInput->getChildByType<geode::NineSlice>(0)) { 
            bg->setColor({71, 71, 131}); 
            bg->setOpacity(255); 
        }
        replayNameInput->setPosition({118.f, 200.f});
        replayNameInput->setString(engine.replay_name);
        replayNameInput->setCallback([&engine](const std::string& text) { engine.replay_name = text; });
        engineTab->addChild(replayNameInput);

        auto replayList = CCSprite::create("GDH_arrow.png"_spr);
        replayList->setScale(0.65f);
        replayList->setRotation(-90.f);
        auto replayListClick = CCMenuItemExt::createSpriteExtra(replayList, [replayNameInput](CCMenuItemSpriteExtra* sender) {
            ReplaySelectLayer::create(replayNameInput)->show();
        });
        replayListClick->setPosition({245.f, 200.f});
        engineTab->addChild(replayListClick);

        auto saveButton = ButtonSprite::create("Save", 65, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 30.f, 0.7f);
        auto saveButtonClick = CCMenuItemExt::createSpriteExtra(saveButton, [&engine, infoLabel](CCMenuItemSpriteExtra* sender) {
            GDH::MaterialLayer(FLAlertLayer::create("Info", engine.save(engine.replay_name).c_str(), "OK"))->show();
        });
        saveButtonClick->setPosition({48, 163});
        engineTab->addChild(saveButtonClick);

        auto loadButton = ButtonSprite::create("Load", 65, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 30.f, 0.7f);
        auto loadButtonClick = CCMenuItemExt::createSpriteExtra(loadButton, [&engine, infoLabel](CCMenuItemSpriteExtra* sender) {
            GDH::MaterialLayer(FLAlertLayer::create("Info", engine.load(engine.replay_name).c_str(), "OK"))->show();
        });
        loadButtonClick->setPosition({135, 163});
        engineTab->addChild(loadButtonClick);

        auto cleanButton = ButtonSprite::create("Clear", 65, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 30.f, 0.7f);
        auto cleanButtonClick = CCMenuItemExt::createSpriteExtra(cleanButton, [&engine, infoLabel](CCMenuItemSpriteExtra* sender) {
            engine.clear();
            GDH::MaterialLayer(FLAlertLayer::create("Info", "Replay has been cleared", "OK"))->show();
        });
        cleanButtonClick->setPosition({222, 163});
        engineTab->addChild(cleanButtonClick);

        auto importButton = CCSprite::create("GDH_importBtn.png"_spr);
        importButton->setScale(0.65f);

        auto importButtonClick = CCMenuItemExt::createSpriteExtra(importButton, [&engine](CCMenuItemSpriteExtra* sender) {
            geode::async::spawn(
                geode::utils::file::pick(
                    geode::utils::file::PickMode::OpenFile,
                    geode::utils::file::FilePickOptions{
                        .filters = { { "Replay Engine v4 Format", { "*.re4" } } }
                    }
                ),
                [&engine](geode::utils::file::PickResult res) {
                    if (!res) return;
                    auto pathOpt = std::move(res).unwrapOr(std::nullopt);
                    if (!pathOpt) return;
                    
                    auto selectedPath = *pathOpt;
                    if (!GDH::Utils::isOnlyAsciiPath(selectedPath)) {
                        GDH::MaterialLayer(FLAlertLayer::create("Import", "Invalid path. Please remove any Cyrillic characters", "OK"))->show();
                        return;
                    }

                    if (selectedPath.extension() != ".re4") {
                        GDH::MaterialLayer(FLAlertLayer::create("Import", "Invalid file format. Only .re4 files are allowed", "OK"))->show();
                        return;
                    }

                    auto destFolder = getFolderMacroPath();
                    std::error_code ec;
                    std::filesystem::create_directories(destFolder, ec);
                    auto destPath = destFolder / selectedPath.filename();

                    std::filesystem::copy_file(selectedPath, destPath, std::filesystem::copy_options::overwrite_existing, ec);
                    if (ec) {
                        GDH::MaterialLayer(FLAlertLayer::create("Import", "Failed to copy file", "OK"))->show();
                        return;
                    }

                    std::string macroName = destPath.stem().string();
                    GDH::MaterialLayer(FLAlertLayer::create("Import", fmt::format("Imported successfully! {}", macroName).c_str(), "OK"))->show();
                }
            );
        });
        importButtonClick->setPosition({315.f, 200.f});
        engineTab->addChild(importButtonClick);

        auto deleteButton = CCSprite::create("GDH_removeBtn.png"_spr);
        deleteButton->setScale(0.65f);
        
        auto deleteButtonClick = CCMenuItemExt::createSpriteExtra(deleteButton, [&engine, replayNameInput](CCMenuItemSpriteExtra* sender) {
            if (engine.replay_name.empty()) {
                GDH::MaterialLayer(FLAlertLayer::create("Delete Macro", "No macro name specified.", "OK"))->show();
                return;
            }

            auto popup = geode::createQuickPopup(
                "Delete Macro",
                fmt::format("Are you sure you want to delete <cy>{}</c>?\n<cr>This action cannot be undone!</c>", engine.replay_name),
                "No", "Yes",
                [&engine](auto, bool btn2) {
                    if (btn2) {
                        auto fileToDelete = getFolderMacroPath() / (engine.replay_name + ".re4");
                        std::error_code ec;
                        if (std::filesystem::exists(fileToDelete) && std::filesystem::remove(fileToDelete, ec)) {
                            GDH::MaterialLayer(FLAlertLayer::create("Delete Macro", "Macro successfully deleted", "OK"))->show();
                        } else {
                            GDH::MaterialLayer(FLAlertLayer::create("Delete Macro", "Failed to delete macro file", "OK"))->show();
                        }
                    }
                }
            );
            GDH::MaterialLayer(popup);
        });
        deleteButtonClick->setPosition({280.f, 200.f});
        engineTab->addChild(deleteButtonClick);

        auto editorButton = ButtonSprite::create("Editor", 65, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 30.f, 0.7f);
        auto editorButtonClick = CCMenuItemExt::createSpriteExtra(editorButton, [](CCMenuItemSpriteExtra* sender) {
            MacroEditorLayer::create()->show();
        });
        editorButtonClick->setPosition({300.f, 55.f});
        engineTab->addChild(editorButtonClick);

        auto recorderButton = ButtonSprite::create("Recorder", 65, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 30.f, 0.7f);
        auto recorderButtonClick = CCMenuItemExt::createSpriteExtra(recorderButton, [&engine, infoLabel](CCMenuItemSpriteExtra* sender) {
            RecorderLayer::create()->show();
        });
        recorderButtonClick->setPosition({300.f, 20.f});
        engineTab->addChild(recorderButtonClick);

        auto accuracy_fix_toggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f, [&config](CCMenuItemToggler* sender) {
            config.set<bool>("engine::accuracy_fix", !sender->isOn());
        });
        accuracy_fix_toggle->toggle(config.get<bool>("engine::accuracy_fix", true));
        accuracy_fix_toggle->setPosition({25.f, 130.f});
        engineTab->addChild(accuracy_fix_toggle);
        engineTab->addChild(tab->AddTextToToggle("Accuracy Fix", accuracy_fix_toggle));

        auto y_velocity_toggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f, [&config](CCMenuItemToggler* sender) {
            config.set<bool>("engine::velocity_fix", !sender->isOn());
        });
        y_velocity_toggle->toggle(config.get<bool>("engine::velocity_fix", false));
        y_velocity_toggle->setPosition({145.f, 130.f});
        engineTab->addChild(y_velocity_toggle);
        engineTab->addChild(tab->AddTextToToggle("Y Velocity Fix", y_velocity_toggle));

        auto ignore_inputs_toggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f, [&config](CCMenuItemToggler* sender) {
            config.set<bool>("engine::ignore_inputs", !sender->isOn());
        });
        ignore_inputs_toggle->toggle(config.get<bool>("engine::ignore_inputs", true));
        ignore_inputs_toggle->setPosition({25.f, 100.f});
        engineTab->addChild(ignore_inputs_toggle);
        engineTab->addChild(tab->AddTextToToggle("Ignore Input on Playback", ignore_inputs_toggle));

        tab->m_scrollLayer->m_contentLayer->addChild(engineTab);
    });
}