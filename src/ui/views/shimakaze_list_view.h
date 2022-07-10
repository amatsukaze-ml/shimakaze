#pragma once

#include "../../pch.h"

namespace shimakaze {
    namespace ui {
        class ShimakazeListView : public gd::CustomListView {
        public:
            static ShimakazeListView* create(cocos2d::CCArray* entries, gd::BoomListType type, float width, float height);
            static ShimakazeListView* create(cocos2d::CCArray* entries, gd::BoomListType type, CCSize size);
        protected:
            void setupList() override;
            gd::TableViewCell* getListCell(const char* key) override;
            void loadCell(gd::TableViewCell* cell, unsigned int index) override;
        };
    }
}