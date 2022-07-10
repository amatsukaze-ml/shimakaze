#include "mod_object.h"

namespace shimakaze
{
    namespace ui
    {
        ModObject *ModObject::create(core::ModEntry *entry)
        {
            ModObject *obj = new ModObject(entry);
            if (obj)
            {
                obj->autorelease();

                return obj;
            }
            else
            {
                CC_SAFE_DELETE(obj);

                return nullptr;
            }
        }

        ModObject::ModObject(core::ModEntry *entry)
        {
            this->entry = entry;
        }

        std::string ModObject::getName()
        {
            auto entry = this->entry;
            auto mod_config = entry->getConfig();
            
            std::optional<std::string> mod_name = mod_config["mod"]["info"]["display_name"].value<std::string>();
            return mod_name ? *mod_name : "Unnamed Mod";
        }

        CCSprite* ModObject::getIcon() {
            auto entry = this->entry;
            auto mod_config = entry->getConfig();
            
            std::optional<std::string> mod_id = mod_config["mod"]["mod_id"].value<std::string>();

            CCSprite* icon = CCSprite::create(std::format("{}-icon.png", *mod_id).c_str());

            if (!icon) {
                // no icon, fallback
                return CCSprite::create("BE_square_001_small.png");
            }

            return icon;
        }

        toml::array ModObject::getAuthors() {
            auto entry = this->entry;
            auto mod_config = entry->getConfig();

            auto view = mod_config["mod"]["info"]["authors"];

            if (view) {
                // there is an entry
                if (view.is_array()) {
                    // it's an array
                    return view.ref<toml::array>();
                }
                else {
                    // it's a string
                    return toml::array { view.ref<std::string>() };
                }
            }

            // no entry
            return toml::array { "Unknown" };
        }
    }
}