#ifndef METEO_COMMONS_UI_MAP_CUSTOM_EVENT_H
#define METEO_COMMONS_UI_MAP_CUSTOM_EVENT_H

#include <QEvent>

namespace google
{
namespace protobuf
{
class Message;
} // protobuf
} // google

namespace meteo
{
namespace map
{
class CustomEvent : public QEvent
{
public:
  static const QEvent::Type Custom;
  enum sub_t {
    SelectGeoPointsByField,
    SelectGeoPointsBySurface,
    ClearGeoPoints
  };

  explicit CustomEvent(sub_t type);
  ~CustomEvent();

  sub_t customType() const;

  void setOwnedMessage(google::protobuf::Message* message);
  google::protobuf::Message* ownedMessage();

private:
  sub_t type_;
  google::protobuf::Message* message_;

};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_CUSTOM_EVENT_H
