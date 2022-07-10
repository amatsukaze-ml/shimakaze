#pragma once

#include "../../pch.h"

namespace shimakaze::ui {
    class ModListViewer : public CCLayer {
    private:
        gd::GJListLayer* list_layer;
    public:
        static ModListViewer* create();
    protected:
        bool init();
    };
}