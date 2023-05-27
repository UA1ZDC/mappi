#ifndef METEO_COMMONS_UI_CUSTOM_FONCOMBOBOX_H
#define METEO_COMMONS_UI_CUSTOM_FONCOMBOBOX_H

#include <qcombobox.h>

class FontItemDelegate : public QAbstractItemDelegate
{
  Q_OBJECT
  public:
    FontItemDelegate( QWidget* p);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const ;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const ;
};

class FontComboBox : public QComboBox
{
  Q_OBJECT
  public:
    FontComboBox( QWidget* parent = 0 );
    ~FontComboBox();

    QSize sizeHint() const;

  protected:
    bool event( QEvent* e );

};


#endif
