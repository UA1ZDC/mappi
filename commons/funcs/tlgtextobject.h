#ifndef METEO_COMMONS_UI_CUSTOM_TLGTEXTOBJECT_H
#define METEO_COMMONS_UI_CUSTOM_TLGTEXTOBJECT_H

#include <commons/container/thash.h>

#include <qpainter.h>
#include <qabstracttextdocumentlayout.h>

enum { TlgTextFormat = QTextFormat::UserObject + 1 };

//!
class TlgTextObject : public QObject, public QTextObjectInterface
{
  Q_OBJECT
  Q_INTERFACES(QTextObjectInterface)

public:
  static const THash<QString,QChar> kSpecChars;

  static bool isSpecChar(const QChar& c);

  enum TlgProperties { Symbol = 1, Value = 2, Font = 3 };

  virtual QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
  virtual void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDoc, const QTextFormat& f);
};

#endif // METEO_COMMONS_UI_CUSTOM_TLGTEXTOBJECT_H
