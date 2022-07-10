#pragma once

#include "../pch.h"

namespace shimakaze::ui {
    class ModListLayer : public CCLayer {
    public:
        static ModListLayer* create();
    public:
        bool init();

        void onModButtonPress(CCObject* sender);
        void onBackButtonPress(CCObject* sender);
    };
}