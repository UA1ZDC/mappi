#ifndef TXMLPROTO_H
#define TXMLPROTO_H


#include <QDomDocument>

namespace google {
namespace protobuf {

class Message;

} // protobuf
} // google

//! \note При разборе и генерации xml, для поиска полей используются имена
//! в нижнем регистре, поэтому для прото-сообщений, поля которых, различаются
//! только регистром, функции могут работать некорректно.
class TXmlProto
{
public:
  TXmlProto(){}

  //! Генерирует DOM-дерево по прото-сообщению
  //! Если fieldName не задано, то используется имя типа сообщения.
  static QDomDocument toXml(const google::protobuf::Message& msg,
                            const QString& tagName = QString());
  //! Заполняет прото-сообщение, считывая информацию из DOM-дерева
  static bool fillProto(google::protobuf::Message* msg, const QDomElement& root);

  //! Возвращает значение поля с именем fieldName в сообщении msg
  //! в виде строки.
  //! Если поле не найдено, тип поля MESSAGE или это repeated-поле, то
  //! вернётся QString().
  static QString getFieldValue(const google::protobuf::Message& msg,
                               const QString& fieldName);
  //! Возвращает список значений для repeated-полей.
  //! Если поле не найдено или тип поля MESSAGE, то возвращается пустой список.
  //! FIXME: существует неоднозначность, т.к. пустой список будет возвращем и
  //!        для поля типа MESSAGE, и для поля не содержащего значения.
  static QStringList getFieldValues(const google::protobuf::Message& msg,
                                    const QString& fieldName);
  //! Устанавливает значение поля fieldName для сообщения msg.
  //! Возвращает false если поле не найдено, тип поля MESSAGE
  static bool setValue(google::protobuf::Message* msg,
                       const QString& fieldName,
                       const QStringList& values);
};


#endif // TXMLPROTO_H
