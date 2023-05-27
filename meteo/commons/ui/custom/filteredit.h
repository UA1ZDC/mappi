#ifndef METEO_COMMONS_UI_CUSTOM_FILTEREDIT_H
#define METEO_COMMONS_UI_CUSTOM_FILTEREDIT_H

#include <qwidget.h>
#include <qmenu.h>
#include <qlineedit.h>

class FilterEdit : public QLineEdit
{
  Q_OBJECT
  public:
    FilterEdit( QWidget* parent, QAction* a );
    ~FilterEdit();

    bool hasFilter() const ;

    QString filter() const { return QLineEdit::text(); }
    void setFilter( const QString& filter );

  protected:
    void keyPressEvent( QKeyEvent* e );

  private:
    QAction* action_;
};

#endif
