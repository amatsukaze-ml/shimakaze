#include "../pch.h"

#include <map>

namespace shimakaze {
    namespace bind {
        class Component {
        private:
            v8::Isolate* isolate;
            COPYABLE_PERSISTENT<v8::ObjectTemplate> object_template;
            std::map<const char*, COPYABLE_PERSISTENT<v8::Value>> values;

        public:
            Module(v8::Isolate* isolate);
        }
    }
}