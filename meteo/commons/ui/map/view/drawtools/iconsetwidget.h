#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_ICONSETWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_ICONSETWIDGET_H

#include <qwidget.h>
#include <qlist.h>
#include <qpair.h>
#include <qicon.h>

class QToolButton;
class QGridLayout;

namespace meteo {
namespace map {

class IconsetWidget : public QWidget
{
  Q_OBJECT
public:
  explicit IconsetWidget(QWidget* parent = nullptr);
  void  setIcons( const QList< QPair< QIcon, QString > >& icons );
  void loadIconsFromPaths(const QStringList& paths );

  int currentIndex() const { return index_; }
  QIcon currentIcon() const ;
  QString currentIconPath() const;
  bool setCurrentIconIndx(int index);
  int iconsSize() { return icons_.size(); }

private slots:
  void slotChangeIcon();

private:
  int index_;
  QList< QPair< QIcon, QString> > icons_;
  QList<QToolButton*> buttons_;
  QGridLayout* layout_;

signals:
  void iconChanged(int);

};

}
}

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_ICONSETWIDGET_H
