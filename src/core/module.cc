#include "module.h"

namespace shimakaze
{
    namespace module
    {
        v8::MaybeLocal<v8::Promise> dynamic_call(v8::Local<v8::Context> context, v8::Local<v8::Data> host_defined_options, v8::Local<v8::Value> resource_name, v8::Local<v8::String> specifier, v8::Local<v8::FixedArray> import_assertions)
        {
            // create a promise for rejection and resolution
            v8::Local<v8::Promise::Resolver> promise_resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
            v8::MaybeLocal<v8::Promise> promise(promise_resolver->GetPromise());

            // get the specifier name
            v8::String::Utf8Value specifier_name(context->GetIsolate(), specifier);

            std::cout << "dynamic_call: " << bind::to_cstr(specifier_name) << std::endl;

            promise_resolver->Resolve(context, specifier);
            return promise;
        }

        v8::MaybeLocal<v8::Module> static_call(v8::Local<v8::Context> context, v8::Local<v8::String> specifier, v8::Local<v8::FixedArray> import_assertions, v8::Local<v8::Module> referrer)
        {
            v8::String::Utf8Value specifier_name(context->GetIsolate(), specifier);
            const char* specifier_name_cstr = bind::to_cstr(specifier_name);

            context->GetIsolate()->ThrowException(v8::Exception::ReferenceError(bind::to_v8(context->GetIsolate(), "get the fuck out of here with that shit")));

            // todo: this is fucking bad LOL
            // so basically i need to make proper module importation
            v8::MaybeLocal<v8::Module> mod = v8::MaybeLocal<v8::Module>();
            return mod;
        }

        void import_meta(v8::Local<v8::Context> context, v8::Local<v8::Module> module, v8::Local<v8::Object> meta)
        {
            // do nothing for now
            meta->Set(
                context,
                bind::to_v8(context->GetIsolate(), "url"),
                bind::to_v8(context->GetIsolate(), "http://example.com/"));
        }
    }
}