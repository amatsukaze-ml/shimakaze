#pragma once

#include "../../pch.h"

#include "../../core/mods/mod_entry.h"

namespace shimakaze {
    namespace ui {
        class ModObject : public CCObject {
        public:
            static ModObject* create(core::ModEntry* entry);
            ModObject(core::ModEntry* entry);

            std::string getName();
            CCSprite* getIcon();
            toml::array getAuthors();
        protected:
            core::ModEntry* entry;
        };
    }
}