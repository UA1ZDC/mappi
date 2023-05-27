#ifndef METEO_COMMONS_UI_CUSTOM_DESIGNERPLUGIN_TPLUGINS_H
#define METEO_COMMONS_UI_CUSTOM_DESIGNERPLUGIN_TPLUGINS_H

#include <qglobal.h>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#include <QtUiPlugin/customwidget.h>
#else
#include <QtDesigner/customwidget.h>
#endif

#include <qobject.h>
#include <qlist.h>

class TPlugins : public QObject, public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "meteodesigner" FILE "meteodesigner.json" )
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
  public:
    TPlugins( QObject* parent = 0 );
    QList<QDesignerCustomWidgetInterface*> customWidgets() const { return widgets_; }

  private:
    QList<QDesignerCustomWidgetInterface*> widgets_;
};

#endif
