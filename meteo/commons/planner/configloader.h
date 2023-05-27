#ifndef METEO_COMMONS_PLANNER_CONFIGLOADER_H
#define METEO_COMMONS_PLANNER_CONFIGLOADER_H

#include <qobject.h>

class QFileSystemWatcher;

namespace google {
namespace protobuf {
class Message;
} // protobuf
} // google

namespace meteo {

class ConfigLoader : public QObject
{
  Q_OBJECT
public:
  //!
  ConfigLoader(const QString& path, bool autoLoad = true, QObject *parent = 0);

public slots:
  virtual bool slotLoad(const QString& path = QString()) { Q_UNUSED( path ); return false; }

protected:
  bool loadConfigPart(::google::protobuf::Message* part, const QString& filename) const;

private:
  // служебные
  QFileSystemWatcher* watcher_;
};

} // meteo

#endif // METEO_COMMONS_PLANNER_CONFIGLOADER_H
