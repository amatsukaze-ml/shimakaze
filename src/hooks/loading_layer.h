#pragma once

#include "../pch.h"

inline CCLabelBMFont *progress_label;
inline CCSprite *progress_bar;
inline CCSize progress_bar_base_size;

namespace shimakaze
{
    namespace loading
    {
        void update_progress(float max_number, float current_number);
        void update_progress_text(const char* text);
        void replace_to_menu_layer();
    }
}

SHIMAKAZE_HOOK_ARGS(LoadingLayer_init, CCLayer *, bool, bool fromReload);
SHIMAKAZE_HOOK(LoadingLayer_loadAssets, CCLayer *, void);
SHIMAKAZE_HOOK(LoadingLayer_loadingFinished, CCLayer *, void);