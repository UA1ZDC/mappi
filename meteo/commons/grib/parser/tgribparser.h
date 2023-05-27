#ifndef TGRIB_PARSER_H
#define TGRIB_PARSER_H

#include <stdint.h>

#include <qstring.h>

namespace google {
  namespace protobuf {
    class Reflection;
    class Message;
  }
}

class QDomNode;
class QDomElement;

//!парсинг grib данных, считанных из БД и описанных в xml и proto-файлах
class TGribParser {
public:
  TGribParser();
  virtual ~TGribParser();

  void setup(const uint8_t* data, uint64_t size, const QString& file, short version, uint16_t templ);
  bool parse();
  bool createFromString(uint16_t templ, int version, const std::string& src);

protected:
  virtual google::protobuf::Message* createDefinition(uint16_t templ, int version) = 0;

  bool parseTemplate();
  virtual void parseCustom(google::protobuf::Message* msg, const uint8_t* data, uint32_t dataSize, QDomElement& node, int32_t startIdx);
  float calcValueFactor(const uint8_t* data, const QDomElement& el, int32_t startIdx);

  QDomNode setContent(const QString& fileName, uint16_t templ);

private:
  bool parse(QDomNode&);
  void parseMessage(QDomNode& node, google::protobuf::Message * sub, int startIdx);
  void fillRepeated(google::protobuf::Message* msg, QDomElement& node, const google::protobuf::Reflection * refl, int startIdx);

  QDomNode findXmlTemplate(const QDomElement& el, const QString& attrVal);

private:
  const uint8_t* _data;
  uint32_t _size;
  google::protobuf::Message* _msg;
  uint16_t _templ;
  short _version;
  QString _file;
};

#endif
