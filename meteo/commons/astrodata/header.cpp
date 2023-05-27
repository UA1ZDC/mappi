#include "header.h"


// class HeaderItem
HeaderItem::HeaderItem(QStandardItem* item, bool isRotateText /*=*/) :
    m_item(item)
{

    if (m_item != 0)
    {
        m_item->setData(isRotateText, Qt::UserRole);
    }
}

HeaderItem::~HeaderItem()
{
    m_item = 0;
}

QStandardItem* HeaderItem::item()
{
    return m_item;
}

QStandardItem* HeaderItem::item() const
{
    return m_item;
}

QString HeaderItem::text() const
{
    return (m_item != 0 ? m_item->text() : QString());
}

bool HeaderItem::isRotateText() const
{
    return m_item->data(Qt::UserRole).toBool();
}

bool HeaderItem::isRoot() const
{
    return (m_item != 0 ? m_item->parent() == 0 : false);
}

bool HeaderItem::isValid() const
{
    return (m_item != 0);
}

int HeaderItem::countChild() const
{
    return (m_item != 0 ? m_item->columnCount() : 0);
}

bool HeaderItem::hasChild() const
{
    return (countChild() != 0);
}

void HeaderItem::appendChild(QStandardItem* item)
{
    m_item->appendColumn(QList<QStandardItem*>() << item);
}

void HeaderItem::appendChild(const HeaderItem& item)
{
    m_item->appendColumn(QList<QStandardItem*>() << item.item());
}

HeaderItem HeaderItem::child(int index) const
{
    return HeaderItem(m_item->child(0, index));
}

HeaderItem::list_t HeaderItem::childList() const
{
    HeaderItem::list_t list;
    for (int index = 0; index < countChild(); ++index)
    {
        list.append(child(index));
    }
    return list;
}

HeaderItem HeaderItem::parent() const
{
    return HeaderItem(m_item != 0 ? m_item->parent() : 0);
}

HeaderItem::list_t HeaderItem::parentList() const
{
    HeaderItem::list_t list;
    QStandardItem* item = m_item;
    while (item != 0)
    {
        list.push_front(HeaderItem(item));
        item = item->parent();
    }
    return list;
}


// class Header
Header::Header()
{
}

Header::~Header()
{
}

void Header::appendItem(const HeaderItem& item)
{
    m_list.append(item);
}

HeaderItem Header::item(int index) const
{
    return m_list.at(index);
}

int Header::countRootItem() const
{
    return m_list.count();
}

HeaderItem::list_t Header::rootList() const
{
    return m_list;
}

HeaderItem Header::leaf(int sectionIndex) const
{
    int leafIndex = -1;
    for (int index = 0; index < countRootItem(); ++index)
    {
        HeaderItem item(leaf(m_list.at(index), sectionIndex, leafIndex));
        if (item.isValid())
        {
            return item;
        }
    }

    return HeaderItem(0);
}

HeaderItem::list_t Header::leafList(int sectionIndex) const
{
    HeaderItem::list_t list;
    if (sectionIndex < countRootItem())
    {
        HeaderItem leafItem(item(sectionIndex));
        for (int index = 0; index < leafItem.countChild(); ++index)
        {
            list.append(leafList(leafItem.child(index)));
        }
    }
    return list;
}

HeaderItem::list_t Header::leafList(const HeaderItem& item) const
{
    HeaderItem::list_t list;
    if (item.isValid())
    {
        if (item.hasChild())
        {
            for (int index = 0; index < item.countChild(); ++index)
            {
                list.append(leafList(item.child(index)));
            }
        }
        else
        {
            list.append(item);
        }
    }
    return list;
}

HeaderItem Header::leaf(const HeaderItem& item, int sectionIndex, int& leafIndex) const
{
    if (item.isValid())
    {
        if (item.hasChild())
        {
            for (int index = 0; index < item.countChild(); ++index)
            {
                HeaderItem leafItem(leaf(item.child(index), sectionIndex, leafIndex));
                if (leafItem.isValid())
                {
                    return leafItem;
                }
            }
        }
        else
        {
            ++leafIndex;
            if (leafIndex == sectionIndex)
            {
                return item;
            }
        }
    }

    return HeaderItem(0);
}
