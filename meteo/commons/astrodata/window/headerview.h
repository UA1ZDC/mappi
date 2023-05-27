#ifndef _HEADER_VIEW_H_
#define _HEADER_VIEW_H_

#include <meteo/commons/astrodata/header.h>
#include <QHeaderView>


/** @brief */
class SpanHeader :
    public QHeaderView
{
    Q_OBJECT
public :
    SpanHeader(Qt::Orientation orientation, QWidget* parent = 0);
    virtual ~SpanHeader();

    void setHeaderInfo(const Header& m_header);

protected :
    virtual void styleSelectionOptionForCell(QStyleOptionHeader& option,
        QItemSelectionModel* selectModel, int logicalInd) const = 0;

    QStyleOptionHeader styleOptionForCell(int logicalIndex) const;
    void setForegroundBrush(QStyleOptionHeader& opt, const HeaderItem& item) const;
    void setBackgroundBrush(QStyleOptionHeader& opt, const HeaderItem& item) const;

    QSize cellSize(const HeaderItem& headerItem, QStyleOptionHeader styleOptions) const;
    int currentCellWidth(const HeaderItem& searchedItem, const HeaderItem& leafItem, int sectionIndex) const;
    int currentCellLeft(const HeaderItem& searchedItem, const HeaderItem& leafItem,
                        int sectionIndex, int left) const;

    Header& headerInfo();
    const Header& headerInfo() const;

private :
    int indexOf(const HeaderItem::list_t& list, const HeaderItem& leafItem) const;

private slots :
    void onSectionResized(int logicalIndex);

private :
    Header m_header;
};


/** */
class HorizontalSpanHeader :
    public SpanHeader
{
public :
    HorizontalSpanHeader(QWidget* parent = 0);
    virtual ~HorizontalSpanHeader();

protected :
    virtual QSize sectionSizeFromContents(int logicalIndex) const;
    virtual void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const;

    virtual void styleSelectionOptionForCell(QStyleOptionHeader& option,
        QItemSelectionModel* selectModel, int logicalInd) const;
};

#endif // _HEADER_VIEW_H_
