#include "menu_layer.h"

#include "../core/core.h"

#include <format>

namespace shimakaze {
    namespace menu {
        void update_mod_count(std::size_t count) {
            std::string modCount = std::format("{} mods loaded", count);
            mod_count_label->setString(modCount.c_str());
        }
    }
}

SHIMAKAZE_CALL(MenuLayer_init, CCLayer *, bool)
{
    if (!MenuLayer_init(self)) {
        return false;
    }

    // size of screen
    CCSize size = CCDirector::sharedDirector()->getWinSize();

    // add shimakaze label
    std::string shimakazeVersion = std::format("Shimakaze {}", SHIMAKAZE_VERSION);
    std::string modCount = std::format("{} mods loaded", shimakaze::core::g_mod_count);

    shimakaze_label = CCLabelBMFont::create(shimakazeVersion.c_str(), "bigFont.fnt");
    mod_count_label = CCLabelBMFont::create(modCount.c_str(), "bigFont.fnt");

    shimakaze_label->setAnchorPoint({ 1.0f, 0.0f });
    shimakaze_label->setScale(0.25f);
    shimakaze_label->setPosition(ccp(size.width, 7.5f));

    mod_count_label->setAnchorPoint({ 1.0f, 0.0f });
    mod_count_label->setScale(0.25f);
    mod_count_label->setPosition(ccp(size.width, 0.0f));

    self->addChild(shimakaze_label);
    self->addChild(mod_count_label);

    return true;
}