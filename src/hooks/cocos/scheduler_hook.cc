#include "scheduler_hook.h"

#include "../../core/core.h"

#include <deque>
#include <mutex>
#include <thread>

std::deque<std::function<void(v8::Local<v8::Context>)>> g_v8_context_runners;
std::deque<v8::Isolate*> g_v8_isolates;

bool in_context_runner = false;

namespace shimakaze
{
    namespace scheduler
    {
        void run_under_context(v8::Isolate* isolate, std::function<void(v8::Local<v8::Context>)> func)
        {
            g_v8_context_runners.push_back(func);
            g_v8_isolates.push_back(isolate);

            std::cout << "added runner" << std::endl;
        }
        
        void run_context_function(v8::Isolate* isolate, std::function<void(v8::Local<v8::Context>)> func)
        {
            std::cout << "thread made" << std::endl;
            if (in_context_runner == true) {
                // wait a bit, we're on a seperate thread anyways xd
                while (in_context_runner == true) {
                    if (in_context_runner == false) {
                        break;
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }

            in_context_runner = true;
            // create a context under this thread
            
            std::cout << "locking isolate to this thread" << std::endl;
            v8::Locker locker(isolate);
            std::cout << "creating isolate scope" << std::endl;
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);
            std::cout << "created scope" << std::endl;

            std::cout << "creating global object" << std::endl;
            v8::Local<v8::ObjectTemplate> global_object = core::create_global_object(isolate);
            std::cout << "created global object" << std::endl;

            std::cout << "creating ctx" << std::endl;
            v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, core::g_global_context);
            std::cout << "created ctx" << std::endl;

            // run function after thoroughly locking the current isolate to the thread
            func(context);

            // we're done
            v8::Unlocker unlocker(isolate);
            in_context_runner = false;
        }
    }
}

SHIMAKAZE_CALL_ARGS(CCScheduler_update, CCScheduler *, void, float dt)
{

    if (g_v8_context_runners.empty() || in_context_runner == true) {
        // run to the game loop if we have no contexts to run
        CCScheduler_update(self, dt);

        return;
    }

    // run v8 contexts
    for (auto const &func : g_v8_context_runners)
    {
        // get current isolate
        std::cout << "found a runner" << std::endl;
        v8::Isolate* isolate = g_v8_isolates.front();

        // create a seperate thread
            std::cout << "making thred" << std::endl;
        std::thread context_thread(&shimakaze::scheduler::run_context_function, isolate, func);

        context_thread.join();

        // dequeue
        g_v8_context_runners.pop_front();
        g_v8_isolates.pop_front();
    }

    // update scheduler
    CCScheduler_update(self, dt);
}