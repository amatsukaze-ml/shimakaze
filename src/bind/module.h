#pragma once

#include "../pch.h"

#include <map>

namespace shimakaze
{
    namespace bind
    {
        class Module
        {
        private:
            v8::Isolate *isolate;
            v8::Local<v8::ObjectTemplate> object_template;
            std::map<const char *, v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>>> values;

        public:
            Module(v8::Isolate *isolate);

            v8::Isolate *get_isolate();
            v8::Local<v8::ObjectTemplate> get_template();
            std::map<const char *, v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>>> get_values();
            v8::Local<v8::Object> construct();

            template <typename Instance>
            Module &set(const char *name, v8::Local<Instance> value);

            Module &value(const char *name, v8::Local<v8::Value> value, bool add_to_map);
            Module &value(const char *name, v8::Local<v8::ObjectTemplate> value, bool add_to_map);   // die
            Module &value(const char *name, v8::Local<v8::FunctionTemplate> value, bool add_to_map); // die

            template <typename Var>
            Module &variable(const char *name, Var &value);

            template <typename Const>
            Module &constant(const char *name, v8::Local<Const> &value);

            Module &function(const char *name, v8::FunctionCallback func);

            Module &submodule(const char *name, Module &module);

            Module &merge(Module &module);
        };
    }
}