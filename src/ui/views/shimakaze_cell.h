#pragma once

#include "../../pch.h"

namespace shimakaze {
    namespace ui {
        class ShimakazeCell : public gd::TableViewCell, public gd::FLAlertLayerProtocol {
        public:
            static ShimakazeCell* create(const char* key, CCSize size);
        public:
            void setIndex(unsigned int index);
            void setupObjectData(CCObject* object);
            void updateBGColor();
        protected:
            unsigned int index;

            ShimakazeCell(const char* key, CCSize size);
            void onClick(CCObject* sender);
            void FLAlert_Clicked(gd::FLAlertLayer* alert, bool btn2) override;
        };
    }
}