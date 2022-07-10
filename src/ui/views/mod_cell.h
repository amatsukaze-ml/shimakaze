#pragma once

#include "../../pch.h"

#include "shimakaze_cell.h"

#include "../objects/mod_object.h"

namespace shimakaze {
    namespace ui {
        class ModCell : public ShimakazeCell {
        public:
            void setupObjectData(ModObject* object);
        protected:
            void onClick(CCObject* sender);
            void FLAlert_Clicked(gd::FLAlertLayer* alert, bool btn2) override;
        };
    }
}