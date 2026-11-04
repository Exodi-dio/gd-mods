#include "hack_settings_popup.hpp"
#include "../../core/config.hpp"
#include "../../core/utils.hpp"

void updatePopupRowAlignment(cocos2d::CCMenu* row) {
    if (!row) return;
    if (auto layout = static_cast<geode::RowLayout*>(row->getLayout())) {
        row->updateLayout(); 
    }
}

HackSettingsPopup* HackSettingsPopup::create(GDH::Hack& hack) {
    auto ret = new HackSettingsPopup();
    if (ret->init(hack)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool HackSettingsPopup::init(GDH::Hack& hack) {
    if (!geode::Popup::init(260.f, 230.f, "GDH_square.png"_spr)) return false;

    m_hack = &hack;
    auto winSize = m_mainLayer->getContentSize();

    auto title = geode::Label::create(hack.getName() + " Settings", "GoogleSans.fnt"_spr);
    title->setPosition({winSize.width / 2.f, winSize.height - 18.f});
    title->setScale(0.6f);
    m_mainLayer->addChild(title);

    auto closeBtn = cocos2d::CCSprite::create("GDH_closeBtn.png"_spr);
    closeBtn->setScale(0.75f);
    m_closeBtn->setSprite(closeBtn);

    m_scrollLayer = geode::prelude::ScrollLayer::create({240.f, 190.f});
    m_scrollLayer->setPosition({winSize.width / 2.f - 120.f, 10.f});
    m_scrollLayer->m_contentLayer->setLayout(
        geode::ColumnLayout::create()
            ->setAutoScale(false)
            ->setAxisReverse(true)
            ->setAutoGrowAxis(true)
            ->setGap(6.f)
    );
    m_mainLayer->addChild(m_scrollLayer);

    m_scrollLayer->m_contentLayer->addChild(cocos2d::CCNode::create());

    if (hack.avaibleCustomWindowCocos()) {
        hack.callCustomWindowCocos(this);
    }

    m_scrollLayer->m_contentLayer->addChild(cocos2d::CCNode::create());

    m_scrollLayer->m_contentLayer->updateLayout();
    m_scrollLayer->updateLayout();
    m_scrollLayer->moveToTop();

    return true;
}

void HackSettingsPopup::prepareNewRow() {
    m_currentRow = cocos2d::CCMenu::create();
    m_currentRow->setContentSize({230.f, 32.f});
    m_currentRow->setLayout(
        geode::RowLayout::create()
            ->setGap(8.f)
            ->setAxisAlignment(geode::AxisAlignment::Center)
            ->setCrossAxisAlignment(geode::AxisAlignment::Center)
            ->setAutoScale(false)
    );
    m_scrollLayer->m_contentLayer->addChild(m_currentRow);
}

void HackSettingsPopup::addConfigToggle(const std::string& labelText, const std::string& key, bool defaultValue, geode::Function<void(bool)> callback) {
    auto& config = Config::get();
    prepareNewRow();
    
    auto toggle = CCMenuItemExt::createTogglerWithFilename("GDH_togglerOn.png"_spr, "GDH_togglerOff.png"_spr, 0.8f, [key, callback = std::move(callback)](CCMenuItemToggler* sender) mutable {
        bool newValue = !sender->isOn();
        Config::get().set<bool>(key, newValue);
        if (callback) {
            callback(newValue);
        }
    });
    toggle->toggle(config.get<bool>(key, defaultValue));
    m_currentRow->addChild(toggle);

    auto label = geode::Label::create(labelText, "GoogleSans.fnt"_spr);
    label->setScale(0.55f); 
    if (label->getScaledContentSize().width > 160.f) {
        label->setScale(160.f / label->getContentSize().width);
    }
    m_currentRow->addChild(label);

    m_currentRow->updateLayout();
    updatePopupRowAlignment(m_currentRow);
}

void HackSettingsPopup::addConfigIntInput(const std::string& labelText, const std::string& key, int min, int max, int defaultValue, geode::Function<void(int)> callback) {
    auto& config = Config::get();
    prepareNewRow();
    
    auto input = geode::TextInput::create(50.f, "Val", "GoogleSans.fnt"_spr);
    if (auto* bg = input->getChildByType<geode::NineSlice>(0)) { 
        bg->setColor({71, 71, 131}); 
        bg->setOpacity(255); 
    }
    input->setScale(0.65f);
    input->setString(std::to_string(config.get<int>(key, defaultValue)));
    input->setFilter("0123456789-");
    input->setMaxCharCount(6);
    
    input->setCallback([key, min, max, callback = std::move(callback)](const std::string& str) mutable {
        auto value = geode::utils::numFromString<int>(str);
        if (!value.isErr()) {
            int val = std::clamp(value.unwrap(), min, max);
            Config::get().set<int>(key, val);
            
            if (callback) {
                callback(val);
            }
        }
    });
    m_currentRow->addChild(input);
    
    auto label = geode::Label::create(labelText, "GoogleSans.fnt"_spr);
    label->setScale(0.55f);
    if (label->getScaledContentSize().width > 160.f) {
        label->setScale(160.f / label->getContentSize().width);
    }
    m_currentRow->addChild(label);
    
    m_currentRow->updateLayout();
    updatePopupRowAlignment(m_currentRow);
}

void HackSettingsPopup::addConfigFloatInput(const std::string& labelText, const std::string& key, float min, float max, float defaultValue, geode::Function<void(float)> callback) {
    auto& config = Config::get();
    prepareNewRow();
    
    auto input = geode::TextInput::create(50.f, "Val", "GoogleSans.fnt"_spr);
    if (auto* bg = input->getChildByType<geode::NineSlice>(0)) { 
        bg->setColor({71, 71, 131}); 
        bg->setOpacity(255); 
    }
    input->setScale(0.65f);
    input->setString(fmt::format("{:.2f}", config.get<float>(key, defaultValue)));
    input->setFilter("0123456789-.");
    input->setMaxCharCount(6);
    
    input->setCallback([key, min, max, callback = std::move(callback)](const std::string& str) mutable {
        auto value = geode::utils::numFromString<float>(str);
        if (!value.isErr()) {
            float val = std::clamp(value.unwrap(), min, max);
            Config::get().set<float>(key, val);
            
            if (callback) {
                callback(val);
            }
        }
    });
    m_currentRow->addChild(input);

    auto label = geode::Label::create(labelText, "GoogleSans.fnt"_spr);
    label->setScale(0.55f);
    if (label->getScaledContentSize().width > 160.f) {
        label->setScale(160.f / label->getContentSize().width);
    }
    m_currentRow->addChild(label);

    m_currentRow->updateLayout();
    updatePopupRowAlignment(m_currentRow);
}

void HackSettingsPopup::addConfigColor3Hex(const std::string& labelText, const std::string& key, const std::string& defaultHex) {
    auto& config = Config::get();
    prepareNewRow();

    std::string currentHex = config.get<std::string>(key, defaultHex);
    cocos2d::ccColor3B color = GDH::Utils::hexToColor(currentHex);

    auto colorSprite = ColorChannelSprite::create();
    colorSprite->setScale(0.6f);
    colorSprite->setColor(color);

    auto colorBtn = CCMenuItemExt::createSpriteExtra(colorSprite, [this, key, defaultHex, colorSprite](CCMenuItemSpriteExtra*) {
        std::string curHex = Config::get().get<std::string>(key, defaultHex);
        cocos2d::ccColor3B curColor = GDH::Utils::hexToColor(curHex);

        cocos2d::ccColor4B popupColor = { curColor.r, curColor.g, curColor.b, 255 };

        auto popup = CompatibleColorPopup::create(popupColor, false);
        popup->setCallback([weakPopup = geode::WeakRef(this), key, colorSprite](cocos2d::ccColor4B const& formatColor) {
            if (!weakPopup.lock()) return;
            
            std::string newHex = fmt::format("{:02X}{:02X}{:02X}", formatColor.r, formatColor.g, formatColor.b);
            Config::get().set<std::string>(key, newHex);
            colorSprite->setColor(geode::cocos::to3B(formatColor));
        });
        popup->show();
    });
    m_currentRow->addChild(colorBtn);

    auto label = geode::Label::create(labelText, "GoogleSans.fnt"_spr);
    label->setScale(0.55f);
    if (label->getScaledContentSize().width > 120.f) {
        label->setScale(120.f / label->getContentSize().width);
    }
    m_currentRow->addChild(label);

    auto resetBtnSprite = ButtonSprite::create("Reset", 40, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 25.f, 0.4f);
    auto resetBtn = CCMenuItemExt::createSpriteExtra(resetBtnSprite, [key, defaultHex, colorSprite](CCMenuItemSpriteExtra*) {
        Config::get().set<std::string>(key, defaultHex);
        cocos2d::ccColor3B defColor = GDH::Utils::hexToColor(defaultHex);
        colorSprite->setColor(defColor);
    });
    m_currentRow->addChild(resetBtn);

    m_currentRow->updateLayout();
    updatePopupRowAlignment(m_currentRow);
}

void HackSettingsPopup::addConfigColor4Hex(const std::string& labelText, const std::string& key, const std::string& defaultHex) {
    auto& config = Config::get();
    prepareNewRow();

    std::string currentHex = config.get<std::string>(key, defaultHex);
    cocos2d::ccColor4F color4F = GDH::Utils::hexToColor4F(currentHex);

    cocos2d::ccColor3B color3B = {
        static_cast<GLubyte>(color4F.r * 255.f),
        static_cast<GLubyte>(color4F.g * 255.f),
        static_cast<GLubyte>(color4F.b * 255.f)
    };
    GLubyte alphaByte = static_cast<GLubyte>(color4F.a * 255.f);

    auto colorSprite = ColorChannelSprite::create();
    colorSprite->setScale(0.6f);
    colorSprite->setColor(color3B);
    colorSprite->setOpacity(alphaByte);

    auto colorBtn = CCMenuItemExt::createSpriteExtra(colorSprite, [this, key, defaultHex, colorSprite](CCMenuItemSpriteExtra*) {
        std::string curHex = Config::get().get<std::string>(key, defaultHex);
        cocos2d::ccColor4F cur4F = GDH::Utils::hexToColor4F(curHex);

        cocos2d::ccColor4B popupColor = {
            static_cast<GLubyte>(cur4F.r * 255.f),
            static_cast<GLubyte>(cur4F.g * 255.f),
            static_cast<GLubyte>(cur4F.b * 255.f),
            static_cast<GLubyte>(cur4F.a * 255.f)
        };

        auto popup = CompatibleColorPopup::create(popupColor, true); 
        popup->setCallback([weakPopup = geode::WeakRef(this), key, colorSprite](cocos2d::ccColor4B const& formatColor) {
            if (!weakPopup.lock()) return;
            
            std::string newHex = fmt::format("{:02X}{:02X}{:02X}{:02X}", formatColor.r, formatColor.g, formatColor.b, formatColor.a);
            Config::get().set<std::string>(key, newHex);
            
            colorSprite->setColor(geode::cocos::to3B(formatColor));
            colorSprite->setOpacity(formatColor.a);
        });
        popup->show();
    });
    m_currentRow->addChild(colorBtn);

    auto label = geode::Label::create(labelText, "GoogleSans.fnt"_spr);
    label->setScale(0.55f);
    if (label->getScaledContentSize().width > 120.f) {
        label->setScale(120.f / label->getContentSize().width);
    }
    m_currentRow->addChild(label);

    auto resetBtnSprite = ButtonSprite::create("Reset", 40, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 25.f, 0.4f);
    auto resetBtn = CCMenuItemExt::createSpriteExtra(resetBtnSprite, [key, defaultHex, colorSprite](CCMenuItemSpriteExtra*) {
        Config::get().set<std::string>(key, defaultHex);
        cocos2d::ccColor4F def4F = GDH::Utils::hexToColor4F(defaultHex);
        
        colorSprite->setColor({
            static_cast<GLubyte>(def4F.r * 255.f),
            static_cast<GLubyte>(def4F.g * 255.f),
            static_cast<GLubyte>(def4F.b * 255.f)
        });
        colorSprite->setOpacity(static_cast<GLubyte>(def4F.a * 255.f));
    });
    m_currentRow->addChild(resetBtn);

    m_currentRow->updateLayout();
    updatePopupRowAlignment(m_currentRow);
}

void HackSettingsPopup::addSeparator(float height) {
    m_currentRow = nullptr; 
    auto node = cocos2d::CCLayerColor::create({104, 104, 191, 80});
    node->setContentSize({230.f, height});
    m_scrollLayer->m_contentLayer->addChild(node);
}