#include "menu_layer.h"

#include <format>

SHIMAKAZE_CALL(MenuLayer_init, CCLayer *, bool)
{
    if (!MenuLayer_init(self)) {
        return false;
    }

    // size of screen
    CCSize size = CCDirector::sharedDirector()->getWinSize();

    // add shimakaze label
    std::string shimakazeVersion = std::format("Shimakaze {}", SHIMAKAZE_VERSION);
    std::string modCount = std::format("0 mods loaded");

    auto shimakazeLabel = CCLabelBMFont::create(shimakazeVersion.c_str(), "bigFont.fnt");
    auto modCountLabel = CCLabelBMFont::create(modCount.c_str(), "bigFont.fnt");

    shimakazeLabel->setAnchorPoint({ 1.0f, 0.0f });
    shimakazeLabel->setScale(0.25f);
    shimakazeLabel->setPosition(ccp(size.width, 7.5f));

    modCountLabel->setAnchorPoint({ 1.0f, 0.0f });
    modCountLabel->setScale(0.25f);
    modCountLabel->setPosition(ccp(size.width, 0.0f));

    self->addChild(shimakazeLabel);
    self->addChild(modCountLabel);

    return true;
}