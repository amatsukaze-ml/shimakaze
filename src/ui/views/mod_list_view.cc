#include "mod_list_view.h"

#include "../cells/mod_cell.h"

namespace shimakaze::ui {
    // real brother moment right here
    ModListView *ModListView::create(cocos2d::CCArray *entries, gd::BoomListType type, float width, float height)
    {
        ModListView *instance = new ModListView();

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

    ModListView *ModListView::create(cocos2d::CCArray *entries, gd::BoomListType type, CCSize size)
    {
        ModListView *instance = new ModListView();

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

    void ModListView::setupList()
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

    gd::TableViewCell* ModListView::getListCell(const char *key)
    {
        return ModCell::create(key, {this->m_fWidth, this->m_fItemSeparation});
    }

    void ModListView::loadCell(gd::TableViewCell *cell, unsigned int index)
    {
        ModObject *object = reinterpret_cast<ModObject *>(this->m_pEntries->objectAtIndex(index));
        ModCell* modCell = reinterpret_cast<ModCell *>(cell);

        std::cout << "hi owo" << std::endl;

        modCell->setIndex(index);
        modCell->setupObjectData(object);
        modCell->updateBGColor();
    }
}