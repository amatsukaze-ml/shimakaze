#pragma once

#include "../pch.h"

inline CCLabelBMFont* shimakaze_label;
inline CCLabelBMFont* mod_count_label;

namespace shimakaze {
    namespace menu {
        void update_mod_count(std::size_t count);
    }
}

SHIMAKAZE_HOOK(MenuLayer_init, CCLayer*, bool);