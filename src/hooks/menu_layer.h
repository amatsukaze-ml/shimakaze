#pragma once

#include "../pch.h"

namespace shimakaze {
    namespace menu {
        inline CCLabelBMFont* shimakaze_label;
        inline CCLabelBMFont* shimakaze_mod_count_label;

        void update_mod_count(std::size_t count);
    }
}

inline bool (__thiscall *MenuLayer_init)(CCLayer* self);
bool __fastcall MenuLayer_init_H(CCLayer* self, void *);