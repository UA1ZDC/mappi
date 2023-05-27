#ifndef TPLUGINS_H
#define TPLUGINS_H

#include <customwidget.h>
#include <qobject.h>
#include <qlist.h>

class TPlugins : public QObject, public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
  public:
    TPlugins( QObject* parent = 0 );
    QList<QDesignerCustomWidgetInterface*> customWidgets() const { return widgets_; }

  private:
    QList<QDesignerCustomWidgetInterface*> widgets_;
};

#endif
