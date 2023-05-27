#ifndef METEO_COMMONS_UI_CUSTOM_RESIZEDMBOBOX_H
#define METEO_COMMONS_UI_CUSTOM_RESIZEDMBOBOX_H

#include <qcombobox.h>

class ResizedComboItemDelegate : public QAbstractItemDelegate
{
  Q_OBJECT
  public:
    ResizedComboItemDelegate( QWidget* p);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const ;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const ;
};

class ResizedComboBox : public QComboBox
{
  Q_OBJECT
  public:
    ResizedComboBox( QWidget* parent = 0 );
    ~ResizedComboBox();

    QSize sizeHint() const;

  protected:
   // void showPopup ();
    bool event( QEvent* e );

};


#endif
