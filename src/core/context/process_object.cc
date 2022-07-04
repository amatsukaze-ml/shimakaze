#include "process_object.h"

namespace shimakaze
{
    namespace context
    {
        // process functions
        void process_print(const v8::FunctionCallbackInfo<v8::Value> &args)
        {
            bool first = true;
            for (int i = 0; i < args.Length(); i++)
            {
                v8::HandleScope handle_scope(args.GetIsolate());
                if (first)
                {
                    first = false;
                }
                else
                {
                    printf(" ");
                }
                v8::String::Utf8Value str(args.GetIsolate(), args[i]);
                const char *cstr = *str;
                printf("%s", cstr);
            }
            printf("\n");
            fflush(stdout);
        }

        // create process object
        bind::Module& create_process_object(v8::Isolate *isolate)
        {
            bind::Module module(isolate);

            // process.print(string)
            module.function("print", process_print);

            return module;
        }
    }
}