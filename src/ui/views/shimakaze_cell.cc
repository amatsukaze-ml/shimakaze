#include "shimakaze_cell.h"

namespace shimakaze
{
    namespace ui
    {
        ShimakazeCell *ShimakazeCell::create(const char *key, CCSize size)
        {
            ShimakazeCell *instance = new ShimakazeCell(key, size);

            if (instance)
            {
                return instance;
            }
            else
            {
                CC_SAFE_DELETE(instance);

                return nullptr;
            }
        }

        void ShimakazeCell::setIndex(unsigned int index)
        {
            this->index = index;
        }

        ShimakazeCell::ShimakazeCell(const char *name, CCSize size) : gd::TableViewCell(name, size.width, size.height)
        {
            return;
        }

        void ShimakazeCell::setupObjectData(CCObject *object)
        {
            return;
        }

        void ShimakazeCell::updateBGColor()
        {
            if (this->index & 1)
            {
                this->m_pBGLayer->setColor(ccc3(0xC2, 0x72, 0x3E));
            }
            else
            {
                this->m_pBGLayer->setColor(ccc3(0xA1, 0x58, 0x2C));
            }

            this->m_pBGLayer->setOpacity(0xFF);
        }

        void ShimakazeCell::onClick(CCObject *sender)
        {
        return;
        }

        void ShimakazeCell::FLAlert_Clicked(gd::FLAlertLayer *alert, bool btn2)
        {
            return;
        }
    }
}