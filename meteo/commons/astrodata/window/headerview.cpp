#include "headerview.h"
#include <QPainter>


// class SpanHeader
SpanHeader::SpanHeader(Qt::Orientation orientation, QWidget* parent /*=*/) :
    QHeaderView(orientation, parent)
{
    QObject::connect(this, SIGNAL(sectionResized(int, int, int)),
        this, SLOT(onSectionResized(int)));
}

SpanHeader::~SpanHeader()
{
    for (int rootIndex = 0; rootIndex < m_header.countRootItem(); ++rootIndex)
    {
        delete m_header.item(rootIndex).item();
    }
}

void SpanHeader::setHeaderInfo(const Header& header)
{
    m_header = header;
}

QStyleOptionHeader SpanHeader::styleOptionForCell(int logicalInd) const
{
    QStyleOptionHeader option;
    initStyleOption(&option);
    if (window()->isActiveWindow())
    {
        option.state |= QStyle::State_Active;
    }

    option.textAlignment = Qt::AlignCenter;
    option.iconAlignment = Qt::AlignVCenter;
    option.section = logicalInd;

    int visual = visualIndex(logicalInd);
    if (count() == 1)
    {
        option.position = QStyleOptionHeader::OnlyOneSection;
    }
    else
    {
        if (visual == 0)
        {
            option.position = QStyleOptionHeader::Beginning;
        }
        else
        {
            if (visual == count() - 1)
            {
                option.position = QStyleOptionHeader::End;
            }
            else
            {
                option.position = QStyleOptionHeader::Middle;
            }
        }
    }

    QItemSelectionModel* selectModel = selectionModel();
    if (selectModel != 0)
    {
        styleSelectionOptionForCell(option, selectModel, logicalInd);
    }

    return option;
}

void SpanHeader::setForegroundBrush(QStyleOptionHeader& option, const HeaderItem& item) const
{
    QVariant brush = item.item()->index().data(Qt::ForegroundRole);
    if (brush.canConvert(QMetaType::QBrush))
    {
        option.palette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(brush));
    }
}

void SpanHeader::setBackgroundBrush(QStyleOptionHeader& option, const HeaderItem& item) const
{
    QVariant brush = item.item()->index().data(Qt::BackgroundRole);
    if (brush.canConvert(QMetaType::QBrush))
    {
        option.palette.setBrush(QPalette::Button, qvariant_cast<QBrush>(brush));
        option.palette.setBrush(QPalette::Window, qvariant_cast<QBrush>(brush));
    }
}

QSize SpanHeader::cellSize(const HeaderItem& leafItem, QStyleOptionHeader styleOptions) const
{
    QSize result;
    QModelIndex index = leafItem.item()->index();
    QVariant variant(index.data(Qt::SizeHintRole));
    if (variant.isValid())
    {
        result = qvariant_cast<QSize>(variant);
    }

    QFont fnt(font());
    QVariant var(index.data(Qt::FontRole));
    if (var.isValid() && var.canConvert(QMetaType::QFont) )
    {
        fnt = qvariant_cast<QFont>(var);
    }

    fnt.setBold(true);
    QFontMetrics fm(fnt);
    QSize size(fm.size(0, index.data(Qt::DisplayRole).toString()));
    // TODO поворот
    // if (index.data(Qt::UserRole).isValid())
    // {
    //     size.transpose();
    // }

    QSize decorationsSize(style()->sizeFromContents(QStyle::CT_HeaderSection, &styleOptions, QSize(), this));
    QSize emptyTextSize(fm.size(0, QString()));
    return result.expandedTo(size + decorationsSize - emptyTextSize);
}

int SpanHeader::currentCellWidth(const HeaderItem& searchItem,
    const HeaderItem& leafItem, int sectionIndex) const
{
    HeaderItem::list_t list = m_header.leafList(searchItem);
    if (list.empty())
    {
        return sectionSize(sectionIndex);
    }

    int width = 0;
    int firstLeafSectionIndex = sectionIndex - indexOf(list, leafItem);
    for (int i = 0; i < list.size(); ++i)
    {
        width += sectionSize(firstLeafSectionIndex + i);
    }

    return width;
}

int SpanHeader::currentCellLeft(const HeaderItem& searchItem, const HeaderItem& leafItem,
    int sectionIndex, int left) const
{
    HeaderItem::list_t list = m_header.leafList(searchItem);
    if (!list.empty())
    {
        int n = indexOf(list, leafItem);
        int firstLeafSectionIndex = sectionIndex - n;
        --n;
        for (; n >= 0; --n)
        {
            left -= sectionSize(firstLeafSectionIndex + n);
        }
    }
    return left;
}

Header& SpanHeader::headerInfo()
{
    return m_header;
}

const Header& SpanHeader::headerInfo() const
{
    return m_header;
}

int SpanHeader::indexOf(const HeaderItem::list_t& list, const HeaderItem& leafItem) const
{
    for (int index = 0; index < list.size(); ++index)
    {
        if (list.at(index).item() == leafItem.item())
        {
            return index;
        }
    }
    return -1;
}

void SpanHeader::onSectionResized(int logicalIndex)
{
    if (isSectionHidden(logicalIndex))
    {
        return;
    }

    HeaderItem leafItem(headerInfo().leaf(logicalIndex));
    if (leafItem.isValid())
    {
        HeaderItem::list_t leafList(headerInfo().leafList(leafItem.parent()));
        for (int n = indexOf(leafList, leafItem); 0 < n; --n)
        {
            --logicalIndex;
            int w = viewport()->width();
            int h = viewport()->height();
            int pos = sectionViewportPosition(logicalIndex);
            QRect rect(pos, 0, w - pos, h);
            if (isRightToLeft())
            {
                rect.setRect(0, 0, pos + sectionSize(logicalIndex), h);
            }
            viewport()->update(rect.normalized());
        }
    }
}


// class HorizontalSpanHeader
HorizontalSpanHeader::HorizontalSpanHeader(QWidget* parent /*=*/) :
    SpanHeader(Qt::Horizontal, parent)
{
}

HorizontalSpanHeader::~HorizontalSpanHeader()
{
}

QSize HorizontalSpanHeader::sectionSizeFromContents(int logicalIndex) const
{
    if (0 < headerInfo().countRootItem())
    {
        HeaderItem leafItem(headerInfo().leaf(logicalIndex));
        if (leafItem.isValid())
        {
            QStyleOptionHeader styleOption(styleOptionForCell(logicalIndex));
            QSize size(cellSize(leafItem, styleOption));
            leafItem = leafItem.parent();
            while (leafItem.isValid())
            {
                size.rheight() += cellSize(leafItem, styleOption).height();
                leafItem = leafItem.parent();
            }
            return size;
        }
    }
    return QHeaderView::sectionSizeFromContents(logicalIndex);
}

void HorizontalSpanHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    if (!rect.isValid())
    {
        QHeaderView::paintSection(painter, rect, logicalIndex);
    }

    HeaderItem leafItem(headerInfo().leaf(logicalIndex));
    if (!leafItem.isValid())
    {
        return ;
    }

    QPointF oldBO(painter->brushOrigin());
    int top = rect.y();
    HeaderItem::list_t parentList(leafItem.parentList());
    for (int index = 0; index < parentList.size(); ++index)
    {
        QStyleOptionHeader realStyleOptions(styleOptionForCell(logicalIndex));
        if ((index < (parentList.size() - 1)) &&
            (realStyleOptions.state.testFlag(QStyle::State_Sunken) ||
            realStyleOptions.state.testFlag(QStyle::State_On))
        )
        {
            QStyle::State state(QStyle::State_Sunken | QStyle::State_On);
            realStyleOptions.state &= (~state);
        }

        HeaderItem cellItem = parentList.at(index);
        QStyleOptionHeader uniopt(realStyleOptions);
        setForegroundBrush(uniopt, cellItem);
        setBackgroundBrush(uniopt, cellItem);
        int height = cellSize(cellItem, uniopt).height();
        if (cellItem.item() == leafItem.item())
        {
            height = rect.height() - top;
        }

        QRect r(currentCellLeft(cellItem, leafItem, logicalIndex, rect.left()),
            top,
            currentCellWidth(cellItem, leafItem, logicalIndex),
            height
        );

        uniopt.text = cellItem.text();
        painter->save();
        uniopt.rect = r;
        if (cellItem.isRotateText())
        {
            style()->drawControl(QStyle::CE_HeaderSection, &uniopt, painter, this);
            QMatrix matrix;
            matrix.rotate(-90);
            painter->setWorldMatrix(matrix, true);
            QRect newRect(0, 0,  r.height(), r.width());
            newRect.moveCenter(QPoint(-r.center().y(), r.center().x()));
            uniopt.rect = newRect;
            style()->drawControl(QStyle::CE_HeaderLabel, &uniopt, painter, this);
        }
        else
        {
            style()->drawControl(QStyle::CE_Header, &uniopt, painter, this);
        }

        painter->restore();
        top += height;
    }

    painter->setBrushOrigin(oldBO);
}

void HorizontalSpanHeader::styleSelectionOptionForCell(QStyleOptionHeader& option,
    QItemSelectionModel* selectModel, int logicalInd) const
{
    int visual = visualIndex(logicalInd);
    bool isPreviousSelected = selectModel->isColumnSelected(logicalIndex(visual - 1), rootIndex());
    bool isNextSelected = selectModel->isColumnSelected(logicalIndex(visual + 1), rootIndex());

    option.selectedPosition = QStyleOptionHeader::NotAdjacent;
    if (isPreviousSelected && isNextSelected)
    {
        option.selectedPosition = QStyleOptionHeader::NextAndPreviousAreSelected;
    }
    else if (isPreviousSelected)
    {
        option.selectedPosition = QStyleOptionHeader::PreviousIsSelected;
    }
    else if (isNextSelected)
    {
        option.selectedPosition = QStyleOptionHeader::NextIsSelected;
    }

    if (sectionsClickable() && highlightSections())
    {
        if (selectModel->columnIntersectsSelection(logicalInd, rootIndex()))
        {
            option.state |= QStyle::State_On;
        }

        if (selectModel->isColumnSelected(logicalInd, rootIndex()))
        {
            option.state |= QStyle::State_Sunken;
        }
    }
}
