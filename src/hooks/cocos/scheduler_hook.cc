#include "scheduler_hook.h"

#include "../../core/core.h"
#include "../../core/handler.h"

#include "../menu_layer.h"

#include <deque>
#include <mutex>
#include <thread>
#include <tuple>
#include <any>

std::deque<std::tuple<v8::Isolate *, std::string, std::function<void(std::string, v8::Local<v8::Context>)>>> g_v8_context_runners;
std::deque<std::tuple<v8::Isolate *, v8::Local<v8::Value>, std::function<void(v8::Local<v8::Value>, v8::Local<v8::Context>)>>> g_v8_value_context_runners;

std::deque<std::function<void()>> g_main_thread_runners;
std::deque<std::tuple<std::string, std::string, COPYABLE_PERSISTENT<v8::Function>>> g_add_hook_runners;

namespace shimakaze
{
    namespace scheduler
    {
        void run_on_main_thread(std::function<void()> func)
        {
            g_main_thread_runners.push_back(func);
        }

        void add_hook_on_main_thread(std::string hook_map, std::string name, COPYABLE_PERSISTENT<v8::Function> hook)
        {
            g_add_hook_runners.push_back(std::make_tuple(hook_map, name, hook));
        }

        void run_under_context(v8::Isolate *isolate, std::string script, std::function<void(std::string, v8::Local<v8::Context>)> func)
        {
            g_v8_context_runners.push_back(std::make_tuple(isolate, script, func));
        }

        void run_value_under_context(v8::Isolate *isolate, v8::Local<v8::Value> value, std::function<void(v8::Local<v8::Value>, v8::Local<v8::Context>)> func)
        {
            g_v8_value_context_runners.push_back(std::make_tuple(isolate, value, func));
        }

        void run_context_function(v8::Isolate *isolate, std::string script, std::function<void(std::string, v8::Local<v8::Context>)> func)
        {
            in_context_runner = true;

            // create a context under this thread
            v8::Locker locker(isolate);
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);

            v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, core::g_global_context);

            // run function after thoroughly locking the current isolate to the thread
            func(script, context);

            // we're done
            v8::Unlocker unlocker(isolate);

            in_context_runner = false;
        }

        void run_value_context_function(v8::Isolate *isolate, v8::Local<v8::Value> value, std::function<void(v8::Local<v8::Value>, v8::Local<v8::Context>)> func)
        {
            in_context_runner = true;

            // create a context under this thread
            v8::Locker locker(isolate);
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);

            v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, core::g_global_context);

            // run function after thoroughly locking the current isolate to the thread
            func(value, context);

            // we're done
            v8::Unlocker unlocker(isolate);

            in_context_runner = false;
        }

    }
}

SHIMAKAZE_CALL_ARGS(CCScheduler_update, CCScheduler *, void, float dt)
{
    if (shimakaze::scheduler::scheduler_started == false)
    {
        shimakaze::scheduler::scheduler_started = true;
        shimakaze::scheduler::in_context_runner = false;
    }

    if ((g_v8_context_runners.empty() && g_v8_value_context_runners.empty() && g_main_thread_runners.empty() && g_add_hook_runners.empty()) || shimakaze::scheduler::in_context_runner == true)
    {
        // run to the game loop if we have no contexts to run
        CCScheduler_update(self, dt);

        return;
    }

    for (auto const &func : g_add_hook_runners)
    {
        std::string hook_map = std::get<0>(func);
        if (hook_map == "menulayer") {
            shimakaze::core::handler::add_hook(shimakaze::menu::menulayer_hooks, std::get<1>(func), std::get<2>(func));
        }

        // dequeue
        g_add_hook_runners.pop_front();
    }

    for (auto const &runner : g_main_thread_runners)
    {
        runner();

        // dequeue
        g_main_thread_runners.pop_front();
    }


    // run v8 contexts
    for (auto const &func : g_v8_context_runners)
    {
        if (shimakaze::scheduler::in_context_runner == true)
        {
            break;
        }

        shimakaze::scheduler::in_context_runner = true;

        // get current isolate
        v8::Isolate *isolate = std::get<0>(func);

        // create a seperate thread
        std::thread context_thread(&shimakaze::scheduler::run_context_function, isolate, std::get<1>(func), std::get<2>(func));

        context_thread.detach();

        // dequeue
        g_v8_context_runners.pop_front();
    }

    // run v8 contexts under a value
    for (auto const &func : g_v8_value_context_runners)
    {
        if (shimakaze::scheduler::in_context_runner == true)
        {
            break;
        }

        shimakaze::scheduler::in_context_runner = true;
        // get current isolate
        v8::Isolate *isolate = std::get<0>(func);

        // create a seperate thread
        std::thread context_thread(&shimakaze::scheduler::run_value_context_function, isolate, std::get<1>(func), std::get<2>(func));

        context_thread.detach();

        // dequeue
        g_v8_value_context_runners.pop_front();
    }

    // update scheduler
    CCScheduler_update(self, dt);
}