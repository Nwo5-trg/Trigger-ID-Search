#include <Geode/modify/FindObjectPopup.hpp>
#include <nwo5.silly-api/include/include.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include "settings.hpp"
#include <ranges>

using namespace nwo5::prelude;
using namespace geode::prelude;

enum class SearchMode {
    All,
    Primary,
    Secondary,
    AllTargets,
    PrimaryInput,
    SecondaryInput,
    AllInputs
};

enum class FindType {
    Group = 0,
    Item = 1,
    Collision = 2
};
// lol it works so :3c
namespace nwo5::utils {
    template<>
    constexpr editor::trigger::InputType enum_cast(FindType pEnum) {
        switch (pEnum) {
            case FindType::Group: return editor::trigger::InputType::Group;
            case FindType::Item: return editor::trigger::InputType::Item;
            case FindType::Collision: return editor::trigger::InputType::Collision;
        }
    }
}

static auto getSearchMode() {
    static std::unordered_map<std::string, SearchMode> map{
        {"All", SearchMode::All}, {"Primary", SearchMode::Primary},
        {"Secondary", SearchMode::Secondary},{"All Targets", SearchMode::AllTargets},
        {"Primary Input", SearchMode::PrimaryInput}, {"Secondary Input", SearchMode::SecondaryInput},
        {"All Inputs", SearchMode::AllInputs}
    };

    return map[Settings::searchMode];
}

class $modify(FindObjectPopupHook, FindObjectPopup) {
    struct Fields {
        bool findGroups = false;
    };

    static bool filterObject(GameObject* pObj, SearchMode pSearchMode, FindType pType, bool pFindGroup, int pID) {
        namespace trigger = editor::trigger;

        if (!pID) {
            return false;
        }

        if (pFindGroup) {
            switch (pType) {
                case FindType::Group: {
                    return editor::object::hasGroup(pObj, pID);
                }
                case FindType::Item: {
                    return pObj->m_objectID == trigger::COUNTER_LABEL && trigger::primaryTarget(pObj) == pID;
                }
                case FindType::Collision: {
                    return pObj->m_objectID == trigger::COLLISION_BLOCK && trigger::primaryTarget(pObj) == pID;
                }
            }
        }

        const auto type = enum_cast<trigger::InputType>(pType);

        if (!trigger::is(pObj) || pObj->m_objectID == trigger::COUNTER_LABEL || pObj->m_objectID == trigger::COLLISION_BLOCK) {
            return false;
        }

        const std::array<int, 4> ids{
            trigger::primaryTargetType(pObj) == type ? trigger::primaryTarget(pObj) : 0,
            trigger::secondaryTargetType(pObj) == type ? trigger::secondaryTarget(pObj) : 0,
            trigger::primaryInputType(pObj) == type ? trigger::primaryInput(pObj) : 0,
            trigger::secondaryInputType(pObj) == type ? trigger::secondaryInput(pObj) : 0,
        };

        if (Settings::logs) {
            log::debug("{} | {}, {}, {}, {}", pObj->m_objectID, ids[0], ids[1], ids[2], ids[3]);
        }

        switch (pSearchMode) {
            case SearchMode::All: {
                return ids[0] == pID || ids[1] == pID || ids[2] == pID || ids[3] == pID;
            }
            case SearchMode::Primary: {
                return ids[0] == pID;
            }
            case SearchMode::Secondary: {
                return ids[1] == pID;
            }
            case SearchMode::AllTargets: {
                return ids[0] == pID || ids[1] == pID;
            }
            case SearchMode::PrimaryInput: {
                return ids[2] == pID;
            }
            case SearchMode::SecondaryInput: {
                return ids[3] == pID;
            }
            case SearchMode::AllInputs: {
                return ids[2] == pID || ids[3] == pID;
            }
        }
    }

    bool init() {
        if (!FindObjectPopup::init()) {
            return false;
        }

        auto bg = m_mainLayer->getChildByType<CCScale9Sprite>(0);

        if (!bg) {
            return true;
        }

        auto menu = ui::node(Setup(ui::menu(ui::verticalDistrbLayout(2.5f)))
            .id("button-menu"_spr)
            .pos(
                bg->getPosition() + (ccp(bg->getScaledContentWidth(), -bg->getScaledContentHeight()) / 2) 
                + ccp(-5.0f, 5.0f)
            )
            .anchor(1.0f, 0.0f)
            .parent(m_mainLayer)
        );
        // gah
        menu->setTouchPriority(-510);

        for (int i = 0; i < 3; i++) {
            auto button = ui::node(Setup(ui::buttonSprite(
                fmt::format("button{}.png"_spr, i), this, menu_selector(FindObjectPopupHook::onFindTriggers)
            ))
                .id("button-{}"_spr, i)
                .scaleToFit(30.0f)
                .tag(i)
                .parent(menu)
            );
        }

        auto findGroupsToggle = ui::node(Setup(ui::togglerBase(
            this, menu_selector(FindObjectPopupHook::onToggleFindGroups)
        ))
            .id("find-groups-toggle"_spr)
            .pos(
                bg->getPosition() - (bg->getScaledContentSize() / 2) 
                - m_buttonMenu->getPosition() + ccp(20.0f, 20.0f)
            )
            .scaleToFit(30.0f)
            .parent(m_buttonMenu)
        );

        auto settingsButton = ui::node(Setup(ui::buttonFrame(
                "GJ_optionsBtn_001.png", this, menu_selector(FindObjectPopupHook::onIDSearchSettings)
            ))
                .id("id-seach-settings-button"_spr)
                // remind me to make a better system for positioning in popups with setup cuz this is js wacky
                .pos(
                    bg->getPosition() + (bg->getScaledContentSize() / 2) 
                    - m_buttonMenu->getPosition() - ccp(20.0f, 20.0f)
                )
                .scaleToFit(30.0f)
                .parent(m_buttonMenu)
            );

        m_inputNode->setAllowedChars("1234567890,");
        m_inputNode->setMaxLabelLength(9999);

        this->getChildByType<CCLabelBMFont>(0)->setString("Find ID");
        
        return true;
    }

    // i love std::ranges shenanigans gotta b one of my favorite genders (idek if its more readable than js manually writing shit out but its fun)
    void onFindTriggers(CCObject* pSender) {
        const auto type = enum_cast<FindType>(pSender->getTag());

        const auto split = string::splitView(std::string_view(m_inputNode->getString().c_str()), ",");
        const auto groups = std::ranges::to<std::vector>(
            std::views::transform(split, [] (const auto& pStr) {
                return utils::numFromString<int>(pStr).unwrapOr(0);
            })
        );

        if (Settings::logs) {
            log::debug("searching ids");
            for (auto group : groups) {
                log::debug("{}", group);
            }
            log::debug("string\n{}", std::string_view(m_inputNode->getString().c_str()));
        }

        const auto findGroups = m_fields->findGroups;
        const auto searchMode = getSearchMode();

        auto foundObjs = CCArray::create();

        const bool selectionFilter = Settings::useSelectionAsFilter && !editor::selection::empty();
        auto objs = selectionFilter ? editor::selection::get() : editor::layer()->m_objects;

        for (auto obj : CCArrayExt<GameObject*>(objs)) {
            if (Settings::listAnd) {
                if (!std::ranges::all_of(groups, [&] (int pID) {
                    return filterObject(obj, searchMode, type, findGroups, pID); 
                })) {
                    continue;
                }
            }
            else {
                if (!std::ranges::any_of(groups, [&] (int pID) {
                    return filterObject(obj, searchMode, type, findGroups, pID); 
                })) {
                    continue;
                }
            }

            foundObjs->addObject(obj);
        }
        
        if (Settings::autoDelete) {
            editor::object::remove(foundObjs, true);
        }
        else {
            if (Settings::includeCurrentSelection && !selectionFilter) {
                editor::selection::add(foundObjs);
            }
            else {
                editor::selection::set(foundObjs);
            }

            if (Settings::moveCameraToSelection) {
                const auto bounds = editor::object::bounds(foundObjs);

                editor::move(bounds.origin + bounds.size / 2);
                editor::setZoom(
                    std::clamp(
                        (CCDirector::get()->getWinSize().width / bounds.size.width) / 1.5f, 
                        Settings::zoomLimit.get(), editor::zoom()
                    )
                );
            }
        }

        editor::update();

        if (Settings::closeOnSelect) {
            static_cast<CCMenuItemSpriteExtra*>(this->getChildByIDRecursive("close-button"))->activate();
        }
    }

    void onIDSearchSettings(CCObject*) {
        openSettingsPopup(Mod::get(), true);
    }

    void onToggleFindGroups(CCObject*) {
        m_fields->findGroups = !m_fields->findGroups;
    }
};

$on_mod(Loaded) {
	SettingsManager::get()->load();
}