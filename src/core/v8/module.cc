#include "module.h"

namespace shimakaze
{
    namespace bind
    {
        Module::Module(v8::Isolate *isolate)
        {
            this->isolate = isolate;
            this->object_template = v8::ObjectTemplate::New(isolate);
        }

        v8::Isolate *Module::get_isolate()
        {
            return this->isolate;
        }

        v8::Local<v8::ObjectTemplate> Module::get_template()
        {
            return object_template;
        }

        v8::Local<v8::Object> Module::construct()
        {
            return object_template->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
        }

        template <typename Instance>
        Module &Module::set(const char *name, v8::Local<Instance> value)
        {
            object_template->Set(isolate, name, value);
            return *this;
        }

        template <typename Var>
        Module &Module::variable(const char *name, Var &value)
        {
            v8::HandleScope var_scope(isolate);

            object_template->SetAccessor(
                v8::String::NewUtf8Value(isolate, name),
                [](v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value> &info)
                {
                    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
                    v8::Local<v8::Object> self = info.Holder();
                    v8::Local<v8::Value> value = self->Get(context, property).ToLocalChecked();
                    info.GetReturnValue().Set(value);
                },
                [](v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void> &info)
                {
                    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
                    v8::Local<v8::Object> self = info.Holder();
                    self->Set(context, property, value);
                });
            return *this;
        }

        template <typename Const>
        Module &Module::constant(const char *name, Const &value)
        {
            v8::HandleScope var_scope(isolate);

            object_template->Set(isolate,
                                 name,
                                 value,
                                 v8::PropertyAttribute(v8::ReadOnly | v8::DontDelete));

            return *this;
        }

        Module &Module::function(const char *name, v8::FunctionCallback func)
        {
            object_template->Set(isolate, name, v8::FunctionTemplate::New(isolate, func));
            return *this;
        }

        Module &Module::submodule(const char *name, Module &module)
        {
            object_template->Set(isolate, name, module.get_template());
            return *this;
        }
    }
}