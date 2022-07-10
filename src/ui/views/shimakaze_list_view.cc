#include "shimakaze_list_view.h"

#include "../cells/shimakaze_cell.h"

namespace shimakaze::ui
{
    ShimakazeListView *ShimakazeListView::create(cocos2d::CCArray *entries, gd::BoomListType type, float width, float height)
    {
        ShimakazeListView *instance = new ShimakazeListView();

        if (instance && instance->init(entries, type, width, height))
        {
            instance->autorelease();

            return instance;
        }
        else
        {
            CC_SAFE_DELETE(instance);

            return nullptr;
        }
    }

    ShimakazeListView *ShimakazeListView::create(cocos2d::CCArray *entries, gd::BoomListType type, CCSize size)
    {
        ShimakazeListView *instance = new ShimakazeListView();

        if (instance && instance->init(entries, type, size.width, size.height))
        {
            instance->autorelease();

            return instance;
        }
        else
        {
            CC_SAFE_DELETE(instance);

            return nullptr;
        }
    }

    void ShimakazeListView::setupList()
    {
        unsigned int count = this->m_pEntries->count();

        this->m_fItemSeparation = 90.0f;

        if (count)
        {
            this->m_pTableView->reloadData();

            if (count == 1)
            {
                this->m_pTableView->moveToTopWithOffset(this->m_fItemSeparation);
            }
            else
            {
                this->m_pTableView->moveToTop();
            }
        }
    }

    gd::TableViewCell* ShimakazeListView::getListCell(const char *key)
    {
        return ShimakazeCell::create(key, {this->m_fWidth, this->m_fItemSeparation});
    }

    void ShimakazeListView::loadCell(gd::TableViewCell *cell, unsigned int index)
    {
        CCObject *object = reinterpret_cast<CCObject *>(this->m_pEntries->objectAtIndex(index));
        ShimakazeCell* shima_cell = reinterpret_cast<ShimakazeCell *>(cell);

        shima_cell->setIndex(index);
        shima_cell->setupObjectData(object);
        shima_cell->updateBGColor();
    }
}