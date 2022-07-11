#include "mod_entry.h"

#include "../handler.h"

namespace shimakaze::core {
    ModEntry::ModEntry(toml::table config, COPYABLE_PERSISTENT<v8::Module> mod) {
        this->config = config;
        this->mod = mod;
    }

    toml::table ModEntry::getConfig() {
        return this->config;
    }

    void ModEntry::refresh_resources() {
        auto mod_path = this->config["__temp_mod_path"].ref<std::string>();
        shimakaze::core::handler::run_mod_resources(mod_path, this->config);
    }
}