#ifndef METEO_COMMONS_UI_CUSTOM_STRETCHCOMBOBOX_H
#define METEO_COMMONS_UI_CUSTOM_STRETCHCOMBOBOX_H

#include <qcombobox.h>

class StretchItemDelegate : public QAbstractItemDelegate
{
  Q_OBJECT
  public:
    StretchItemDelegate( QWidget* p);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const ;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const ;
};

class StretchComboBox : public QComboBox
{
  Q_OBJECT
  public:
    StretchComboBox( QWidget* parent = 0 );
    ~StretchComboBox();

    QSize sizeHint() const;

  protected:
    bool event( QEvent* e );

};


#endif
