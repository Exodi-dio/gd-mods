#include <Geode/Geode.hpp>
#include "../hacks_tab.hpp"
#include "../../../core/gui.hpp"

using namespace geode::prelude;

static int s_typeIndex = 0;
static int s_creatorIndex = 0;
static int s_playerIndex = 0;
static std::string s_inputValue = "";

static const std::vector<std::string> s_typeItems = {
    "Creator", "Player"
};

static const std::vector<std::string> s_creatorItems = {
    "Object ID"
};

static const std::vector<std::string> s_playerItems = {
    "Attempts", "Jumps", "Normal %", "Position X", "Position Y", 
    "Practice %", "Song ID", "Speed", "Level ID"
};

void refreshVariablesTab(HacksTab* tab);

template <typename T>
static void processInt(T&& target, bool isSet, geode::TextInput* input) {
    if (isSet) {
        if (auto res = geode::utils::numFromString<int>(input->getString())) {
            target = res.unwrap();
        }
    } else {
        s_inputValue = std::to_string(static_cast<int>(target));
        input->setString(s_inputValue);
    }
}

static void processFloat(float& target, bool isSet, geode::TextInput* input) {
    if (isSet) {
        if (auto res = geode::utils::numFromString<float>(input->getString())) {
            target = res.unwrap();
        }
    } else {
        s_inputValue = fmt::format("{:.2f}", target);
        input->setString(s_inputValue);
    }
}

static void executeGetSet(bool isSet, geode::TextInput* input) {
    if (s_typeIndex == 0) {
        if (auto editor = EditorUI::get()) {
            switch (s_creatorIndex) {
                case 0: processInt(editor->m_selectedObjectIndex, isSet, input); break;
            }
        } else {
            GDH::MaterialLayer(FLAlertLayer::create("Error", "EditorUI is not available", "OK"))->show();
        }
    } else {
        if (auto pl = PlayLayer::get()) {
            if (!pl->m_level || !pl->m_player1) {
                GDH::MaterialLayer(FLAlertLayer::create("Error", "Player/Level object is null", "OK"))->show();
                return;
            }
            auto& level = *pl->m_level;
            auto& player = *pl->m_player1;

            switch (s_playerIndex) {
                case 0: processInt(level.m_attempts, isSet, input); break;
                case 1: processInt(level.m_jumps, isSet, input); break;
                case 2: processInt(level.m_normalPercent, isSet, input); break;
                case 3: processFloat(player.m_position.x, isSet, input); break;
                case 4: processFloat(player.m_position.y, isSet, input); break;
                case 5: processInt(level.m_practicePercent, isSet, input); break;
                case 6: processInt(level.m_songID, isSet, input); break;
                case 7: processFloat(player.m_playerSpeed, isSet, input); break;
                case 8: processInt(level.m_levelID, isSet, input); break;
            }
        } else {
            GDH::MaterialLayer(FLAlertLayer::create("Error", "PlayLayer is not available", "OK"))->show();
        }
    }
}

void refreshVariablesTab(HacksTab* tab) {
    if (!tab || !tab->m_scrollLayer || !tab->m_scrollLayer->m_contentLayer) return;

    auto content = tab->m_scrollLayer->m_contentLayer;
    content->removeAllChildren();

    content->setLayout(
        ColumnLayout::create()
            ->setAutoScale(false)
            ->setAxisReverse(true)
            ->setAutoGrowAxis(true)
            ->setGap(6.f)
    );

    auto typeSelectorRow = CCMenu::create();
    typeSelectorRow->setContentSize({340.f, 32.f});
    typeSelectorRow->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Center)->setCrossAxisAlignment(AxisAlignment::Center)->setGap(8.f));

    auto typeLeftArrow = CCSprite::create("GDH_arrow.png"_spr);
    typeLeftArrow->setScale(0.6f);
    auto typeLeftBtn = CCMenuItemExt::createSpriteExtra(typeLeftArrow, [tab](auto) {
        if (s_typeIndex > 0) s_typeIndex--;
        else s_typeIndex = static_cast<int>(s_typeItems.size()) - 1;
        refreshVariablesTab(tab);
    });
    typeSelectorRow->addChild(typeLeftBtn);

    auto typeLabelContainer = CCNode::create();
    typeLabelContainer->setContentSize({140.f, 20.f});

    auto typeLabel = geode::Label::create(s_typeItems[s_typeIndex], "GoogleSans.fnt"_spr);
    typeLabel->setScale(0.6f);
    typeLabel->setAlignment(geode::Label::Alignment::Center);
    typeLabel->setPosition({70.f, 10.f});
    typeLabelContainer->addChild(typeLabel);
    typeSelectorRow->addChild(typeLabelContainer);

    auto typeRightArrow = CCSprite::create("GDH_arrow.png"_spr);
    typeRightArrow->setScale(0.6f);
    typeRightArrow->setFlipX(true);
    auto typeRightBtn = CCMenuItemExt::createSpriteExtra(typeRightArrow, [tab](auto) {
        if (s_typeIndex < static_cast<int>(s_typeItems.size()) - 1) s_typeIndex++;
        else s_typeIndex = 0;
        refreshVariablesTab(tab);
    });
    typeSelectorRow->addChild(typeRightBtn);

    typeSelectorRow->updateLayout();
    content->addChild(typeSelectorRow);

    const auto& currentItems = (s_typeIndex == 0) ? s_creatorItems : s_playerItems;
    int& currentIndex = (s_typeIndex == 0) ? s_creatorIndex : s_playerIndex;

    auto varSelectorRow = CCMenu::create();
    varSelectorRow->setContentSize({340.f, 32.f});
    varSelectorRow->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Center)->setCrossAxisAlignment(AxisAlignment::Center)->setGap(8.f));

    auto varLeftArrow = CCSprite::create("GDH_arrow.png"_spr);
    varLeftArrow->setScale(0.6f);
    auto varLeftBtn = CCMenuItemExt::createSpriteExtra(varLeftArrow, [tab, &currentItems, &currentIndex](auto) {
        if (currentIndex > 0) currentIndex--;
        else currentIndex = static_cast<int>(currentItems.size()) - 1;
        refreshVariablesTab(tab);
    });
    varSelectorRow->addChild(varLeftBtn);

    auto varLabelContainer = CCNode::create();
    varLabelContainer->setContentSize({140.f, 20.f});

    auto varLabel = geode::Label::create(currentItems[currentIndex], "GoogleSans.fnt"_spr);
    varLabel->setScale(0.6f);
    varLabel->setAlignment(geode::Label::Alignment::Center);
    varLabel->setPosition({70.f, 10.f});
    varLabelContainer->addChild(varLabel);
    varSelectorRow->addChild(varLabelContainer);

    auto varRightArrow = CCSprite::create("GDH_arrow.png"_spr);
    varRightArrow->setScale(0.6f);
    varRightArrow->setFlipX(true);
    auto varRightBtn = CCMenuItemExt::createSpriteExtra(varRightArrow, [tab, &currentItems, &currentIndex](auto) {
        if (currentIndex < static_cast<int>(currentItems.size()) - 1) currentIndex++;
        else currentIndex = 0;
        refreshVariablesTab(tab);
    });
    varSelectorRow->addChild(varRightBtn);

    varSelectorRow->updateLayout();
    content->addChild(varSelectorRow);

    tab->addSeparator();

    auto inputRow = CCMenu::create();
    inputRow->setContentSize({340.f, 30.f});

    auto valueInput = geode::TextInput::create(200.f, "Value", "GoogleSans.fnt"_spr);
    if (auto* bg = valueInput->getChildByType<geode::NineSlice>(0)) { 
        bg->setColor({71, 71, 131}); 
        bg->setOpacity(255); 
    }
    valueInput->setPosition({170.f, 15.f});
    valueInput->setString(s_inputValue);
    valueInput->setCallback([](const std::string& text) {
        s_inputValue = text;
    });
    inputRow->addChild(valueInput);
    content->addChild(inputRow);

    auto actionMenu = CCMenu::create();
    actionMenu->setContentSize({340.f, 25.f});

    auto getBtnSpr = ButtonSprite::create("Get", 80, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 26.f, 0.55f);
    auto getBtn = CCMenuItemExt::createSpriteExtra(getBtnSpr, [valueInput](auto) {
        executeGetSet(false, valueInput);
    });
    getBtn->setPosition({120.f, 12.5f});
    actionMenu->addChild(getBtn);

    auto setBtnSpr = ButtonSprite::create("Set", 80, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 26.f, 0.55f);
    auto setBtn = CCMenuItemExt::createSpriteExtra(setBtnSpr, [valueInput](auto) {
        executeGetSet(true, valueInput);
    });
    setBtn->setPosition({220.f, 12.5f});
    actionMenu->addChild(setBtn);

    content->addChild(actionMenu);

    content->updateLayout();
    tab->m_scrollLayer->updateLayout();
    tab->m_scrollLayer->moveToTop();
}

$execute {
    auto& gui = GDH::Gui::get();
    auto& window = gui.getWindow("Variables");

    window.setCustomWindowCocos([](cocos2d::CCNode* node) {
        auto tab = static_cast<HacksTab*>(node);
        refreshVariablesTab(tab);
    });
}