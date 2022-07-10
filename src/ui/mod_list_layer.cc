#include "mod_list_layer.h"

#include "viewers/mod_list_viewer.h"

namespace shimakaze::ui
{
    ModListLayer *ModListLayer::create()
    {
        ModListLayer *instance = new ModListLayer();

        if (instance && instance->init())
        {
            instance->autorelease();
            return instance;
        }
        else
        {
            return nullptr;
        }
    }

    bool ModListLayer::init()
    {
        // size of screen
        CCSize size = CCDirector::sharedDirector()->getWinSize();

        // add background
        CCSprite *background = CCSprite::create("GJ_gradientBG.png");

        background->setScaleX(size.width / background->getContentSize().width);
        background->setScaleY(size.height / background->getContentSize().height);
        background->setAnchorPoint({0, 0});
        background->setColor({0, 102, 255});
        background->setZOrder(-1);

        // top right items
        CCMenu* top_menu = CCMenu::create();

        // create back button
        gd::CCMenuItemSpriteExtra* back_button = gd::CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
                this,
                menu_selector(ModListLayer::onBackButtonPress)
        );

        top_menu->addChild(back_button);
        top_menu->setPosition(ccp(25.0f, size.height - 25.0));

        // comedy
        this->setKeypadEnabled(true);

        // create mod list viewer
        ModListViewer *viewer = ModListViewer::create();

        viewer->setZOrder(2);

        // add children
        this->addChild(background);
        this->addChild(viewer);
        this->addChild(top_menu);


        return true;
    }

    void ModListLayer::onModButtonPress(CCObject *sender)
    {
        // meow
        ModListLayer *modListLayer = ModListLayer::create();

        // create scene
        CCScene *scene = CCScene::create();
        scene->addChild(modListLayer);

        // push scene
        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
    }

    void ModListLayer::onBackButtonPress(CCObject *sender)
    {
        // pop scene
        CCDirector::sharedDirector()->popScene();
    }
}