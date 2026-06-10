#pragma once

namespace TriggerIDSearch {
    class FindMenu : public cocos2d::CCMenu {
    protected:
        cocos2d::CCLabelBMFont* m_label = nullptr;

        std::vector<std::vector<GameObject*>> m_objs;
        size_t m_index = 0;
        float m_zoom = 1.0f;

        bool m_enabled = false;

        bool init();

        void showIndex(size_t pIndex);

        void onHide(cocos2d::CCObject*);
        void onNext(cocos2d::CCObject*);
        void onPrevious(cocos2d::CCObject*);

        static constexpr float LABEL_SIZE = 20.0f;
        static constexpr float BUTTON_SIZE = 15.0f;
        static constexpr float ARROW_SIZE = 20.0f;
        static constexpr float GAP = 10.0f;
        static constexpr float ARROW_GAP = 50.0f;

    public:
        void show(cocos2d::CCArray* pObjs);
        void hide();
        
        bool isEnabled() const;

        static FindMenu* create();
    };

    void showFindMenu(cocos2d::CCArray* pObjs);
}