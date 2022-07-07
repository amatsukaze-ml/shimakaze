#include "loading_layer.h"

#include "../core/core.h"

#include <thread>
#include <chrono>

namespace shimakaze
{
    namespace loading
    {
        void update_progress(float max_number, float current_number)
        {
            progress_bar->setTextureRect(CCRect(0, 0, (current_number / max_number) * (progress_bar_base_size.width - 5.0f), progress_bar->getContentSize().height));
        }
        
        void update_progress_text(const char *text)
        {
            progress_label->setString(text);
        }

        void replace_to_menu_layer()
        {
            // manifest a menu layer
            CCScene* menu_layer = gd::MenuLayer::scene(false);

            // replace current scene with it
            CCDirector::sharedDirector()->replaceScene(menu_layer);
        }
    }
}

SHIMAKAZE_CALL_ARGS(LoadingLayer_init, CCLayer *, bool, bool fromReload)
{
    if (!LoadingLayer_init(self, fromReload))
    {
        return false;
    }

    // size of screen
    CCSize size = CCDirector::sharedDirector()->getWinSize();

    // add progress label
    progress_label = CCLabelBMFont::create("Waiting for Geometry Dash to load...", "bigFont.fnt");

    // center it in the screen and put it above gd's loading bar
    progress_label->setAnchorPoint({0.5f, 0.5f});
    progress_label->setScale(0.5f);
    progress_label->setPosition(ccp(size.width / 2, (size.height / 2) - 40.0f));

    // create our own progress bar
    CCSprite *progress_groove = CCSprite::create("slidergroove.png");
    progress_bar = CCSprite::create("sliderBar.png");

    // move it to below gd's actual progress bar
    progress_groove->setAnchorPoint({0.5f, 0.5f});
    progress_groove->setPosition(ccp(size.width / 2.0f, (size.height / 2) - 75.0f));
    progress_bar->setAnchorPoint({0.0f, 0.5f});
    progress_bar->setPosition(ccp(progress_groove->getTextureRect().getMaxX() - 27.5f, (size.height / 2) - 75.0f));

    // assign base size of progress bar
    progress_bar->setTextureRect(CCRect(0, 0, progress_groove->getContentSize().width, progress_bar->getContentSize().height));
    progress_bar_base_size = progress_bar->getContentSize();

    // set to 0%
    shimakaze::loading::update_progress(1.0f, 0.0f);

    self->addChild(progress_label);
    self->addChild(progress_bar);
    self->addChild(progress_groove);

    return true;
}

SHIMAKAZE_CALL(LoadingLayer_loadingFinished, CCLayer *, void)
{
    // show_debug
    toml::table *shimakaze_main = shimakaze::core::g_config["main"].as_table();
    bool show_debug = shimakaze_main->get("show_debug")->value_or(true);

    // gd finished loading
    shimakaze::loading::update_progress_text("Setup: Starting Shimakaze...");

    // start shimakaze core
    shimakaze::core::start();
}