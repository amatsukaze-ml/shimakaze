#include "mod_cell.h"

namespace shimakaze
{
    namespace ui
    {
        void ModCell::setupObjectData(ModObject *object)
        {
            // set mod name
            CCLabelBMFont *name = CCLabelBMFont::create(object->getName().c_str(), "bigFont.fnt");

            name->setScale(0.65f);
            name->setAnchorPoint({0.0f, 0.5f});
            name->setPosition(ccp(72.5f, this->m_fHeight - 27.5f));

            // set mod authors
            std::string authors = "by ";
            int i = 0;

            for (const auto &author : object->getAuthors())
            {
                if (i > 2 && i < object->getAuthors().size() - 1)
                {
                    authors += ", and more";
                    break;
                }

                if (i == 0)
                {
                    authors += author.ref<std::string>();
                }
                else
                {
                    authors += std::format(", {}", author.ref<std::string>());
                }
            }

            CCLabelBMFont *authors_label = CCLabelBMFont::create(authors.c_str(), "goldFont.fnt");

            authors_label->setScale(0.55f);
            authors_label->setAnchorPoint({0.0f, 0.5f});
            authors_label->setPosition(ccp(72.5f, this->m_fHeight - 41.5f));

            // set mod icon
            // also known as the worst section of this entire initialization process

            // I'm sorry
            //  - Github Copilot 

            CCSprite *icon = object->getIcon();
            CCSprite *icon_border = CCSprite::createWithSpriteFrameName("squareBorder.png");

            icon_border->setScale(1.175f);
            icon_border->setAnchorPoint({0.0f, 0.0f});
            icon_border->setPosition({18.5f, (this->m_fHeight / 2) - 24.5f});

            // laica code no tocuhy
            const float icon_scalar = 1.1f;
            icon->setScaleX((icon_border->getContentSize().width / icon->getContentSize().width) * icon_scalar);
            icon->setScaleY((icon_border->getContentSize().height / icon->getContentSize().height) * icon_scalar);
            icon->setPosition({ icon_border->getContentSize().width / 2 + 0.025f, icon_border->getContentSize().height / 2 + 2.0f });

            icon->setAnchorPoint({ 0.0f, 0.0f });

            // add icon as child

            // set bg opacity
            this->m_pBGLayer->setOpacity(0xFF);

            // add children
            this->m_pLayer->addChild(name);
            this->m_pLayer->addChild(icon);
            this->m_pLayer->addChild(icon_border);
            this->m_pLayer->addChild(authors_label);
            this->m_pLayer->setVisible(true);
        }

        void ModCell::onClick(CCObject *sender)
        {
        }

        void ModCell::FLAlert_Clicked(gd::FLAlertLayer *alert, bool btn2)
        {
            if (btn2)
            {
            }
        }
    }
}