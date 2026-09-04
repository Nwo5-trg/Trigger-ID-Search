#include <Geode/modify/EditorUI.hpp>
#include <nwo5.silly-api/include/include.hpp>
#include <nwo5.ui-scaling/include/include.hpp>
#include "find-menu.hpp"
#include "settings.hpp"

using namespace geode::prelude;
using namespace nwo5::uiscaling::prelude;
using namespace nwo5::prelude;

namespace TriggerIDSearch {
    bool FindMenu::init() {
        if (!CCMenu::init()) {
            return false;
        }

        Setup(this)
            .layout(ui::column()
                .alignment(AxisAlignment::Start)
                .gap(GAP)
                .autoScale(false)
                .grow(false)
            )
            .size(ARROW_GAP * 2 + ARROW_SIZE, LABEL_SIZE + BUTTON_SIZE + GAP)
            .ignoreAnchorForPos(false)
            .visible(false);

        ui::button(
            ButtonSprite::create("ok"), this, menu_selector(FindMenu::onHide)
        )
            .id("ok-button"_spr)
            .scaleHeightToFit(BUTTON_SIZE)
            .parent(this);

        m_label = ui::label(" ", Font::Default)
            .id("current-index-label"_spr)
            .scaleHeightToFit(LABEL_SIZE);

        ui::menu(ui::row()
            .alignment(AxisAlignment::Center)
            .gap(ARROW_GAP)
            .autoScale(false)
            .grow(false)
        )
            .id("menu"_spr)
            .children(
                ui::buttonFrame(
                    ui::frame::PINK_ARROW, this, menu_selector(FindMenu::onPrevious)
                )
                    .id("prev-layer-button"_spr)
                    .scaleToFit(ARROW_SIZE),
                m_label,
                ui::buttonFrame(
                    ui::frame::PINK_ARROW, this, menu_selector(FindMenu::onNext)
                )
                    .id("next-layer-button"_spr)
                    .scaleToFit(ARROW_SIZE)
                    .flipX()
            )
            .parent(this);

        return true;
    }
    
    void FindMenu::showIndex(size_t pIndex) {
        if (m_objs.empty()) {
            return;
        }

        m_label->setText(fmt::format("{}/{}", pIndex + 1, m_objs.size()));

        editor::object::moveTo(m_objs[pIndex], true, 1.5f, Settings::zoomLimit, editor::zoom());
        editor::selection::set(m_objs[pIndex], true, true, true, true);
        editor::update();
    }

    void FindMenu::onHide(CCObject*) {
        this->hide();
    }
    void FindMenu::onNext(CCObject*) {
        this->showIndex(m_index = (m_index + 1 == m_objs.size() ? 0 : m_index + 1));
    }
    void FindMenu::onPrevious(CCObject*) {
        this->showIndex(m_index = (!m_index ? m_objs.size() - 1 : m_index - 1));
    }

    void FindMenu::show(CCArray* pObjs) {
        if (!pObjs->count()) {
            return;
        }

        this->setVisible(true);

        m_enabled = true;
        m_objs.clear();
        editor::object::cluster(m_objs, pObjs, Settings::findMenuClustering);

        m_label->setText(fmt::format("{0}/{0}", m_objs.size()));
        m_label->getParent()->updateLayout();
        
        this->showIndex(m_index = 0);

        if (m_objs.size() <= 1) {
            hide();
        }
    }
    void FindMenu::hide() {
        this->setVisible(false);

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

        m_fields->findMenu = Setup(TriggerIDSearch::FindMenu::create())
            .id("find-menu"_spr)
            .parent(this);

        this->updateFindMenuPosition(1.0f);

        this->addEventListener(uiscaling::EditorUI::Changed(), [this] (float pScale) {
            this->updateFindMenuPosition(pScale);
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
    
    void updateFindMenuPosition(float pScale) {
        auto menu = m_fields->findMenu;

        if (!menu) {
            return;
        }

        Setup(menu)
            .scale(pScale)
            .pos(
                CCDirector::get()->getWinSize().width / 2, 
                (m_toolbarHeight + ui::sh(menu) / 2) + (5.0f * pScale)
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