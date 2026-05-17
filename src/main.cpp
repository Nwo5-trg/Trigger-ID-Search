#include <Geode/Geode.hpp>
#include <Geode/modify/SetIDPopup.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <vector>

using namespace geode::prelude;

auto mod = Mod::get();

constexpr char const *SearchNavigatorID = "nwo5.trigger_id_search/navigator";

std::unordered_set<int> collisionObjects{
    1815, 1816, 3609, 3640};

std::unordered_set<int> getIntSet(std::string input)
{
    std::unordered_set<int> intSet;
    auto start = 0;
    while (true)
    {
        auto comma = input.find(',', start);
        intSet.insert(std::strtol(input.substr(start, comma - start).c_str(), nullptr, 10));
        if (comma == std::string::npos)
            break;
        start = comma + 1;
    }
    return intSet;
}

class SearchNavigator : public CCNode
{
public:
    static SearchNavigator *create()
    {
        auto ret = new SearchNavigator();
        if (ret && ret->init())
        {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool init() override
    {
        if (!CCNode::init())
            return false;

        this->setID(SearchNavigatorID);

        m_menu = CCMenu::create();
        m_menu->setID("navigation-menu");
        this->addChild(m_menu);

        auto prevSprite = ButtonSprite::create("Prev", 26, 0, 0.28f, false);
        prevSprite->setScale(0.8f);

        m_prevButton = CCMenuItemSpriteExtra::create(
            prevSprite,
            this,
            menu_selector(SearchNavigator::onPrev));

        auto nextSprite = ButtonSprite::create("Next", 26, 0, 0.28f, false);
        nextSprite->setScale(0.8f);

        m_nextButton = CCMenuItemSpriteExtra::create(
            nextSprite,
            this,
            menu_selector(SearchNavigator::onNext));

        m_prevButton->setPosition(ccp(-18.f, 0.f));
        m_nextButton->setPosition(ccp(18.f, 0.f));

        m_menu->addChild(m_prevButton);
        m_menu->addChild(m_nextButton);

        m_counterLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_counterLabel->setScale(0.35f);
        m_counterLabel->setID("result-counter");
        this->addChild(m_counterLabel);

        this->updatePosition();
        this->setVisible(false);
        return true;
    }

    void setResults(CCArray *objects)
    {
        m_results.clear();
        m_index = 0;

        for (auto obj : CCArrayExt<GameObject *>(objects))
            m_results.push_back(obj->m_uniqueID);

        this->centerCurrent();
    }

    void clearResults()
    {
        m_results.clear();
        m_index = 0;
        this->setVisible(false);
    }

    void onPrev(CCObject *)
    {
        if (m_results.size() <= 1)
            return;

        if (m_index == 0)
            m_index = m_results.size() - 1;
        else
            --m_index;

        this->centerCurrent();
    }

    void onNext(CCObject *)
    {
        if (m_results.size() <= 1)
            return;

        m_index = (m_index + 1) % m_results.size();
        this->centerCurrent();
    }

    bool hasValidResults()
    {
        for (auto id : m_results)
        {
            if (this->findObjectByID(id))
                return true;
        }

        return false;
    }

private:
    std::vector<int> m_results;
    size_t m_index = 0;
    CCMenu *m_menu = nullptr;
    CCMenuItemSpriteExtra *m_prevButton = nullptr;
    CCMenuItemSpriteExtra *m_nextButton = nullptr;
    CCLabelBMFont *m_counterLabel = nullptr;

    void updatePosition()
    {
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // centered above bottom editor UI
        float y = 92.f;

        m_menu->setPosition(ccp(winSize.width / 2.f, y));

        if (m_counterLabel)
            m_counterLabel->setPosition(ccp(winSize.width / 2.f, y + 18.f));
    }

    void updateControls()
    {
        this->updatePosition();
        this->setVisible(!m_results.empty());

        if (m_counterLabel && !m_results.empty())
        {
            auto text = std::to_string(m_index + 1) + "/" + std::to_string(m_results.size());
            m_counterLabel->setCString(text.c_str());
        }
    }

    GameObject *findObjectByID(int uniqueID)
    {
        auto editor = LevelEditorLayer::get();
        if (!editor)
            return nullptr;

        for (auto obj : CCArrayExt<GameObject *>(editor->m_objects))
            if (obj->m_uniqueID == uniqueID)
                return obj;

        return nullptr;
    }

    GameObject *currentObject()
    {
        while (!m_results.empty())
        {
            if (m_index >= m_results.size())
                m_index = 0;

            if (auto obj = this->findObjectByID(m_results[m_index]))
                return obj;

            m_results.erase(m_results.begin() + m_index);
        }
        return nullptr;
    }

    void centerCurrent()
    {
        auto editUI = EditorUI::get();
        auto obj = this->currentObject();

        if (editUI && obj)
            editUI->centerCameraOnObject(obj);

        this->updateControls();
    }
};
SearchNavigator *getSearchNavigator(EditorUI *editUI)
{
    if (!editUI)
        return nullptr;

    if (auto existing = editUI->getChildByID(SearchNavigatorID))
        return static_cast<SearchNavigator *>(existing);

    auto navigator = SearchNavigator::create();
    if (navigator)
        editUI->addChild(navigator, 1000);

    return navigator;
}

void clearSearchNavigator(EditorUI *editUI)
{
    if (!editUI)
        return;

    if (auto navigator = static_cast<SearchNavigator *>(editUI->getChildByID(SearchNavigatorID)))
        navigator->removeFromParentAndCleanup(true);
}

class $modify(EditorUI)
{
    void deselectAll()
    {
        EditorUI::deselectAll();
        clearSearchNavigator(this);
    }

    void onDeleteSelected(CCObject *sender)
    {
        EditorUI::onDeleteSelected(sender);
        clearSearchNavigator(this);
    }

    void onDelete(CCObject *sender)
    {
        EditorUI::onDelete(sender);

        auto navigator = getSearchNavigator(this);
        if (!navigator)
            return;

        if (!navigator->hasValidResults())
            clearSearchNavigator(this);
    }
};

class $modify(IDPopup, SetIDPopup)
{
    struct Fields
    {
        bool findGroups = false;
    };

    bool init(int current, int begin, int end, gd::string title, gd::string button, bool p5, int p6, float p7, bool p8, bool p9)
    {
        if (!SetIDPopup::init(current, begin, end, title, button, p5, p6, p7, p8, p9))
            return false;
        if (typeinfo_cast<FindObjectPopup *>(this))
        {
            auto menu = this->getChildByType<CCLayer>(0)->getChildByType<CCMenu>(0);
            for (int i = 0; i < 3; i++)
            {
                auto button = CCMenuItemSpriteExtra::create(CCSprite::create(("nwo5.trigger_id_search/button" + std::to_string(i) + ".png").c_str()), this, menu_selector(IDPopup::findTriggers));
                button->setPosition(ccp(86, 0 + (i * 40)));
                button->setScale(0.75);
                button->m_baseScale = 0.75;
                button->setTag(i);
                menu->addChild(button);
            }
            auto input = this->getChildByType<CCLayer>(0)->getChildByType<CCTextInputNode>(0);
            input->setAllowedChars("1234567890,");
            input->setMaxLabelLength(999);
            auto toggler = CCMenuItemToggler::create(CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
                                                     CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"), this, menu_selector(IDPopup::onToggleMode));
            toggler->setPosition(ccp(-86, 0));
            menu->addChild(toggler);
        }
        return true;
    }

    void findTriggers(CCObject *sender)
    {
        int type = static_cast<CCNode *>(sender)->getTag();
        if (m_fields->findGroups)
            type = -1;
        auto editUI = EditorUI::get();
        auto editor = LevelEditorLayer::get();
        auto objs = type == -1 ? editor->m_objects : editor->m_drawGridLayer->m_effectGameObjects;
        auto groups = getIntSet(std::string(m_inputNode->getString()));
        auto foundObjs = CCArray::create();
        for (auto obj : CCArrayExt<GameObject *>(objs))
            if (hasID(obj, groups, type))
                foundObjs->addObject(obj);

        // for (int id : getIntVector(input)) { // i could optimise this to o(n) but fuck off
        //     if (id < 1 || id > 9999) continue;
        //     for (auto obj : CCArrayExt<GameObject*>(objs)) {
        //         if (auto trigger = typeinfo_cast<EffectGameObject*>(obj)) {
        //             if (hasID(trigger, id, type)) triggers.addObject(trigger);
        //         }
        //     }
        // }
        if (!mod->getSettingValue<bool>("include-current-selection") || mod->getSettingValue<bool>("auto-delete"))
            editUI->deselectAll();
        editUI->selectObjects(foundObjs, true);
        if (auto navigator = getSearchNavigator(editUI))
        {
            if (mod->getSettingValue<bool>("auto-delete"))
                navigator->clearResults();
            else
                navigator->setResults(foundObjs);
        }
        if (mod->getSettingValue<bool>("auto-delete"))
            editUI->m_trashBtn->activate();
        if (mod->getSettingValue<bool>("update-editor") && !mod->getSettingValue<bool>("auto-delete"))
        {
            editUI->updateButtons();
            editUI->updateDeleteButtons();
            editUI->updateEditMenu();
            editUI->updateGridNodeSize();
            editUI->updateObjectInfoLabel();
        }
        if (mod->getSettingValue<bool>("close-on-select"))
            this->getChildByType<CCLayer>(0)->getChildByType<CCMenu>(0)->getChildByType<CCMenuItemSpriteExtra>(3)->activate();
    }

    void onToggleMode(CCObject *sender)
    {
        m_fields->findGroups = !m_fields->findGroups;
    }

    bool hasID(GameObject *gameObj, std::unordered_set<int> id, int filterType)
    {
        if (filterType == 0)
        {
            auto obj = static_cast<EffectGameObject *>(gameObj);
            if (id.contains(obj->m_targetGroupID))
                return true;
            if (id.contains(obj->m_centerGroupID))
                return true;
            if (id.contains(obj->m_targetModCenterID))
                return true;
            if (id.contains(obj->m_rotationTargetID))
                return true;
        }
        if (filterType == 1 && !collisionObjects.contains(gameObj->m_objectID))
        {
            auto obj = static_cast<EffectGameObject *>(gameObj);
            if (id.contains(obj->m_itemID))
                return true;
            if (id.contains(obj->m_itemID2))
                return true;
        }
        if (filterType == 2 && collisionObjects.contains(gameObj->m_objectID))
        {
            auto obj = static_cast<EffectGameObject *>(gameObj);
            if (id.contains(obj->m_itemID))
                return true;
            if (id.contains(obj->m_itemID2))
                return true;
        }
        if (filterType == -1)
        {
            auto groups = gameObj->m_groups;
            if (!groups)
                return false;
            for (auto group : *groups)
                if (id.contains(group))
                    return true;
        }
        return false;
    }
};
