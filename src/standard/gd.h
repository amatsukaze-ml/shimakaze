#pragma once

#include "../pch.h"

namespace shimakaze
{
    namespace standard
    {
        v8::Local<v8::Module> create_gd_module(v8::Isolate *isolate);
    }
}