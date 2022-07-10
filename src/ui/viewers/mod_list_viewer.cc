#include "mod_list_viewer.h"

#include "../objects/mod_object.h"

#include "../views/mod_list_view.h"

#include "../../core/handler.h"

namespace shimakaze::ui
{
    ModListViewer *ModListViewer::create()
    {
        ModListViewer *viewer = new ModListViewer();

        // create mod view

        if (viewer && viewer->init())
        {
            viewer->autorelease();

            return viewer;
        }
        else
        {
            CC_SAFE_DELETE(viewer);

            return nullptr;
        }
    }

    bool ModListViewer::init() {
        // size of screen
        CCSize size = CCDirector::sharedDirector()->getWinSize();

        CCArray *ccMods = CCArray::create();

        for (const auto &mod : core::handler::g_mod_map)
        {
            ccMods->addObject(ModObject::create(mod.second));
        }

        ModListView *view = ModListView::create(ccMods, gd::BoomListType::kBoomListTypeDefault, {356.0f, 220.0f});

        // create list layer
        this->list_layer = gd::GJListLayer::create(view, "Mods", {0, 0, 0, 180}, 356.0f, 220.0f);

        this->list_layer->setPosition(size / 2.0f - this->list_layer->getScaledContentSize() / 2.0f);
        this->list_layer->setAnchorPoint({ 0.5f, 0.5f });

        this->addChild(this->list_layer);

        // add a message stating that there are no mods installed
        if (core::handler::g_mod_map.empty())
        {
            CCLabelBMFont* label = CCLabelBMFont::create("No mods are installed.", "bigFont.fnt");
            label->setPosition(this->list_layer->getScaledContentSize() / 2);
            label->setAnchorPoint({ 0.5f, 0.5f });
            label->setScale(0.75f);

            this->list_layer->addChild(label);
        }

        // return self init
        return true;
    }
}