#include <Geode/Geode.hpp>
#include "../hacks_tab.hpp"
#include "../../../core/gui.hpp"
#include "../../../core/config.hpp"
#include "../../../core/utils.hpp"
#include <algorithm>
#include <cctype>

using namespace geode::prelude;

namespace {
    bool containsIgnoreCase(std::string str, std::string query) {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { 
            return static_cast<char>(std::tolower(c)); 
        });
        std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) { 
            return static_cast<char>(std::tolower(c)); 
        });
        return str.find(query) != std::string::npos;
    }

    void refreshSearchResults(HacksTab* tab, const std::string& searchQuery) {
        if (!tab || !tab->m_scrollLayer || !tab->m_scrollLayer->m_contentLayer) return;

        auto content = tab->m_scrollLayer->m_contentLayer;

        std::vector<cocos2d::CCNode*> toRemove;
        auto children = content->getChildren();
        if (children && children->count() > 1) {
            for (unsigned int i = 1; i < children->count(); i++) {
                toRemove.push_back(static_cast<cocos2d::CCNode*>(children->objectAtIndex(i)));
            }
        }

        for (auto* child : toRemove) {
            content->removeChild(child, true);
        }

        tab->m_currentRow = nullptr;

        if (searchQuery.empty()) {
            tab->addText("Type a query and press Search", 0.45f);
        } else {
            auto& gui = GDH::Gui::get();
            auto& config = Config::get();
            size_t matchCount = 0;

            if (containsIgnoreCase("tps", searchQuery) || containsIgnoreCase("tps enabled", searchQuery) || containsIgnoreCase("tps value", searchQuery)) {
                tab->addConfigFloatInput("TPS Value", "invisible.tps::value", 240.f, 1.f, FLT_MAX);
                tab->addHackToggle("TPS Enabled", "invisible.tps");
                matchCount++;
            }

            #if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_ANDROID64)
            if (containsIgnoreCase("lock delta", searchQuery) || containsIgnoreCase("lockdt", searchQuery) || containsIgnoreCase("delta", searchQuery)) {
                tab->addConfigFloatInput("Lock Delta Value", "invisible.lock_delta::value", 240.f, 1.f, FLT_MAX, [&config](float value) {
                    if (config.get<bool>("invisible.lock_delta::sync_tps", true)) {
                        auto val = config.get<float>("invisible.lock_delta::value", 240.f);
                        config.set<float>("invisible.tps::value", val);
                    }
                });
                tab->addHackToggle("Lock Delta Enabled", "invisible.lock_delta");

                tab->addConfigToggle("Real Time (Lock Delta)", "invisible.lock_delta::real_time", true);
                tab->addConfigToggle("Sync TPS w/Lock DT", "invisible.lock_delta::sync_tps", true, [&config](bool enabled) {
                    if (config.get<bool>("invisible.lock_delta::sync_tps", true)) {
                        auto value = config.get<float>("invisible.tps::value", 240.f);
                        config.set<float>("invisible.lock_delta::value", value);
                    }
                });
                matchCount++;
            }
            #endif

            if (containsIgnoreCase("speedhack", searchQuery) || containsIgnoreCase("speed hack", searchQuery) || containsIgnoreCase("speed", searchQuery)) {
                tab->addConfigFloatInput("Speedhack Value", "invisible.speedhack::value", 1.f, 0.01f, 50.f, [&gui](float) {
                    gui.rescanActiveCheats();
                });
                tab->addHackToggle("Speedhack Enabled", "invisible.speedhack", false, [&gui](bool) {
                    gui.rescanActiveCheats();
                });
                tab->addHackToggle("Speedhack Audio", "invisible.speedhack_audio");
                matchCount++;
            }

            if (containsIgnoreCase("pitch shifter", searchQuery) || containsIgnoreCase("pitch", searchQuery) || containsIgnoreCase("shifter", searchQuery)) {
                tab->addConfigIntInput("Pitch Shifter (-24 to 24)", "invisible.pitch_shifter::value", 1.f, -12, 12, [&config](int value) {
                    bool enabled = config.get<bool>("invisible.pitch_shifter", false);
                    GDH::Utils::setPitchShifter(enabled ? config.get<int>("invisible.pitch_shifter::value", 0) : 0);
                });
                tab->addHackToggle("Pitch Shifter Enabled", "invisible.pitch_shifter");
                matchCount++;
            }

            for (auto& win : gui.getWindows()) {
                if (win.getName() == "Invisible") continue;

                for (auto& hack : win.getHacks()) {
                    if (containsIgnoreCase(hack.getName(), searchQuery)) {
                        tab->addToggle(hack);
                        matchCount++;
                    }
                }
            }

            if (matchCount == 0) {
                tab->addText("No hacks found matching query", 0.45f);
            }
        }

        content->updateLayout();
        tab->m_scrollLayer->updateLayout();
        tab->m_scrollLayer->moveToTop();
    }
}

$execute {
    auto& gui = GDH::Gui::get();
    auto& window = gui.getWindow("Search");

    window.setCustomWindowCocos([](cocos2d::CCNode* node) {
        auto tab = static_cast<HacksTab*>(node);

        auto content = tab->m_scrollLayer->m_contentLayer;
        content->setLayout(
            ColumnLayout::create()
                ->setAutoScale(false)
                ->setAxisReverse(true)
                ->setAutoGrowAxis(false)
                ->setGap(4.f)
        );

        auto searchRow = CCMenu::create();
        searchRow->setContentSize({340.f, 32.f});
        searchRow->setLayout(
            RowLayout::create()
                ->setAxisAlignment(AxisAlignment::Center)
                ->setCrossAxisAlignment(AxisAlignment::Center)
                ->setGap(6.f)
        );

        auto input = TextInput::create(210.f, "Search hack...", "GoogleSans.fnt"_spr);
        if (auto* bg = input->getChildByType<geode::NineSlice>(0)) {
            bg->setColor({71, 71, 131});
            bg->setOpacity(255);
        }
        searchRow->addChild(input);

        auto btnSprite = ButtonSprite::create("Search", 65, true, "GoogleSans.fnt"_spr, "GDH_button_01.png"_spr, 24.f, 0.5f);
        auto searchBtn = CCMenuItemExt::createSpriteExtra(btnSprite, [tab, input](auto) {
            refreshSearchResults(tab, input->getString());
        });
        searchRow->addChild(searchBtn);

        searchRow->updateLayout();
        content->addChild(searchRow);

        refreshSearchResults(tab, "");
    });
}