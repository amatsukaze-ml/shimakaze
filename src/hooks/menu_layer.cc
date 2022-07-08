#include "menu_layer.h"

#include "../core/core.h"
#include "../core/handler.h"

#include "../bindings/menu_layer.h"

#include <format>

namespace shimakaze
{
    namespace menu
    {
        void update_mod_count(std::size_t count)
        {
            std::string modCount = std::format("{} mods loaded", count);
            mod_count_label->setString(modCount.c_str());
        }
    }
}

SHIMAKAZE_CALL(MenuLayer_init, CCLayer *, bool)
{
    if (!MenuLayer_init(self))
    {
        return false;
    }

    // size of screen
    CCSize size = CCDirector::sharedDirector()->getWinSize();

    // add shimakaze label
    std::string shimakazeVersion = std::format("Shimakaze {}", SHIMAKAZE_VERSION);
    std::string modCount = std::format("{} mods loaded", shimakaze::core::g_mod_count);

    shimakaze_label = CCLabelBMFont::create(shimakazeVersion.c_str(), "bigFont.fnt");
    mod_count_label = CCLabelBMFont::create(modCount.c_str(), "bigFont.fnt");

    shimakaze_label->setAnchorPoint({1.0f, 0.0f});
    shimakaze_label->setScale(0.25f);
    shimakaze_label->setPosition(ccp(size.width, 7.5f));

    mod_count_label->setAnchorPoint({1.0f, 0.0f});
    mod_count_label->setScale(0.25f);
    mod_count_label->setPosition(ccp(size.width, 0.0f));

    self->addChild(shimakaze_label);
    self->addChild(mod_count_label);

    // run hooks
    v8::Isolate* isolate = shimakaze::core::g_isolate;
    auto init_hooks = shimakaze::core::handler::get_hook_map(shimakaze::menu::menulayer_hooks, "init");

    for (auto &hook : init_hooks)
    {
        // create a context under this thread
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);

        v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, shimakaze::core::g_global_context);
        // convert to local
        
        v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, hook);
        // cast menulayer to functiontemplate
        v8::Local<v8::FunctionTemplate> menulayer_cast = shimakaze::bindings::cast_menulayer(isolate, self);

        v8::Local<v8::Value> argv[] = {
            menulayer_cast->GetFunction(context).ToLocalChecked()
        };

        func->Call(context, func, 1, argv);

        // we're done
        v8::Unlocker unlocker(isolate);
    }

    return true;
}