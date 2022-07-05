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
            return isolate;
        }

        v8::Local<v8::ObjectTemplate> Module::get_template()
        {
            return object_template;
        }

        std::map<const char *, v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>>> Module::get_values()
        {
            return values;
        }

        v8::Local<v8::Object> Module::construct()
        {
            v8::HandleScope construct_scope(isolate);
            v8::MaybeLocal<v8::Object> maybe_object = object_template->NewInstance(isolate->GetCurrentContext());
            v8::Local<v8::Object> object = maybe_object.ToLocalChecked();

            return object;
        }

        template <typename Instance>
        Module &Module::set(const char *name, v8::Local<Instance> value)
        {
            object_template->Set(isolate, name, value);

            return *this;
        }

        Module &Module::value(const char *name, v8::Local<v8::Value> value, bool add_to_map = false)
        {
            object_template->Set(isolate, name, value);

            if (add_to_map)
            {
                // store the object into a persistent scope
                v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>> persistent(isolate, value);
                values.insert(
                    std::pair(
                        name,
                        persistent));
            }

            return *this;
        }

        Module &Module::value(const char *name, v8::Local<v8::ObjectTemplate> value, bool add_to_map = false)
        {
            object_template->Set(isolate, name, value);

            // we just can't add it to the map at all lol

            return *this;
        }
        Module &Module::value(const char *name, v8::Local<v8::FunctionTemplate> value, bool add_to_map = false)
        {
            object_template->Set(isolate, name, value);

            // we just can't add it to the map at all lol

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
        Module &Module::constant(const char *name, v8::Local<Const> &value)
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
            value(name, v8::FunctionTemplate::New(isolate, func));

            return *this;
        }

        Module &Module::submodule(const char *name, Module &module)
        {
            // depersist the template
            // why do i have to do this?
            object_template->Set(bind::to_v8(isolate, name), module.get_template());

            return *this;
        }

        Module &Module::merge(Module &module)
        {
            // get the module's values
            auto module_values = module.get_values();

            // merge the module's values into this module
            for (auto &[key, value] : module_values)
            {
                // convert it back to local
                v8::Local<v8::Value> local = v8::Local<v8::Value>::New(isolate, value);

                // set the value
                set(key, local);
            }

            return *this;
        }
    }
}