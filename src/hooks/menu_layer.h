#pragma once

#include "../pch.h"

#include <map>

inline CCLabelBMFont* shimakaze_label;
inline CCLabelBMFont* mod_count_label;

namespace shimakaze::menu {
    inline std::map<std::string, std::vector<COPYABLE_PERSISTENT<v8::Function>>> menulayer_hooks;

    void update_mod_count(std::size_t count);
}

SHIMAKAZE_HOOK(MenuLayer_init, CCLayer*, bool);