#include "tlgtextobject.h"

const THash<QString,QChar> TlgTextObject::kSpecChars = THash<QString,QChar>()
  << qMakePair(QString("<SOH>"), QChar(001))
  << qMakePair(QString("<STX>"), QChar(002))
  << qMakePair(QString("<ETX>"), QChar(003))
  << qMakePair(QString("<CR>"), QChar(015))
  << qMakePair(QString("<LF>"), QChar(012))
  << qMakePair(QString("<SO>"), QChar(016))
  << qMakePair(QString("<SI>"), QChar(017))
;


bool TlgTextObject::isSpecChar(const QChar& c)
{
  return TlgTextObject::kSpecChars.values().contains(c);
}

QSizeF TlgTextObject::intrinsicSize(QTextDocument* /*doc*/, int /*posInDocument*/, const QTextFormat& format)
{
  QString v = format.property(Symbol).toString();

  QFontMetrics metric(format.property(Font).value<QFont>());
  return metric.size(Qt::TextSingleLine, v);
}

void TlgTextObject::drawObject(QPainter* painter, const QRectF& rect, QTextDocument* doc,
                               int posInDoc, const QTextFormat& f)
{
  Q_UNUSED( doc );
  Q_UNUSED( posInDoc );

  QString v = f.property(Symbol).toString();
  painter->drawText(rect, v);
}
