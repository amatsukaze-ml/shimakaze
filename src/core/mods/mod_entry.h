#pragma once

#include "../../pch.h"

namespace shimakaze::core {
    class ModEntry {
    protected:
        toml::table config;
        COPYABLE_PERSISTENT<v8::Module> mod;
    public:
        ModEntry(toml::table config, COPYABLE_PERSISTENT<v8::Module> mod);

        toml::table getConfig();
        void refresh_resources();
    };
}