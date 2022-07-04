#pragma once

#include "../../pch.h"

namespace shimakaze
{
    namespace bind
    {
        class Module
        {
        private:
            v8::Isolate *isolate;
            v8::Local<v8::ObjectTemplate> object_template;

        public:
            Module(v8::Isolate *isolate);

            v8::Isolate *get_isolate();
            v8::Local<v8::ObjectTemplate> get_template();
            v8::Local<v8::Object> construct();

            template <typename Instance>
            Module &set(const char *name, v8::Local<Instance> value);

            template <typename Var>
            Module &variable(const char *name, Var &value);

            template <typename Const>
            Module &constant(const char *name, Const &value);

            Module &function(const char *name, v8::FunctionCallback func);

            Module &submodule(const char *name, Module &module);
        };

    }
}