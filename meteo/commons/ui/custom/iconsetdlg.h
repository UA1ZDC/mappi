#ifndef METEO_COMMONS_UI_CUSTOM_ICONSETDLG_H
#define METEO_COMMONS_UI_CUSTOM_ICONSETDLG_H

#include <qdialog.h>
#include <qlist.h>
#include <qpair.h>
#include <qicon.h>

class QToolButton;
class QGridLayout;

class IconsetDlg: public QDialog
{
  Q_OBJECT
public:
  explicit IconsetDlg(QWidget* parent = 0);

  void  setIcons( const QList< QPair< QIcon, QString > >& icons );

  int currentIndex() const { return index_; }
  QIcon currentIcon() const ;

private slots:
  void slotChangeIcon();

private:
  int index_;
  QList< QPair< QIcon, QString> > icons_;
  QList<QToolButton*> buttons_;
  QGridLayout* layout_;
};

#endif
