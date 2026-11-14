// Predictive Input Buffer - menu.
//
// The menu is a direct port of the original BufferMenuV2: a 380x290 popup with
// three pages (Buffer, Practice, Style), an adjust row for every numeric
// option, a help icon next to every option and a help popup with the full
// explanation.

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/cocos.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include "state.hpp"

using namespace geode::prelude;

namespace ib {

    namespace {

        // The corners the Buffer button can use in the pause menu.
        const char* const kButtonCorners[] = {
            "Bottom right",
            "Bottom left",
            "Top right",
            "Top left",
        };

        constexpr int kPageCount = 3;

        constexpr int kPageBuffer = 0;
        constexpr int kPagePractice = 1;
        constexpr int kPageStyle = 2;

        const char* const kPageTitles[kPageCount] = {
            "Buffer",
            "Practice",
            "Style",
        };

        const char* const kPageTabCaptions[kPageCount] = {
            "Buffer",
            "Practice",
            "Style",
        };

        constexpr int kAutoCloseActionTag = 5399;

        std::string formatFloat(float value, int decimals) {
            return fmt::format("{:.{}f}", value, decimals);
        }

        void showHelp(const char* title, const std::string& body) {
            auto* layer = FLAlertLayer::create(nullptr, title, body, "OK", nullptr, 300.f);
            if (!layer) return;
            layer->show();
        }

        void showGeneralHelp() {
            auto* layer = FLAlertLayer::create(
                nullptr,
                "General help",
                "<cy>Buffer</c>: the mod looks a few ticks into the future. If your tap would "
                "kill you, it delays it just enough to save you. 4 ticks = 1 frame at 60 fps.\n"
                "<cy>Normal / Wave</c>: max ticks your tap can be delayed (Wave = wave mode only).\n"
                "<cy>Spam guard</c>: if you spam quickly, the mod lets your taps through untouched.\n"
                "<cy>Spam</c>: how close taps must be to count as spam.\n"
                "<cy>Step</c>: how much the numbers change with - and +.\n"
                "<cy>Presets</c>: ready-made values (normal/wave).\n"
                "<cy>Practice</c>: noclip and speed, PRACTICE MODE ONLY.\n"
                "<cy>Style</c>: custom background, darkening, auto close and button position.\n"
                "Tap the <cy>?</c> next to each option to see its explanation.",
                "OK",
                nullptr,
                400.f,
                true,
                250.f,
                1.f
            );
            if (!layer) return;
            layer->show();
        }

        class BufferMenu : public geode::Popup {
        public:
            static BufferMenu* create() {
                auto* ret = new BufferMenu();
                if (ret->init()) {
                    ret->autorelease();
                    return ret;
                }
                delete ret;
                return nullptr;
            }

            bool init() {
                if (!geode::Popup::init(380.f, 290.f, "GJ_square01.png", CCRect{})) return false;

                setTitle("Predictive Buffer v2");

                m_step = 1;

                buildTabs();
                buildBufferPage();
                buildPracticePage();
                buildStylePage();

                showPage(kPageBuffer);
                refresh();
                applyCustomBackground();

                return true;
            }

        private:
            std::vector<cocos2d::CCNode*> m_pages[kPageCount];

            CCMenuItemSpriteExtra* m_tabs[kPageCount] = {};
            cocos2d::CCLabelBMFont* m_pageTitle = nullptr;

            ButtonSprite* m_bufferBtn = nullptr;
            ButtonSprite* m_spamGuardBtn = nullptr;
            ButtonSprite* m_stepBtn = nullptr;
            ButtonSprite* m_noclipBtn = nullptr;
            ButtonSprite* m_placeBtn = nullptr;

            cocos2d::CCLabelBMFont* m_normalLabel = nullptr;
            cocos2d::CCLabelBMFont* m_waveLabel = nullptr;
            cocos2d::CCLabelBMFont* m_spamLabel = nullptr;
            cocos2d::CCLabelBMFont* m_framesLabel = nullptr;
            cocos2d::CCLabelBMFont* m_speedLabel = nullptr;
            cocos2d::CCLabelBMFont* m_imageLabel = nullptr;
            cocos2d::CCLabelBMFont* m_opacityLabel = nullptr;
            cocos2d::CCLabelBMFont* m_dimLabel = nullptr;
            cocos2d::CCLabelBMFont* m_autoCloseLabel = nullptr;
            cocos2d::CCLabelBMFont* m_buttonSizeLabel = nullptr;

            cocos2d::CCSprite* m_customBg = nullptr;
            cocos2d::CCLayerColor* m_dimLayer = nullptr;

            int m_step = 1;

            // -----------------------------------------------------------------
            //  Building blocks
            // -----------------------------------------------------------------

            CCMenuItemSpriteExtra* makeItem(
                cocos2d::CCNode* sprite,
                std::function<void()> callback
            ) {
                auto* item = CCMenuItemSpriteExtra::create(sprite, nullptr, nullptr, nullptr);
                if (!item) return nullptr;
                geode::cocos::CCMenuItemExt::assignCallback<CCMenuItemSpriteExtra>(
                    item,
                    [callback](CCMenuItemSpriteExtra*) { callback(); }
                );
                return item;
            }

            ButtonSprite* makeButton(const char* caption) {
                return ButtonSprite::create(
                    caption, 0, 0, 1.f, false, "goldFont.fnt", "GJ_button_01.png", 0.f
                );
            }

            void addButton(
                int page,
                ButtonSprite* sprite,
                float scale,
                float x,
                float y,
                std::function<void()> callback
            ) {
                if (!sprite) return;
                sprite->setScale(scale);

                auto* item = makeItem(sprite, std::move(callback));
                if (!item) return;

                m_buttonMenu->addChildAtPosition(item, geode::Anchor::Center, {x, y}, true);
                m_pages[page].push_back(item);
            }

            cocos2d::CCLabelBMFont* addLabel(int page, const char* font, float scale, float x, float y) {
                auto* label = CCLabelBMFont::create("", font);
                if (!label) return nullptr;
                label->setScale(scale);
                m_mainLayer->addChildAtPosition(label, geode::Anchor::Center, {x, y}, true);
                m_pages[page].push_back(label);
                return label;
            }

            // A label that stays visible on every page.
            cocos2d::CCLabelBMFont* addStaticLabel(const char* font, float scale, float x, float y) {
                auto* label = CCLabelBMFont::create("", font);
                if (!label) return nullptr;
                label->setScale(scale);
                m_mainLayer->addChildAtPosition(label, geode::Anchor::Center, {x, y}, true);
                return label;
            }

            void addHelp(int page, float x, float y, const char* title, const char* text) {
                auto* sprite = cocos2d::CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
                if (!sprite) return;
                sprite->setScale(0.55f);

                auto* item = makeItem(sprite, [this, title, text]() {
                    showHelp(title, text);
                });
                if (!item) return;

                m_buttonMenu->addChildAtPosition(item, geode::Anchor::Center, {x, y}, true);
                m_pages[page].push_back(item);
            }

            void addAdjustRow(
                int page,
                float y,
                cocos2d::CCLabelBMFont*& outLabel,
                std::function<void(int)> callback,
                const char* title,
                const char* text
            ) {
                outLabel = addLabel(page, "bigFont.fnt", 0.55f, -20.f, y);

                auto* minus = makeButton("-");
                addButton(page, minus, 0.55f, -135.f, y, [this, callback]() { callback(-1); });

                auto* plus = makeButton("+");
                addButton(page, plus, 0.55f, 95.f, y, [this, callback]() { callback(1); });

                addHelp(page, 132.f, y, title, text);
            }

            // -----------------------------------------------------------------
            //  Pages
            // -----------------------------------------------------------------

            void buildTabs() {
                const float tabX[kPageCount] = {-105.f, 0.f, 105.f};

                for (int i = 0; i < kPageCount; ++i) {
                    auto* sprite = makeButton(kPageTabCaptions[i]);
                    if (!sprite) continue;
                    sprite->setScale(0.6f);

                    auto* item = makeItem(sprite, [this, i]() { showPage(i); });
                    if (!item) continue;

                    m_tabs[i] = item;
                    m_buttonMenu->addChildAtPosition(item, geode::Anchor::Center, {tabX[i], 108.f}, true);
                }

                m_pageTitle = addStaticLabel("goldFont.fnt", 0.45f, 0.f, 82.f);

                auto* helpSprite = cocos2d::CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
                if (helpSprite) {
                    helpSprite->setScale(0.8f);
                    auto* item = makeItem(helpSprite, []() { showGeneralHelp(); });
                    if (item) {
                        m_buttonMenu->addChildAtPosition(item, geode::Anchor::Center, {165.f, 108.f}, true);
                    }
                }
            }

            void buildBufferPage() {
                int page = kPageBuffer;

                m_bufferBtn = makeButton("Buffer: ON");
                addButton(page, m_bufferBtn, 0.5f, -95.f, 58.f, [this]() { toggleBool("enabled"); });
                addHelp(
                    page, -30.f, 58.f, "Buffer ON/OFF",
                    "Turns the whole mod on or off. When OFF your taps pass through untouched."
                );

                m_spamGuardBtn = makeButton("Spam guard: ON");
                addButton(page, m_spamGuardBtn, 0.5f, 70.f, 58.f, [this]() { toggleBool("spam-guard"); });
                addHelp(
                    page, 150.f, 58.f, "Spam guard",
                    "If you make 3 or more taps very close together (spam), the mod does not modify "
                    "them. That way it does not feel weird and the fps do not drop."
                );

                addAdjustRow(
                    page, 30.f, m_normalLabel,
                    [this](int delta) { changeInt("normal-ticks", delta * m_step, 0, 40); },
                    "Normal (ticks)",
                    "Max ticks the mod can delay your tap in cube, ball, UFO, spider, robot and "
                    "swing. Higher corrects more, but delays more. 4 ticks = 1 frame."
                );

                addAdjustRow(
                    page, 2.f, m_waveLabel,
                    [this](int delta) { changeInt("wave-ticks", delta * m_step, 0, 40); },
                    "Wave (ticks)",
                    "Same as Normal, but only while you play in wave. It is usually better to keep "
                    "it lower."
                );

                addAdjustRow(
                    page, -26.f, m_spamLabel,
                    [this](int delta) { changeInt("spam-interval", delta * m_step, 4, 60); },
                    "Spam (ticks)",
                    "Two taps less than this apart count as spam. 20 ticks = 12 taps per second. "
                    "Higher = the spam guard triggers more easily."
                );

                m_stepBtn = makeButton("Step: 1");
                addButton(page, m_stepBtn, 0.5f, -20.f, -52.f, [this]() {
                    m_step = m_step == 1 ? 4 : 1;
                    refresh();
                });
                addHelp(
                    page, 40.f, -52.f, "Step",
                    "How much the numbers change with every - or + tap. At 1 it goes by 1, at 4 it "
                    "goes by 4 (one frame). It does not change the mod, it only makes adjusting "
                    "faster."
                );

                m_framesLabel = addLabel(page, "chatFont.fnt", 0.6f, 0.f, -126.f);

                struct Preset {
                    int normal;
                    int wave;
                    float x;
                    float y;
                };
                static const Preset presets[] = {
                    {2, 2, -128.f, -78.f},
                    {4, 4, -64.f, -78.f},
                    {8, 8, 0.f, -78.f},
                    {12, 8, 64.f, -78.f},
                    {16, 8, 128.f, -78.f},
                    {20, 8, -128.f, -102.f},
                    {20, 12, -64.f, -102.f},
                    {20, 20, 0.f, -102.f},
                    {32, 16, 64.f, -102.f},
                    {40, 20, 128.f, -102.f},
                };

                for (auto& preset : presets) {
                    auto caption = fmt::format("{}/{}", preset.normal, preset.wave);
                    auto* sprite = makeButton(caption.c_str());
                    addButton(page, sprite, 0.45f, preset.x, preset.y, [this, preset]() {
                        auto* mod = Mod::get();
                        mod->setSettingValue<int>("normal-ticks", preset.normal);
                        mod->setSettingValue<int>("wave-ticks", preset.wave);
                        mod->setSettingValue<bool>("enabled", true);
                        syncConfig();
                        refresh();
                    });
                }

                addHelp(
                    page, 165.f, -90.f, "Presets",
                    "Shortcuts that set Normal and Wave at once. The first number is Normal and "
                    "the second is Wave. Example: 20/8 = 20 ticks normal and 8 in wave."
                );
            }

            void buildPracticePage() {
                int page = kPagePractice;

                auto* info = addLabel(page, "chatFont.fnt", 0.65f, 0.f, 60.f);
                if (info) info->setString("Only works in practice mode");

                m_noclipBtn = makeButton("Noclip: OFF");
                addButton(page, m_noclipBtn, 0.6f, -20.f, 30.f, [this]() { toggleBool("noclip"); });
                addHelp(
                    page, 60.f, 30.f, "Noclip",
                    "You do not die from obstacles. Only works in practice mode, never in normal "
                    "levels."
                );

                addAdjustRow(
                    page, -2.f, m_speedLabel,
                    [this](int delta) { changeFloat("game-speed", delta * 0.05f, 0.1f, 2.f, 2); },
                    "Speed",
                    "Game speed, from 0.10x (slow motion) to 2.00x. Practice mode only; outside of "
                    "it it is always 1x."
                );

                struct SpeedPreset {
                    float value;
                    float x;
                };
                static const SpeedPreset speeds[] = {
                    {0.25f, -125.f},
                    {0.5f, -75.f},
                    {0.75f, -25.f},
                    {1.f, 25.f},
                    {1.5f, 75.f},
                    {2.f, 125.f},
                };

                for (auto& speed : speeds) {
                    auto caption = fmt::format("{}x", formatFloat(speed.value, 2));
                    auto* sprite = makeButton(caption.c_str());
                    addButton(page, sprite, 0.45f, speed.x, -34.f, [this, speed]() {
                        Mod::get()->setSettingValue<float>("game-speed", speed.value);
                        syncConfig();
                        refresh();
                    });
                }

                auto* note = addLabel(page, "chatFont.fnt", 0.6f, 0.f, -70.f);
                if (note) note->setString("Outside of practice mode the game always runs at 1x");
            }

            void buildStylePage() {
                int page = kPageStyle;

                m_imageLabel = addLabel(page, "chatFont.fnt", 0.7f, 0.f, 58.f);

                auto* choose = makeButton("Choose image");
                addButton(page, choose, 0.5f, -95.f, 32.f, []() {
                    geode::openSettingsPopup(Mod::get());
                });

                auto* apply = makeButton("Apply");
                addButton(page, apply, 0.5f, 5.f, 32.f, [this]() {
                    applyCustomBackground();
                    refresh();
                });

                auto* remove = makeButton("Remove");
                addButton(page, remove, 0.5f, 80.f, 32.f, [this]() {
                    Mod::get()->setSettingValue<asp::fs::Path>("background-image", asp::fs::Path());
                    syncConfig();
                    applyCustomBackground();
                    refresh();
                });

                addHelp(
                    page, 150.f, 32.f, "Custom background",
                    "Choosing an image opens the mod settings: there you pick a PNG or JPG (up to "
                    "1024x1024). Then tap Apply to see it. Remove goes back to the normal background."
                );

                addAdjustRow(
                    page, 6.f, m_opacityLabel,
                    [this](int delta) { changeInt("background-opacity", delta * 10, 10, 100); },
                    "Opacity",
                    "How visible your background image is, from 10% to 100%."
                );

                addAdjustRow(
                    page, -20.f, m_dimLabel,
                    [this](int delta) { changeInt("background-dim", delta * 10, 0, 90); },
                    "Darken",
                    "Darkens the image so the buttons and text are easier to read."
                );

                addAdjustRow(
                    page, -46.f, m_autoCloseLabel,
                    [this](int delta) { changeInt("auto-close", delta * 5, 0, 120); },
                    "Auto close",
                    "Seconds without touching anything before the menu closes itself. At 0 it never "
                    "closes."
                );

                addAdjustRow(
                    page, -72.f, m_buttonSizeLabel,
                    [this](int delta) { changeFloat("button-scale", delta * 0.1f, 0.4f, 1.2f, 1); },
                    "Button size",
                    "Size of the Buffer button in the pause menu."
                );

                m_placeBtn = makeButton("Place: Bottom right");
                addButton(page, m_placeBtn, 0.5f, -20.f, -100.f, [this]() {
                    auto* mod = Mod::get();
                    auto current = mod->getSettingValue<std::string>("button-corner");
                    int index = 0;
                    for (int i = 0; i < 4; ++i) {
                        if (current == kButtonCorners[i]) {
                            index = i;
                            break;
                        }
                    }
                    mod->setSettingValue<std::string>(
                        "button-corner", kButtonCorners[(index + 1) % 4]
                    );
                    syncConfig();
                    refresh();
                });
                addHelp(
                    page, 70.f, -100.f, "Button place",
                    "Changes the corner where the Buffer button appears in the pause menu. It "
                    "applies the next time you pause."
                );
            }

            // -----------------------------------------------------------------
            //  Behaviour
            // -----------------------------------------------------------------

            void showPage(int page) {
                for (int i = 0; i < kPageCount; ++i) {
                    bool visible = i == page;
                    for (auto* node : m_pages[i]) {
                        node->setVisible(visible);
                    }
                    if (m_tabs[i]) {
                        m_tabs[i]->setScale(visible ? 0.8f : 0.6f);
                    }
                }

                if (m_pageTitle) m_pageTitle->setString(kPageTitles[page]);

                resetAutoClose();
            }

            void toggleBool(const char* key) {
                auto* mod = Mod::get();
                bool value = mod->getSettingValue<bool>(key);
                mod->setSettingValue<bool>(key, !value);
                syncConfig();
                refresh();
            }

            void changeInt(const char* key, int delta, int min, int max) {
                auto* mod = Mod::get();
                int value = mod->getSettingValue<int>(key);
                value = std::clamp(value + delta, min, max);
                mod->setSettingValue<int>(key, value);
                syncConfig();
                refresh();
            }

            void changeFloat(const char* key, float delta, float min, float max, int decimals) {
                auto* mod = Mod::get();
                float value = mod->getSettingValue<float>(key);
                float factor = std::pow(10.f, static_cast<float>(decimals));
                value = std::round((value + delta) * factor) / factor;
                value = std::clamp(value, min, max);
                mod->setSettingValue<float>(key, value);
                syncConfig();
                refresh();
            }

            void resetAutoClose() {
                stopActionByTag(kAutoCloseActionTag);

                int seconds = Mod::get()->getSettingValue<int>("auto-close");
                if (seconds < 1) return;

                auto* sequence = CCSequence::create(
                    CCDelayTime::create(static_cast<float>(seconds)),
                    CCCallFunc::create(this, callfunc_selector(BufferMenu::onAutoClose))
                );
                if (!sequence) return;
                sequence->setTag(kAutoCloseActionTag);
                runAction(sequence);
            }

            void onAutoClose() {
                this->onClose(nullptr);
            }

            void applyCustomBackground() {
                if (m_customBg) {
                    m_customBg->removeFromParent();
                    m_customBg = nullptr;
                }
                if (m_dimLayer) {
                    m_dimLayer->removeFromParent();
                    m_dimLayer = nullptr;
                }
                if (m_bgSprite) m_bgSprite->setVisible(true);

                auto path = Mod::get()->getSettingValue<asp::fs::Path>("background-image");
                if (path.empty()) return;

                std::error_code ec;
                auto fsPath = std::filesystem::path(path.string());
                if (!std::filesystem::exists(fsPath, ec)) {
                    log::warn("[InputBuffer] Background image missing or larger than 8 MB, ignoring it");
                    return;
                }
                auto size = std::filesystem::file_size(fsPath, ec);
                if (ec || size > 0x800000) {
                    log::warn("[InputBuffer] Background image missing or larger than 8 MB, ignoring it");
                    return;
                }

                auto pathString = path.string();
                auto* sprite = cocos2d::CCSprite::create(pathString.c_str());
                if (!sprite) {
                    log::error("[InputBuffer] Could not load the background image");
                    return;
                }

                auto spriteSize = sprite->getContentSize();
                if (spriteSize.width <= 0.f || spriteSize.height <= 0.f) return;

                auto target = m_mainLayer->getContentSize();
                sprite->setScaleX((target.width - 12.f) / spriteSize.width);
                sprite->setScaleY((target.height - 12.f) / spriteSize.height);
                sprite->setPosition({target.width * 0.5f, target.height * 0.5f});

                int opacity = std::clamp(
                    Mod::get()->getSettingValue<int>("background-opacity"), 0, 100
                );
                sprite->setOpacity(static_cast<GLubyte>(opacity * 255 / 100));

                m_customBg = sprite;
                if (m_bgSprite) m_bgSprite->setZOrder(-3);
                m_mainLayer->addChild(sprite, -2);

                int dim = std::clamp(
                    Mod::get()->getSettingValue<int>("background-dim"), 0, 90
                );
                if (dim >= 1) {
                    auto* layer = cocos2d::CCLayerColor::create(
                        ccc4(0, 0, 0, dim * 255 / 100),
                        target.width - 12.f,
                        target.height - 12.f
                    );
                    if (layer) {
                        layer->setPosition({6.f, 6.f});
                        m_dimLayer = layer;
                        m_mainLayer->addChild(layer, -1);
                    }
                }
            }

            void refresh() {
                resetAutoClose();

                auto* mod = Mod::get();
                auto& cfg = S().cfg;

                auto setLabel = [](cocos2d::CCLabelBMFont* label, std::string text) {
                    if (label) label->setString(text.c_str());
                };
                auto setButton = [](ButtonSprite* button, std::string text) {
                    if (button) button->setString(text.c_str());
                };

                setButton(m_bufferBtn, cfg.enabled ? "Buffer: ON" : "Buffer: OFF");
                setButton(m_spamGuardBtn, cfg.spamGuard ? "Spam guard: ON" : "Spam guard: OFF");
                setButton(m_stepBtn, fmt::format("Step: {}", m_step));
                setLabel(m_normalLabel, fmt::format("Normal: {} ticks", cfg.normalTicks));
                setLabel(m_waveLabel, fmt::format("Wave: {} ticks", cfg.waveTicks));
                setLabel(m_spamLabel, fmt::format("Spam: {} ticks", cfg.spamInterval));
                setLabel(m_framesLabel, fmt::format(
                    "Normal {:.2f}f | Wave {:.2f}f @ 60fps (4 ticks = 1 frame)",
                    static_cast<double>(cfg.normalTicks),
                    static_cast<double>(cfg.waveTicks)
                ));
                setButton(m_noclipBtn, cfg.noclip ? "Noclip: ON" : "Noclip: OFF");
                setLabel(m_speedLabel, fmt::format("Speed: x{:.2f}", cfg.gameSpeed));

                auto path = mod->getSettingValue<asp::fs::Path>("background-image");
                if (path.empty()) {
                    setLabel(m_imageLabel, "Image: none");
                } else {
                    auto name = std::filesystem::path(path.string()).filename().string();
                    setLabel(m_imageLabel, fmt::format("Image: {}...", name));
                }

                int opacity = std::clamp(mod->getSettingValue<int>("background-opacity"), 0, 100);
                setLabel(m_opacityLabel, fmt::format("Opacity: {}%", opacity));

                int dim = std::clamp(mod->getSettingValue<int>("background-dim"), 0, 90);
                setLabel(m_dimLabel, fmt::format("Darken: {}%", dim));

                int autoClose = mod->getSettingValue<int>("auto-close");
                if (autoClose < 1) {
                    setLabel(m_autoCloseLabel, "Auto close: never");
                } else {
                    setLabel(m_autoCloseLabel, fmt::format("Auto close: {} s", autoClose));
                }

                setLabel(m_buttonSizeLabel, fmt::format("Button size: {:.1f}", cfg.buttonScale));
                setButton(m_placeBtn, fmt::format("Place: {}", cfg.buttonCorner));
            }
        };

    }

    void openMenu() {
        auto* menu = BufferMenu::create();
        if (!menu) return;
        menu->show();
    }

    void refreshMenu() {
        // The menu keeps its own state, it only has to be opened again.
    }

    void addPauseButton(PauseLayer* layer) {
        if (!layer) return;

        auto& cfg = S().cfg;

        auto* sprite = ButtonSprite::create(
            "Buffer", 0, 0, 1.f, false, "goldFont.fnt", "GJ_button_01.png", 0.f
        );
        if (!sprite) return;
        sprite->setScale(cfg.buttonScale);

        auto* item = CCMenuItemSpriteExtra::create(sprite, nullptr, nullptr, nullptr);
        if (!item) return;
        geode::cocos::CCMenuItemExt::assignCallback<CCMenuItemSpriteExtra>(
            item,
            [](CCMenuItemSpriteExtra*) { openMenu(); }
        );

        auto* menu = CCMenu::create();
        if (!menu) return;
        menu->addChild(item);

        auto winSize = CCDirector::get()->getWinSize();
        if (cfg.buttonCorner == "Bottom left") {
            item->setPosition({60.f, 30.f});
        } else if (cfg.buttonCorner == "Top right") {
            item->setPosition({winSize.width - 60.f, winSize.height - 30.f});
        } else if (cfg.buttonCorner == "Top left") {
            item->setPosition({60.f, winSize.height - 30.f});
        } else {
            item->setPosition({winSize.width - 60.f, 30.f});
        }

        layer->addChild(menu, 10);
    }

}
