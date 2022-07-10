#pragma once

#include "../../pch.h"

namespace shimakaze {
    namespace core {
        class ModEntry {
        protected:
            toml::table config;
            COPYABLE_PERSISTENT<v8::Module> mod;
        public:
            ModEntry(toml::table config, COPYABLE_PERSISTENT<v8::Module> mod);

            toml::table getConfig();
        };
    }
}