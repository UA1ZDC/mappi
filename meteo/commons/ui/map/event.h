#ifndef METEO_MAP_EVENT_H
#define METEO_MAP_EVENT_H

#include <qobject.h>
#include <qevent.h>
#include <qlist.h>

namespace meteo {
namespace map {

class EventHandler;
class Layer;

class MapEvent : public QEvent
{
  public:
    static const QEvent::Type MapChanged;

    MapEvent();
    ~MapEvent();
};

class DocumentEvent : public QEvent
{
  public:
    static const QEvent::Type DocumentChanged;
    enum ChangeType {
      Changed           = 0,
      IncutChanged      = 1,
      LegendChanged     = 2,
      DocumentLoaded    = 3,
      NameChanged       = 4,

    };

    DocumentEvent( ChangeType cht );
    ~DocumentEvent();

    ChangeType changeType() const { return changetype_; }

  private:
    ChangeType changetype_;

};

class LayerEvent : public QEvent
{
  public:
    static const QEvent::Type LayerChanged;
    enum ChangeType {
      Deleted           = 0,
      Visibility        = 1,
      Added             = 2,
      Activity          = 3,
      Moved             = 4,
      ObjectChanged     = 5
    };

    LayerEvent( const QString& uuid, ChangeType changetype, const QRect& rect = QRect() );
    ~LayerEvent();

    const QString& layer() const { return uuid_; }
    ChangeType changeType() const { return changetype_; }
    const QRect& changerect() const { return changerect_; }

  private:
    QString uuid_;
    ChangeType changetype_;
    QRect changerect_;
};

class EventHandler : public QObject
{
  Q_OBJECT
  public:
    EventHandler();
    ~EventHandler();

    void notifyMapChanges();
    void notifyDocumentChanges( DocumentEvent::ChangeType changetype );
    void notifyLayerChanges( Layer* layer, LayerEvent::ChangeType changetype );
    void postEvent( QEvent* ev );

    bool event( QEvent* event );

    void muteEvents();
    void turnEvents();

    bool muted() const { return muted_; }

  protected:
    bool eventFilter( QObject* watched, QEvent* e );

  private:
    bool muted_;

  Q_SIGNALS:
    void mapChanged();
    void layerChanged( map::Layer* layer, int ch );
};

}
}

#endif
