#include <Geode/modify/EditorUI.hpp>
#include <nwo5.silly-api/include/include.hpp>
#include <alphalaneous.tinker/include/UIScaling.hpp>
#include "find-menu.hpp"
#include "settings.hpp"

using namespace geode::prelude;
using namespace nwo5::prelude;

namespace TriggerIDSearch {
    bool FindMenu::init() {
        if (!CCMenu::init()) {
            return false;
        }

        Setup(this)
            .size(ARROW_GAP * 2 + ARROW_SIZE, LABEL_SIZE + BUTTON_SIZE + GAP)
            .ignoreAnchorForPos(false)
            .visible(false);

        m_label = ui::node(Setup(ui::label(" "))
            .id("current-index-label"_spr)
            .pos(getContentWidth() / 2, BUTTON_SIZE + GAP + LABEL_SIZE / 2)
            .scaleHeightToFit(LABEL_SIZE)
            .parent(this)
        );

        auto next = ui::node(Setup(ui::buttonFrame(
            "GJ_arrow_03_001.png", this, menu_selector(FindMenu::onNext)
        ))
            .id("next-layer-button"_spr)
            .scaleToFit(ARROW_SIZE)
            .flipX()
            .right(m_label, ARROW_GAP)
            .parent(this)
        );
        auto prev = ui::node(Setup(ui::buttonFrame(
            "GJ_arrow_03_001.png", this, menu_selector(FindMenu::onPrevious)
        ))
            .id("prev-layer-button"_spr)
            .scaleToFit(ARROW_SIZE)
            .left(m_label, ARROW_GAP)
            .parent(this)
        );

        auto okButton = ui::node(Setup(ui::button(
            ButtonSprite::create("ok"), this, menu_selector(FindMenu::onHide)
        ))
            .id("ok-button"_spr)
            .pos(getContentWidth() / 2, BUTTON_SIZE / 2)
            .scaleHeightToFit(BUTTON_SIZE)
            .parent(this)
        );

        return true;
    }
    
    void FindMenu::showIndex(size_t pIndex) {
        if (m_objs.empty()) {
            return;
        }

        m_label->setString(fmt::format("{}/{}", pIndex + 1, m_objs.size()).c_str());

        editor::object::moveTo(m_objs[pIndex], true, 1.5f, Settings::zoomLimit, m_zoom);
        editor::selection::set(m_objs[pIndex], true, true, true, true);
        editor::update();
    }

    void FindMenu::onHide(CCObject*) {
        hide();
    }
    void FindMenu::onNext(CCObject*) {
        showIndex(m_index = (m_index + 1 == m_objs.size() ? 0 : m_index + 1));
    }
    void FindMenu::onPrevious(CCObject*) {
        showIndex(m_index = (!m_index ? m_objs.size() - 1 : m_index - 1));
    }

    void FindMenu::show(CCArray* pObjs) {
        if (!pObjs->count()) {
            return;
        }

        setVisible(true);

        m_enabled = true;
        m_objs.clear();
        m_zoom = editor::zoom();
        editor::object::cluster(m_objs, pObjs, Settings::findMenuClustering);

        showIndex(m_index = 0);

        if (m_objs.size() <= 1) {
            hide();
        }
    }
    void FindMenu::hide() {
        setVisible(false);

        m_objs.clear();
        m_enabled = false;
    }

    bool FindMenu::isEnabled() const {
        return m_enabled;
    }

    FindMenu* FindMenu::create() {
        auto ret = new FindMenu();

        if (!ret->init()) {
            delete ret;

            return nullptr;
        }

        ret->autorelease();

        return ret;
    }
}

class $modify(FindMenuEditorUI, EditorUI) {
    struct Fields {
        TriggerIDSearch::FindMenu* findMenu = nullptr;
        ListenerHandle listenerHandle;
    };

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) {
            return false;
        }

        m_fields->findMenu = ui::node(Setup(TriggerIDSearch::FindMenu::create())
            .id("find-menu"_spr)
            .parent(this)
        );

        updateFindMenuPosition();

        m_fields->listenerHandle = tinker::api::ui_scaling::UIScaleUpdated().listen([this] (float, bool, bool) {
            this->updateFindMenuPosition();
        });
        
        return true;
    }

    // called when changing modes
    void resetUI() {
        EditorUI::resetUI();

        if (auto menu = m_fields->findMenu; menu && menu->isEnabled()) {
            menu->setVisible(m_selectedMode == m_deleteModeBtn->getTag());
        }
    }

    void onDeselectAll(CCObject* sender) {
        EditorUI::onDeselectAll(sender);

        if (auto menu = m_fields->findMenu) {
            menu->hide();
        }
    }
    
    void updateFindMenuPosition() {
        auto menu = m_fields->findMenu;

        if (!menu) {
            return;
        }

        Setup(menu)
            .scale(editor::uiScale())
            .pos(
                CCDirector::get()->getWinSize().width / 2, 
                (m_toolbarHeight + menu->getScaledContentHeight() / 2) + (5.0f * editor::uiScale())
            );
    }
};

namespace TriggerIDSearch {
    void showFindMenu(CCArray* pObjs) {
        if (auto ui = editor::ui<FindMenuEditorUI>()) {
            if (auto menu = ui->m_fields->findMenu) {
                menu->show(pObjs);
            }
        }
    }
}