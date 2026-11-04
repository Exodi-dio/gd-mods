#include "hacks_tab.hpp"
#include "../../core/gui.hpp"
#include "../../core/config.hpp"
#include "hack_settings_popup.hpp"

HacksTab* HacksTab::create() {
    auto ret = new HacksTab();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void updateRowAlignment(cocos2d::CCMenu* row) {
    if (!row) return;
    if (auto layout = static_cast<geode::RowLayout*>(row->getLayout())) {
        if (row->getChildrenCount() == 1) {
            layout->setAxisAlignment(geode::AxisAlignment::Center);
        } else {
            layout->setAxisAlignment(geode::AxisAlignment::Start);
        }
        row->updateLayout();
    }
}

void HacksTab::addToggle(GDH::Hack& hck) {
    auto& gui = GDH::Gui::get();
    const std::string ID = hck.getID();
    
    float columnWidth = 175.f; 
    bool disabled = hck.getDisabled();

    auto hackNode = CCMenu::create();
    hackNode->setContentSize({columnWidth, 30.f});
    hackNode->setAnchorPoint({0, 0.5f});

    auto toggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f, [&gui, ID](CCMenuItemToggler* sender) {
        auto* hack = gui.findHackByIDGlobal(ID);
        hack->toggle();
    });
    toggle->setPosition({25.f, 15.f});
    toggle->toggle(hck.getEnabled());
    
    if (disabled) {
        toggle->setEnabled(false);
        toggle->setOpacity(100);
    }

    hackNode->addChild(toggle);

    std::string name = hck.getName();
    auto label = geode::Label::create(name, "GoogleSans.fnt"_spr);
    label->setAnchorPoint({0.f, 0.5f});
    label->setScale(0.65f); 
    label->setPosition({toggle->getPositionX() + 22.f, 15.f});
    if (disabled) label->setOpacity(100);
    if (hck.isCheating()) label->setColor({255, 128, 128});

    float maxLabelWidth = columnWidth - label->getPositionX() - 45.f; 
    if (label->getScaledContentWidth() > maxLabelWidth) {
        label->setScale(label->getScale() * (maxLabelWidth / label->getScaledContentWidth()));
    }
    hackNode->addChild(label);

    float iconXOffset = label->getPositionX() + label->getScaledContentWidth() + 13.f;
    if (hck.avaibleCustomWindowCocos()) {
        auto customSettingsBtn = CCSprite::create("GDH_settingsBtn.png"_spr);
        customSettingsBtn->setScale(0.45f);

        auto customSettingsBtnClick = CCMenuItemExt::createSpriteExtra(customSettingsBtn, [&hck](CCMenuItemSpriteExtra* sender) {
            if (auto popup = HackSettingsPopup::create(hck)) {
                popup->show();
            }
        });
        
        customSettingsBtnClick->setPosition({iconXOffset, 15.f});
        hackNode->addChild(customSettingsBtnClick);
        iconXOffset += 18.5f;
    }

    std::string desc = hck.getDesc();
    if (!desc.empty()) {
        auto descSprite = CCSprite::create("GDH_infoIcon.png"_spr);
        descSprite->setScale(0.5f);
        auto descClick = CCMenuItemExt::createSpriteExtra(descSprite, [name, desc](CCMenuItemSpriteExtra* sender) {
            GDH::MaterialLayer(FLAlertLayer::create(name.c_str(), desc.c_str(), "OK"))->show();
        });
        
        descClick->setPosition({iconXOffset, 15.f});
        hackNode->addChild(descClick);
    }

    if (!m_currentRow || m_currentRow->getChildrenCount() >= 2) {
        prepareNewRow();
    }

    m_currentRow->addChild(hackNode);
    updateRowAlignment(m_currentRow);
}

bool HacksTab::init() {
    if (!CCMenu::init())
        return false;

    setPosition({0, 0});

    m_scrollLayer = ScrollLayer::create({500.f, 250.f});
    m_scrollLayer->m_contentLayer->setLayout(
        geode::ColumnLayout::create()
            ->setAutoScale(false)
            ->setAxisReverse(true)
            ->setAutoGrowAxis(false)
            ->setGap(0.f)
    );
    m_scrollLayer->setPosition({107.f, 5.f});
    m_scrollLayer->m_peekLimitTop = 15;
    m_scrollLayer->m_peekLimitBottom = 15;
    
    addChild(m_scrollLayer);
    
    return true;
}

void HacksTab::addPadding(float height) {
    m_currentRow = nullptr;
    auto node = cocos2d::CCNode::create();
    node->setContentHeight(height);
    m_scrollLayer->m_contentLayer->addChild(node);
}

void HacksTab::addSeparator(float height) {
    m_currentRow = nullptr; 
    auto node = cocos2d::CCLayerColor::create({104, 104, 191, 80});
    node->setContentSize({335.f, height});
    m_scrollLayer->m_contentLayer->addChild(node);
}

void HacksTab::prepareNewRow() {
    float columnWidth = 175.f;
    m_currentRow = CCMenu::create();
    m_currentRow->setContentSize({columnWidth * 2 + 5.f, 32.f});
    m_currentRow->setLayout(
        geode::RowLayout::create()
            ->setGap(0.f)
            ->setAxisAlignment(geode::AxisAlignment::Start)
            ->setCrossAxisAlignment(geode::AxisAlignment::Center)
            ->setAutoScale(false)
    );
    m_scrollLayer->m_contentLayer->addChild(m_currentRow);
}

void HacksTab::addHackToggle(const std::string& labelText, const std::string& key, bool defaultValue, geode::Function<void(bool)> callback) {
    auto& config = Config::get();
    float columnWidth = 175.f;

    auto node = CCMenu::create();
    node->setContentSize({columnWidth, 30.f});
    node->setAnchorPoint({0, 0.5f});
    
    auto toggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f, [key, callback = std::move(callback)](CCMenuItemToggler* sender) mutable {
        auto& gui = GDH::Gui::get();
        auto* hack = gui.findHackByIDGlobal(key);
        if (hack != nullptr)
            hack->toggle();
        
        if (callback) {
            callback(hack->getEnabled());
        }
    });
    toggle->setPosition({25.f, 15.f});
    toggle->toggle(config.get<bool>(key, defaultValue));
    node->addChild(toggle);

    auto label = geode::Label::create(labelText, "GoogleSans.fnt"_spr);
    label->setAnchorPoint({0.f, 0.5f});
    label->setScale(0.65f); 
    label->setPosition({toggle->getPositionX() + 22.f, 15.f});
    node->addChild(label);

    if (!m_currentRow || m_currentRow->getChildrenCount() >= 2) prepareNewRow();
    m_currentRow->addChild(node);
    updateRowAlignment(m_currentRow);
}

void HacksTab::addConfigToggle(
    const std::string& labelText, 
    const std::string& key, 
    bool defaultValue, 
    geode::Function<void(bool)> callback
) {
    auto& config = Config::get();
    float columnWidth = 175.f;

    auto node = CCMenu::create();
    node->setContentSize({columnWidth, 30.f});
    node->setAnchorPoint({0, 0.5f});
    
    auto toggle = CCMenuItemExt::createTogglerWithFilename(
        "GDH_togglerOn.png"_spr, 
        "GDH_togglerOff.png"_spr, 
        0.8f, 
        [key, callback = std::move(callback)](CCMenuItemToggler* sender) mutable {
            bool newValue = !sender->isOn();
            Config::get().set<bool>(key, newValue);
            if (callback) {
                callback(newValue);
            }
        }
    );
    toggle->setPosition({25.f, 15.f});
    toggle->toggle(config.get<bool>(key, defaultValue));
    node->addChild(toggle);

    auto label = geode::Label::create(labelText, "GoogleSans.fnt"_spr);
    label->setAnchorPoint({0.f, 0.5f});
    label->setScale(0.65f); 
    label->setPosition({toggle->getPositionX() + 22.f, 15.f});
    node->addChild(label);

    if (!m_currentRow || m_currentRow->getChildrenCount() >= 2) prepareNewRow();
    m_currentRow->addChild(node);
    updateRowAlignment(m_currentRow);
}

void HacksTab::addConfigIntInput(const std::string& labelText, const std::string& key, int defaultValue, int min, int max, geode::Function<void(int)> callback) {
    auto& config = Config::get();
    float columnWidth = 175.f;
    
    auto node = CCMenu::create();
    node->setContentSize({columnWidth, 30.f});
    
    auto input = geode::TextInput::create(60.f, "Val", "GoogleSans.fnt"_spr);
    if (auto* bg = input->getChildByType<geode::NineSlice>(0)) { 
        bg->setColor({71, 71, 131}); 
        bg->setOpacity(255); 
    }
    input->setScale(0.7f);
    input->setPosition({30.f, 15.f});
    input->setString(std::to_string(config.get<int>(key, defaultValue)));
    input->setFilter("0123456789-");
    input->setMaxCharCount(8);
    
    input->setCallback([key, callback = std::move(callback), min, max](const std::string& str) mutable {
        auto value = geode::utils::numFromString<int>(str);
        if (!value.isErr()) {
            int val = std::clamp(value.unwrap(), min, max);
            Config::get().set<int>(key, val);
            if (callback) callback(val);
        }
    });
    node->addChild(input);
    
    auto label = geode::Label::create(labelText, "GoogleSans.fnt"_spr);
    label->setAnchorPoint({0.f, 0.5f});
    label->setScale(0.55f);
    label->setPosition({60.f, 15.f});
    node->addChild(label);
    
    if (!m_currentRow || m_currentRow->getChildrenCount() >= 2) prepareNewRow();
    m_currentRow->addChild(node);
    updateRowAlignment(m_currentRow);
}

void HacksTab::addConfigFloatInput(const std::string& labelText, const std::string& key, float defaultValue, float min, float max, geode::Function<void(float)> callback) {
    auto& config = Config::get();
    float columnWidth = 175.f;
    
    auto node = CCMenu::create();
    node->setContentSize({columnWidth, 30.f});
    
    auto input = geode::TextInput::create(60.f, "Val", "GoogleSans.fnt"_spr);
    if (auto* bg = input->getChildByType<geode::NineSlice>(0)) { 
        bg->setColor({71, 71, 131}); 
        bg->setOpacity(255); 
    }
    input->setScale(0.7f);
    input->setPosition({30.f, 15.f});
    input->setString(fmt::format("{:.2f}", config.get<float>(key, defaultValue)));
    input->setFilter("0123456789-.");
    input->setMaxCharCount(8);
    
    input->setCallback([key, callback = std::move(callback), min, max](const std::string& str) mutable {
        auto value = geode::utils::numFromString<float>(str);
        if (!value.isErr()) {
            float val = std::clamp(value.unwrap(), min, max);
            Config::get().set<float>(key, val);
            if (callback) callback(val);
        }
    });
    node->addChild(input);

    auto label = geode::Label::create(labelText, "GoogleSans.fnt"_spr);
    label->setAnchorPoint({0.f, 0.5f});
    label->setScale(0.55f);
    label->setPosition({60.f, 15.f});
    node->addChild(label);

    if (!m_currentRow || m_currentRow->getChildrenCount() >= 2) prepareNewRow();
    m_currentRow->addChild(node);
    updateRowAlignment(m_currentRow);
}

void HacksTab::addText(const std::string& text, float scale) {
    auto label = geode::Label::create(text, "GoogleSans.fnt"_spr);
    label->setScale(scale);
    
    prepareNewRow(); 
    m_currentRow->addChild(label);
    
    if (auto layout = static_cast<geode::RowLayout*>(m_currentRow->getLayout())) {
        layout->setAxisAlignment(geode::AxisAlignment::Center);
        m_currentRow->updateLayout();
    }
}

void HacksTab::addConfigButton(const std::string& labelText, geode::Function<void()> callback, const std::string& secondLabelText, geode::Function<void()> secondCallback) {
    float columnWidth = 170.f;
    bool dualMode = !secondLabelText.empty() && secondCallback != nullptr;

    prepareNewRow();

    if (dualMode) {
        auto createHalfNode = [columnWidth](const std::string& text, geode::Function<void()> cb) -> CCMenu* {
            auto node = CCMenu::create();
            node->setContentSize({columnWidth, 30.f});
            node->setAnchorPoint({0, 0.5f});

            auto btnSprite = ButtonSprite::create(text.c_str(), 140, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 25.f, 0.6f);
            auto btnClick = CCMenuItemExt::createSpriteExtra(btnSprite, [cb = std::move(cb)](CCMenuItemSpriteExtra* sender) mutable {
                if (cb) cb();
            });
            btnClick->setPosition({columnWidth / 2.f - 5.f, 15.f});
            node->addChild(btnClick);
            return node;
        };

        m_currentRow->addChild(createHalfNode(labelText, std::move(callback)));
        m_currentRow->addChild(createHalfNode(secondLabelText, std::move(secondCallback)));
    } 
    else {
        auto node = CCMenu::create();
        node->setContentSize({columnWidth * 2, 30.f});
        node->setAnchorPoint({0, 0.5f});

        auto btnSprite = ButtonSprite::create(labelText.c_str(), 315, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 25.f, 0.6f);
        auto btnClick = CCMenuItemExt::createSpriteExtra(btnSprite, [callback = std::move(callback)](CCMenuItemSpriteExtra* sender) mutable {
            if (callback) callback();
        });
        
        btnClick->setPosition({columnWidth - 5.f, 15.f});
        node->addChild(btnClick);

        m_currentRow->addChild(node);
    }

    if (auto layout = static_cast<geode::RowLayout*>(m_currentRow->getLayout())) {
        layout->setAxisAlignment(geode::AxisAlignment::Center);
        m_currentRow->updateLayout();
    }

    m_currentRow = nullptr;
}

geode::Label* HacksTab::AddTextToToggle(const char *str, CCMenuItemToggler* toggler, float x_space) {
    auto label = geode::Label::create(str, "GoogleSans.fnt"_spr);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition({toggler->getPositionX() + x_space, toggler->getPositionY()});
    label->setScale(0.65f);
    return label;
}