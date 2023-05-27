#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_UNDOREDO_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_UNDOREDO_H

#include <qundostack.h>

namespace meteo {
namespace map {
class Object;
class MapScene;
} // map
} // meteo

namespace meteo {
namespace map {

class AddObjectCommand : public QUndoCommand
{
public:
  AddObjectCommand(const QString& layerUuid, Object* obj, MapScene* scene);
  virtual ~AddObjectCommand();

  virtual void undo();
  virtual void redo();

private:
  MapScene* scene_;
  Object* obj_;
  QString layerUuid_;
  QString objectUuid_;
};

} // map
} // meteo


#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_UNDOREDO_H
