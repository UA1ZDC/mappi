#ifndef _HEADER_H_
#define _HEADER_H_

#include <QList>
#include <QStandardItem>


/** @brief */
class HeaderItem
{
public :
    typedef QList<HeaderItem> list_t;

public :
    HeaderItem(QStandardItem* item, bool isRotateText = false);
    ~HeaderItem();

    QStandardItem* item();
    QStandardItem* item() const;

    QString text() const;
    bool isRotateText() const;

    bool isRoot() const;
    bool isValid() const;

    int countChild() const;
    bool hasChild() const;
    void appendChild(QStandardItem* item);
    void appendChild(const HeaderItem& item);
    HeaderItem child(int index) const;
    HeaderItem::list_t childList() const;

    HeaderItem parent() const;
    HeaderItem::list_t parentList() const;

private :
    QStandardItem* m_item;
};


/** @brief */
class Header
{
public :
    Header();
    ~Header();

    void appendItem(const HeaderItem& item);
    HeaderItem item(int index) const;

    int countRootItem() const;
    HeaderItem::list_t rootList() const;

    HeaderItem leaf(int sectionIndex) const;
    HeaderItem::list_t leafList(int sectionIndex) const;
    HeaderItem::list_t leafList(const HeaderItem& item) const;

private :
    HeaderItem leaf(const HeaderItem& item, int sectionIndex, int& leafIndex) const;

private :
    HeaderItem::list_t m_list;
};

#endif // _HEADER_H_
