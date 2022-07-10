#include "mod_entry.h"

namespace shimakaze {
    namespace core {
        ModEntry::ModEntry(toml::table config, COPYABLE_PERSISTENT<v8::Module> mod) {
            this->config = config;
            this->mod = mod;
        }

        toml::table ModEntry::getConfig() {
            return this->config;
        }
    }
}