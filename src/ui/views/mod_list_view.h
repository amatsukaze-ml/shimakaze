#pragma once

#include "../../pch.h"

#include "shimakaze_list_view.h"

#include "../objects/mod_object.h"

namespace shimakaze::ui {
    class ModListView : public ShimakazeListView {
    public:
        static ModListView* create(cocos2d::CCArray* entries, gd::BoomListType type, float width, float height);
        static ModListView* create(cocos2d::CCArray* entries, gd::BoomListType type, CCSize size);
    protected:
        void setupList() override;
        gd::TableViewCell* getListCell(const char* key) override;
        void loadCell(gd::TableViewCell* cell, unsigned int index) override;
    };
}