#include <Geode/Geode.hpp>
#include "../hacks_tab.hpp"
#include "../../../core/gui.hpp"
#include "../../../core/labels.hpp"
#include "../hack_settings_popup.hpp"

using namespace geode::prelude;

void refreshLabelsTab(HacksTab* tab);

static int s_currentCornerIndex = 0;

class AddLabelPopup : public geode::Popup {
protected:
    GDH::Labels::Corner m_corner;
    HacksTab* m_tab;

    bool init(GDH::Labels::Corner corner, HacksTab* tab) {
        if (!geode::Popup::init(300.f, 220.f, "GDH_square.png"_spr)) return false;

        m_corner = corner;
        m_tab = tab;

        auto winSize = m_mainLayer->getContentSize();

        auto title = geode::Label::create("Add Label", "GoogleSans.fnt"_spr);
        title->setPosition({winSize.width / 2, winSize.height - 18.f});
        title->setScale(0.7f);
        m_mainLayer->addChild(title);

        auto closeBtn = CCSprite::create("GDH_closeBtn.png"_spr);
        closeBtn->setScale(0.75f);
        m_closeBtn->setSprite(closeBtn);

        std::vector<std::pair<std::string, std::string>> templates = {
            {"Attempts", "Attempt {attempt}"},
            {"Best Run", "Best Run: {best_run}"},
            {"Best Time", "Best Time: {best_time}"},
            {"CPS Counter", "{cps}/{cps_high}/{clicks}"},
            {"CPS Counter (P1)", "{p1::cps}/{p1::cps_high}/{p1::clicks}"},
            {"CPS Counter (P2)", "{p2::cps}/{p2::cps_high}/{p2::clicks}"},
            {"Cheat Indicator", ""},
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
            {"Practice Percent", "{practice_percent}%"},
            {"Player 1 Information", "{p1::x}{\\n}{p1::y}{\\n}{p1::y_vel}{\\n}P1Flip: {p1::is_upside_down}"},
            {"Player 2 Information", "{p2::x}{\\n}{p2::y}{\\n}{p2::y_vel}{\\n}P2Flip: {p2::is_upside_down}"},
            {"Rainbow Text", "Rainbow Text!!"},
            {"Replay Engine State", "{re_state}"},
            {"Session time", "{session_time}"},
            {"Testmode", "{testmode}"},
            {"Time (12h)", "{time_12}"},
            {"Time (24h)", "{time_24}"},
            {"Spacing", "SPACING"}
        };

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

        for (auto& [name, value] : templates) {
            auto menu = CCMenu::create();
            menu->setContentSize({600.f, 25.f});

            auto btnSprite = ButtonSprite::create(name.c_str(), 220, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 26.f, 0.5f);
            auto btn = CCMenuItemExt::createSpriteExtra(btnSprite, [this, name = name, value = value](auto) {
                auto& manager = GDH::Labels::Manager::get();
                if (name == "Spacing") {
                    manager.labels[m_corner].push_back(GDH::Labels::Label(10.0f)); 
                } 
                else if (name == "Cheat Indicator") {
                    manager.labels[m_corner].push_back(GDH::Labels::Label(1.f, GDH::Labels::LabelType::CheatIndicator));

                    auto& gui = GDH::Gui::get();
                    auto& hack = gui.getWindow("Level").findHackByName("Cheat Indicator");
                    if (!hack.getEnabled()) hack.enable();
                } else {
                    bool isRainbow = (name == "Rainbow Text");
                    bool isCPS = name.contains("CPS Counter");
                    bool isNoclip = (name == "Death Counter" || name == "Noclip Accuracy (%)");
                    std::array<float, 4> color = {1.f, 1.f, 1.f, 0.3f};
                    if (isRainbow) color[3] = 0.85f;

                    manager.labels[m_corner].push_back(
                        GDH::Labels::Label(value, color, 0.35f, isRainbow, isCPS, isNoclip)
                    );
                }
                manager.save();
                refreshLabelsTab(m_tab);
                this->m_closeBtn->activate();
            });
            
            btn->setPosition({300.f, 12.5f});
            menu->addChild(btn);
            scroll->m_contentLayer->addChild(menu);
        }

        scroll->m_contentLayer->addChild(CCNode::create());

        scroll->m_contentLayer->updateLayout();
        scroll->updateLayout();
        scroll->moveToTop();
        return true;
    }
public:
    static AddLabelPopup* create(GDH::Labels::Corner corner, HacksTab* tab) {
        auto ret = new AddLabelPopup();
        if (ret->init(corner, tab)) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

class EditLabelPopup : public geode::Popup {
protected:
    GDH::Labels::Corner m_corner;
    size_t m_index;
    HacksTab* m_tab;

    bool init(GDH::Labels::Corner corner, size_t index, HacksTab* tab) {
        if (!geode::Popup::init(280.f, 210.f, "GDH_square.png"_spr)) return false;

        auto closeBtn = CCSprite::create("GDH_closeBtn.png"_spr);
        closeBtn->setScale(0.75f);
        m_closeBtn->setSprite(closeBtn);

        m_corner = corner;
        m_index = index;
        m_tab = tab;

        auto winSize = m_mainLayer->getContentSize();
        auto& manager = GDH::Labels::Manager::get();
        
        if (m_index >= manager.labels[m_corner].size()) return false;
        auto& label = manager.labels[m_corner][m_index];

        auto title = geode::Label::create("Edit Label", "GoogleSans.fnt"_spr);
        title->setPosition({winSize.width / 2, winSize.height - 18.f});
        title->setScale(0.7f);
        m_mainLayer->addChild(title);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        float y = winSize.height - 50.f;

        auto toggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.75f, [&label](CCMenuItemToggler* sender) {
            label.enabled = !sender->isToggled();
        });
        toggle->setPosition({35.f, y});
        toggle->toggle(label.enabled);
        menu->addChild(toggle);

        auto enabledLabel = geode::Label::create("Enabled", "GoogleSans.fnt"_spr);
        enabledLabel->setScale(0.6f);
        enabledLabel->setAnchorPoint({0.f, 0.5f});
        enabledLabel->setPosition({60.f, y});
        menu->addChild(enabledLabel);

        y -= 35.f;

        if (label.type == GDH::Labels::LabelType::Text) {
            auto input = TextInput::create(240.f, "Text", "chatFont.fnt");
            if (auto* bg = input->getChildByType<geode::NineSlice>(0)) { 
                bg->setColor({71, 71, 131}); 
                bg->setOpacity(255); 
            }
            input->setPosition({winSize.width / 2, y});
            input->setString(label.text);
            input->setFilter("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ ");
            input->setCallback([&label](const std::string& str) {
                label.text = str;
            });
            m_mainLayer->addChild(input);

            y -= 35.f;

            auto sizeLabel = geode::Label::create("Size:", "GoogleSans.fnt"_spr);
            sizeLabel->setScale(0.6f);
            sizeLabel->setPosition({35.f, y});
            menu->addChild(sizeLabel);

            auto sizeInput = TextInput::create(50.f, "Size", "chatFont.fnt");
            if (auto* bg = sizeInput->getChildByType<geode::NineSlice>(0)) { 
                bg->setColor({71, 71, 131}); 
                bg->setOpacity(255); 
            }
            sizeInput->setPosition({85.f, y});
            sizeInput->setString(fmt::format("{:.2f}", label.size));
            sizeInput->setFilter("0123456789.");
            sizeInput->setCallback([&label](const std::string& str) {
                if (auto val = geode::utils::numFromString<float>(str)) {
                    label.size = std::clamp(val.unwrap(), 0.1f, 4.0f);
                }
            });
            m_mainLayer->addChild(sizeInput);

            auto colorLabel = geode::Label::create("Color:", "GoogleSans.fnt"_spr);
            colorLabel->setScale(0.6f);
            colorLabel->setPosition({145.f, y});
            menu->addChild(colorLabel);

            cocos2d::ccColor3B curColor3B = {
                static_cast<GLubyte>(label.color[0] * 255.f),
                static_cast<GLubyte>(label.color[1] * 255.f),
                static_cast<GLubyte>(label.color[2] * 255.f)
            };
            GLubyte curAlpha = static_cast<GLubyte>(label.color[3] * 255.f);

            auto colorSprite = ColorChannelSprite::create();
            colorSprite->setScale(0.7f);
            colorSprite->setColor(curColor3B);
            colorSprite->setOpacity(curAlpha);

            auto colorBtn = CCMenuItemExt::createSpriteExtra(colorSprite, [this, corner = m_corner, index = m_index, colorSprite](auto) {
                auto& manager = GDH::Labels::Manager::get();
                if (index >= manager.labels[corner].size()) return;
                
                auto& targetLabel = manager.labels[corner][index];
                cocos2d::ccColor4B popupColor = {
                    static_cast<GLubyte>(targetLabel.color[0] * 255.f),
                    static_cast<GLubyte>(targetLabel.color[1] * 255.f),
                    static_cast<GLubyte>(targetLabel.color[2] * 255.f),
                    static_cast<GLubyte>(targetLabel.color[3] * 255.f)
                };

                auto popup = CompatibleColorPopup::create(popupColor, true);
                popup->setCallback([weakSelf = geode::WeakRef(this), corner, index, colorSprite](cocos2d::ccColor4B const& formatColor) {
                    if (!weakSelf.lock()) return;

                    auto& mgr = GDH::Labels::Manager::get();
                    if (index < mgr.labels[corner].size()) {
                        auto& l = mgr.labels[corner][index];
                        l.color[0] = formatColor.r / 255.f;
                        l.color[1] = formatColor.g / 255.f;
                        l.color[2] = formatColor.b / 255.f;
                        l.color[3] = formatColor.a / 255.f;
                    }

                    colorSprite->setColor(geode::cocos::to3B(formatColor));
                    colorSprite->setOpacity(formatColor.a);
                });
                popup->show();
            });
            colorBtn->setPosition({190.f, y});
            menu->addChild(colorBtn);

            y -= 30.f;
            
            auto rbToggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.5f, [&label](CCMenuItemToggler* sender) {
                label.rainbow = !sender->isToggled();
            });
            rbToggle->setPosition({30.f, y});
            rbToggle->toggle(label.rainbow);
            menu->addChild(rbToggle);

            auto rbL = geode::Label::create("Rainbow", "GoogleSans.fnt"_spr);
            rbL->setScale(0.35f);
            rbL->setAnchorPoint({0.f, 0.5f});
            rbL->setPosition({45.f, y});
            menu->addChild(rbL);

            auto cpsToggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.5f, [&label](CCMenuItemToggler* sender) {
                label.cps = !sender->isToggled();
            });
            cpsToggle->setPosition({120.f, y});
            cpsToggle->toggle(label.cps);
            menu->addChild(cpsToggle);

            auto cpsL = geode::Label::create("CPS", "GoogleSans.fnt"_spr);
            cpsL->setScale(0.35f);
            cpsL->setAnchorPoint({0.f, 0.5f});
            cpsL->setPosition({135.f, y});
            menu->addChild(cpsL);

            auto ncToggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.5f, [&label](CCMenuItemToggler* sender) {
                label.noclip = !sender->isToggled();
            });
            ncToggle->setPosition({200.f, y});
            ncToggle->toggle(label.noclip);
            menu->addChild(ncToggle);

            auto ncL = geode::Label::create("Noclip", "GoogleSans.fnt"_spr);
            ncL->setScale(0.35f);
            ncL->setAnchorPoint({0.f, 0.5f});
            ncL->setPosition({215.f, y});
            menu->addChild(ncL);

        } else if (label.type == GDH::Labels::LabelType::Spacing) {
            auto spaceLabel = geode::Label::create("Spacing (px):", "GoogleSans.fnt"_spr);
            spaceLabel->setScale(0.4f);
            spaceLabel->setPosition({60.f, y});
            menu->addChild(spaceLabel);

            auto spaceInput = TextInput::create(80.f, "Px", "chatFont.fnt");
            if (auto* bg = spaceInput->getChildByType<geode::NineSlice>(0)) { 
                bg->setColor({71, 71, 131}); 
                bg->setOpacity(255); 
            }
            spaceInput->setPosition({150.f, y});
            spaceInput->setString(fmt::format("{:.2f}", label.size));
            spaceInput->setFilter("0123456789.");
            spaceInput->setCallback([&label](const std::string& str) {
                if (auto val = geode::utils::numFromString<float>(str)) {
                    label.size = std::clamp(val.unwrap(), 0.01f, 256.0f);
                }
            });
            m_mainLayer->addChild(spaceInput);
        } else if (label.type == GDH::Labels::LabelType::CheatIndicator) {
            auto sizeLabel = geode::Label::create("Size (px):", "GoogleSans.fnt"_spr);
            sizeLabel->setScale(0.4f);
            sizeLabel->setPosition({45.f, y});
            menu->addChild(sizeLabel);

            auto sizeInput = TextInput::create(50.f, "Px", "chatFont.fnt");
            if (auto* bg = sizeInput->getChildByType<geode::NineSlice>(0)) { 
                bg->setColor({71, 71, 131}); 
                bg->setOpacity(255); 
            }
            sizeInput->setPosition({95.f, y});
            sizeInput->setString(fmt::format("{:.2f}", label.size));
            sizeInput->setFilter("0123456789.");
            sizeInput->setCallback([&label](const std::string& str) {
                if (auto val = geode::utils::numFromString<float>(str)) {
                    label.size = std::clamp(val.unwrap(), 0.01f, 256.0f);
                }
            });
            m_mainLayer->addChild(sizeInput);

            auto opacityLabel = geode::Label::create("Opacity:", "GoogleSans.fnt"_spr);
            opacityLabel->setScale(0.4f);
            opacityLabel->setPosition({160.f, y});
            menu->addChild(opacityLabel);

            GLubyte curAlpha = static_cast<GLubyte>(label.color[3] * 255.f);

            auto colorSprite = ColorChannelSprite::create();
            colorSprite->setScale(0.7f);
            colorSprite->setColor({255, 255, 255});
            colorSprite->setOpacity(curAlpha);

            auto colorBtn = CCMenuItemExt::createSpriteExtra(colorSprite, [this, corner = m_corner, index = m_index, colorSprite](auto) {
                auto& manager = GDH::Labels::Manager::get();
                if (index >= manager.labels[corner].size()) return;
                
                auto& targetLabel = manager.labels[corner][index];
                cocos2d::ccColor4B popupColor = {
                    255, 255, 255,
                    static_cast<GLubyte>(targetLabel.color[3] * 255.f)
                };

                auto popup = CompatibleColorPopup::create(popupColor, true);
                popup->setCallback([weakSelf = geode::WeakRef(this), corner, index, colorSprite](cocos2d::ccColor4B const& formatColor) {
                    if (!weakSelf.lock()) return;

                    auto& mgr = GDH::Labels::Manager::get();
                    if (index < mgr.labels[corner].size()) {
                        auto& l = mgr.labels[corner][index];
                        l.color[3] = formatColor.a / 255.f;
                    }

                    colorSprite->setOpacity(formatColor.a);
                });
                popup->show();
            });
            colorBtn->setPosition({210.f, y});
            menu->addChild(colorBtn);
        }

        y = 30.f;
        
        auto upBtn = CCMenuItemExt::createSpriteExtra(CCSprite::create("GDH_arrow.png"_spr), [this, corner, tab](auto) {
            auto& targetLabels = GDH::Labels::Manager::get().labels[corner];
            if (m_index > 0) {
                std::swap(targetLabels[m_index - 1], targetLabels[m_index]);
                refreshLabelsTab(m_tab);
                this->m_closeBtn->activate();
            }
        });
        upBtn->getChildByType<CCSprite>(0)->setRotation(90.f);
        upBtn->getChildByType<CCSprite>(0)->setScale(0.6f);
        upBtn->setPosition({35.f, y});
        upBtn->setEnabled(m_index > 0);
        if (m_index == 0) upBtn->setOpacity(100);
        menu->addChild(upBtn);

        auto downBtn = CCMenuItemExt::createSpriteExtra(CCSprite::create("GDH_arrow.png"_spr), [this, corner, tab](auto) {
            auto& targetLabels = GDH::Labels::Manager::get().labels[corner];
            if (m_index < targetLabels.size() - 1) {
                std::swap(targetLabels[m_index], targetLabels[m_index + 1]);
                refreshLabelsTab(tab);
                this->m_closeBtn->activate();
            }
        });
        downBtn->getChildByType<CCSprite>(0)->setRotation(-90.f);
        downBtn->getChildByType<CCSprite>(0)->setScale(0.6f);
        downBtn->setPosition({75.f, y});
        auto& targetLabels = manager.labels[corner];
        downBtn->setEnabled(m_index < targetLabels.size() - 1);
        if (m_index >= targetLabels.size() - 1) downBtn->setOpacity(100);
        menu->addChild(downBtn);

        auto delBtnSprite = ButtonSprite::create("Delete", 60, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 25.f, 0.4f);
        auto delBtn = CCMenuItemExt::createSpriteExtra(delBtnSprite, [this, corner, tab](auto) {
            auto& targetLabels = GDH::Labels::Manager::get().labels[corner];
            targetLabels.erase(targetLabels.begin() + m_index);
            refreshLabelsTab(tab);
            this->m_closeBtn->activate();
        });
        delBtn->setPosition({winSize.width - 55.f, y});
        menu->addChild(delBtn);

        return true;
    }
public:
    static EditLabelPopup* create(GDH::Labels::Corner corner, size_t index, HacksTab* tab) {
        auto ret = new EditLabelPopup();
        if (ret->init(corner, index, tab)) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

void refreshLabelsTab(HacksTab* tab) {
    if (!tab || !tab->m_scrollLayer || !tab->m_scrollLayer->m_contentLayer) return;

    auto content = tab->m_scrollLayer->m_contentLayer;
    content->removeAllChildren();

    content->setLayout(
        ColumnLayout::create()
        ->setAutoScale(false)
        ->setAxisReverse(true)
        ->setAutoGrowAxis(true)
        ->setGap(5.f)
    );
    

    tab->addConfigToggle("Disable All", "labels::disable_all", false);
    auto layout = static_cast<geode::RowLayout*>(tab->m_currentRow->getLayout());
    layout->setPadding({35.f, 0.f, 0.f, 0.f});
    tab->m_currentRow->updateLayout();

    auto& manager = GDH::Labels::Manager::get();

    auto padsMenu = CCMenu::create();
    padsMenu->setContentSize({305.f, 20.f});
    padsMenu->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Start)->setCrossAxisAlignment(AxisAlignment::Center)->setGap(7.f)->setPadding({-10.f,0,0,0}));

    auto padsSpacer = CCNode::create();
    padsSpacer->setContentSize({5.f, 0.f});
    padsMenu->addChild(padsSpacer);

    auto lbl1 = geode::Label::create("Corner Pad:", "GoogleSans.fnt"_spr);
    padsMenu->addChild(lbl1);

    auto input1 = TextInput::create(55.f, "Pad", "GoogleSans.fnt"_spr);
    input1->setString(fmt::format("{:.1f}", manager.cornerPadding));
    input1->setCallback([](const std::string& str) {
        if (auto val = geode::utils::numFromString<float>(str)) {
            GDH::Labels::Manager::get().cornerPadding = std::clamp(val.unwrap(), 0.f, 256.f);
        }
    });
    if (auto* bg = input1->getChildByType<geode::NineSlice>(0)) { 
        bg->setColor({71, 71, 131}); 
        bg->setOpacity(255); 
    }
    padsMenu->addChild(input1);

    auto padSpacer = CCNode::create();
    padSpacer->setContentSize({8.f, 0.f});
    padsMenu->addChild(padSpacer);

    auto lbl2 = geode::Label::create("Mid Pad:", "GoogleSans.fnt"_spr);
    padsMenu->addChild(lbl2);

    auto input2 = TextInput::create(55.f, "Pad", "GoogleSans.fnt"_spr);
    input2->setString(fmt::format("{:.1f}", manager.midPadding));
    input2->setCallback([](const std::string& str) {
        if (auto val = geode::utils::numFromString<float>(str)) {
            GDH::Labels::Manager::get().midPadding = std::clamp(val.unwrap(), 0.f, 256.f);
        }
    });
    if (auto* bg = input2->getChildByType<geode::NineSlice>(0)) { 
        bg->setColor({71, 71, 131}); 
        bg->setOpacity(255); 
    }
    padsMenu->addChild(input2);
    padsMenu->updateLayout();
    content->addChild(padsMenu);

    tab->addSeparator();
    
    std::vector<std::string> cornerNames = {
        "Top Left", "Top Center", "Top Right",
        "Center Left", "Center Center", "Center Right",
        "Bottom Left", "Bottom Center", "Bottom Right"
    };

    GDH::Labels::Corner corner = static_cast<GDH::Labels::Corner>(s_currentCornerIndex);

    auto selectorRow = CCMenu::create();
    selectorRow->setContentSize({340.f, 32.f});
    selectorRow->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Start)->setCrossAxisAlignment(AxisAlignment::Center)->setGap(6.f)->setPadding({-10.f,0,0,0}));

    auto leftSpacer = CCNode::create();
    leftSpacer->setContentSize({5.f, 0.f});
    selectorRow->addChild(leftSpacer);

    auto leftArrowSprite = CCSprite::create("GDH_arrow.png"_spr);
    leftArrowSprite->setScale(0.6f);
    auto leftBtn = CCMenuItemExt::createSpriteExtra(leftArrowSprite, [tab](auto) {
        if (s_currentCornerIndex > 0) s_currentCornerIndex--;
        else s_currentCornerIndex = 8;
        refreshLabelsTab(tab);
    });
    selectorRow->addChild(leftBtn);

    auto labelWidthContainer = CCNode::create();
    labelWidthContainer->setContentSize({110.f, 20.f});

    auto currentCornerLabel = geode::Label::create(cornerNames[s_currentCornerIndex], "GoogleSans.fnt"_spr);
    currentCornerLabel->setScale(0.6f);
    currentCornerLabel->setAlignment(geode::Label::Alignment::Center);

    currentCornerLabel->setPosition({55.f, 10.f}); 
    labelWidthContainer->addChild(currentCornerLabel);

    selectorRow->addChild(labelWidthContainer);

    auto rightArrowSprite = CCSprite::create("GDH_arrow.png"_spr);
    rightArrowSprite->setScale(0.6f);
    rightArrowSprite->setFlipX(true);
    auto rightBtn = CCMenuItemExt::createSpriteExtra(rightArrowSprite, [tab](auto) {
        if (s_currentCornerIndex < 8) s_currentCornerIndex++;
        else s_currentCornerIndex = 0;
        refreshLabelsTab(tab);
    });
    selectorRow->addChild(rightBtn);

    auto spring = CCNode::create();
    spring->setContentSize({75.f, 0.f});
    selectorRow->addChild(spring);

    auto addBtnSprite = ButtonSprite::create("Add Label", 70, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 24.f, 0.6f);
    auto addBtn = CCMenuItemExt::createSpriteExtra(addBtnSprite, [corner, tab](auto) {
        AddLabelPopup::create(corner, tab)->show();
    });
    selectorRow->addChild(addBtn);

    selectorRow->updateLayout();
    content->addChild(selectorRow);

    tab->addSeparator();

    auto& labelsList = manager.labels[corner];
    if (labelsList.empty()) {
        auto emptyLabel = geode::Label::create("No labels added here", "GoogleSans.fnt"_spr);
        emptyLabel->setScale(0.45f);
        emptyLabel->setOpacity(100);
        content->addChild(emptyLabel);
    } else {
        for (size_t idx = 0; idx < labelsList.size(); idx++) {
            auto& label = labelsList[idx];

            auto itemRow = CCMenu::create();
            itemRow->setContentSize({340.f, 20.f}); 
            itemRow->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Start)->setCrossAxisAlignment(AxisAlignment::Center)->setGap(6.f)->setPadding({-10.f,0,0,0}));

            auto itemSpacer = CCNode::create();
            itemSpacer->setContentSize({5.f, 0.f});
            itemRow->addChild(itemSpacer);

            auto editBtnSprite = ButtonSprite::create("Edit", 26.f, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 24.f, 0.6f);
            auto editBtn = CCMenuItemExt::createSpriteExtra(editBtnSprite, [corner, idx, tab](auto) {
                EditLabelPopup::create(corner, idx, tab)->show();
            });
            itemRow->addChild(editBtn);

            std::string displayText = (label.type == GDH::Labels::LabelType::Text) ? (label.text.empty() ? "Empty Text" : label.text)
                : fmt::format("{}: {:.2f}px", (label.type == GDH::Labels::LabelType::Spacing) ? "Spacing" : "Cheat Indicator", label.size);

            auto textLabel = geode::Label::create(displayText, "GoogleSans.fnt"_spr);
            itemRow->addChild(textLabel);
            
            itemRow->updateLayout();
            content->addChild(itemRow);
        }
    }

    content->updateLayout();
    
    tab->m_scrollLayer->updateLayout();
    tab->m_scrollLayer->moveToTop();
}

$execute {
    auto& gui = GDH::Gui::get();
    auto& window = gui.getWindow("Labels");

    window.setCustomWindowCocos([](cocos2d::CCNode* node) {
        auto tab = static_cast<HacksTab*>(node);

        geode::queueInMainThread([tab]() {
            refreshLabelsTab(tab);
        });
    });
}