#include "coordedit.h"

#include <commons/mathtools/mnmath.h>

#include <qevent.h>

#include <cross-commons/debug/tlog.h>

const QString CoordEdit::kDisplayFormat = QString::fromUtf8("%1° %2' %3\"");

const QList<QChar> CoordEdit::kSpecSymbols = QList<QChar>() << '\'' << '"' << QChar(0x00B0);

bool CoordData::checkFormat(const QString& s)
{
  if ( s.isEmpty() ) { return true; }


  QStringList patterns;
  if ( s.contains(QChar(0x00B0)) ) {
    patterns += QString::fromUtf8("\\s*(\\d+)?°\\s*");
    patterns += QString::fromUtf8("\\s*(\\d+)?°\\s*(\\d+)?'\\s*");
    patterns += QString::fromUtf8("\\s*(\\d+)?°\\s*(\\d+)?'\\s*(\\d+)?\"\\s*");
    patterns += QString::fromUtf8("\\s*\\d+°\\s*\\d+'\\s*\\d+\"\\s*");
  }
  else {
    patterns += QString::fromUtf8("\\s*\\d+(\\s{1}\\d+)?(\\s{1}\\d+\\s*)?");
  }

  QRegExp rx;
  foreach ( const QString& p, patterns ) {
    rx.setPattern(p);
    if ( rx.exactMatch(s) ) {
      return true;
    }
  }

  return false;
}

CoordData CoordData::fromString(const QString& s, double min, double max)
{
  if ( !CoordData::checkFormat(s) ) { return CoordData(); }

  CoordData d;
  d.setMinimum(min);
  d.setMaximum(max);
  d.dmsFromString(s);
  return d;
}

void CoordData::dmsFromString(const QString& s)
{
  QString ss = s.trimmed();

  QRegExp splitRx;

  if ( ss.contains(QChar(0x00B0)) ) {
    splitRx.setPattern(QString::fromUtf8("[°'\"]{1}"));
    ss.remove(QRegExp("\\s"));
  }
  else {
    splitRx.setPattern("\\s+");
  }

  QStringList list = ss.split(splitRx, QString::KeepEmptyParts);

  if ( list.size() >= 1 ) { setDecDegree(list.at(0).toInt()); }
  if ( list.size() >= 2 ) { setMinute(list.at(1).toInt()); }
  if ( list.size() >= 3 ) { setSecond(list.at(2).toInt()); }
}

double CoordData::toDecDegree() const
{
  const int k = 1000000;
  int64_t D = degree * k;
  int64_t M = minute * k;
  int64_t S = second * k;

  double dd = D + (M / 60.0) + (S / 3600.0);

  dd = dd / k;

  return dd;
}

void CoordData::setDecDegree(double dd)
{
  dd += .0001;

  MnMath::Angle a;
  MnMath::deg2deg60(dd, a);
  degree = a.degree;
  minute = a.minute;
  second = a.second;

  if ( !checkMinimum() ) { setDecDegree(min_); }
  if ( !checkMaximum() ) { setDecDegree(max_); }
}

void CoordData::addDegree(int d)
{
  degree += d;

  if ( !checkMinimum() ) { setDecDegree(min_); }
  if ( !checkMaximum() ) { setDecDegree(max_); }
}

void CoordData::addMinute(int m)
{
  minute += m;

  while ( minute >= 60 ) { minute -= 60; addDegree(1);  }
  while ( minute < 0   ) { minute += 60; addDegree(-1); }

  if ( !checkMinimum() ) { setDecDegree(min_); }
  if ( !checkMaximum() ) { setDecDegree(max_); }
}

void CoordData::addSecond(int s)
{
  second += s;

  while ( second >= 60 ) { second -= 60; addMinute(1);  }
  while ( second < 0   ) { second += 60; addMinute(-1); }

  if ( !checkMinimum() ) { setDecDegree(min_); }
  if ( !checkMaximum() ) { setDecDegree(max_); }
}

void CoordData::setDMS(int d, int m, int s)
{
  degree = d;
  minute = m;
  second = s;

  if ( !checkMinimum() ) { setDecDegree(min_); }
  if ( !checkMaximum() ) { setDecDegree(max_); }
}

void CoordData::setMinimum(double decDegree)
{
   min_ = decDegree;
   if ( !checkMinimum() ) { setDecDegree(min_); }
}

void CoordData::setMaximum(double decDegree)
{
   max_ = decDegree;
   if ( !checkMaximum() ) { setDecDegree(max_); }
}

bool CoordData::checkMinimum() const
{
  const int k = 1000000;
  int64_t d = toDecDegree() * k;
  int64_t m = min_ * k;

  return d >= m;
}

bool CoordData::checkMaximum() const
{
  const int k = 1000000;
  int64_t d = toDecDegree() * k;
  int64_t m = max_ * k;

  return d <= m;
}

//
//
//
//
//

CoordEdit::CoordEdit(QWidget *parent)
  : QLineEdit(parent)
{
  setDms(value_);

  const QFontMetrics fm(fontMetrics());
  QString s = kDisplayFormat.arg("360", "59", "59");
  int w = fm.width(s) + 20;
  int h = QLineEdit::sizeHint().height();
  setMinimumSize(QSize(w, h));

  connect( this, SIGNAL(textChanged(QString)), SLOT(slotTextChanged(QString)) );
}

CoordEdit::~CoordEdit()
{
}

CoordEdit::Section CoordEdit::currentSection() const
{
  int begDeg = 0;
  int endDeg = 0;
  getPositions(&begDeg, &endDeg, kDegreeSection);

  int begMin = 0;
  int endMin = 0;
  getPositions(&begMin, &endMin, kMinuteSection);

  int begSec = 0;
  int endSec = 0;
  getPositions(&begSec, &endSec, kSecondSection);

  int p = cursorPosition();

  if ( p >= begDeg && p <= endDeg ) { return kDegreeSection; }
  if ( p >= begMin && p <= endMin ) { return kMinuteSection; }
  if ( p >= begSec && p <= endSec ) { return kSecondSection; }

  return kNoSection;
}

void CoordEdit::setCurrentSection(CoordEdit::Section section)
{
  int begin = 0;
  int end = 0;
  getPositions(&begin, &end, section);

  setCursorPosition(end);
  setSelection(begin, end - begin);
}

double CoordEdit::decDegree() const
{
  return value_.toDecDegree();
}

void CoordEdit::setDecDegree(double dd)
{
  CoordData coord;
  coord.setDecDegree(dd);

  if ( coord == value_ ) { return; }

  setDms(coord);
}

bool CoordEdit::setDms(const CoordData& data)
{
  value_.setDegree(data.degree);
  value_.setMinute(data.minute);
  value_.setSecond(data.second);

  updateEditor();

  return true;
}

void CoordEdit::slotTextChanged(const QString& txt)
{
  if ( CoordData::checkFormat(txt) ) {
    value_.dmsFromString(txt);
  }

  updateEditor();
}

void CoordEdit::keyPressEvent(QKeyEvent* event)
{
  QList<int> numKeys = QList<int>()
    << Qt::Key_0
    << Qt::Key_1
    << Qt::Key_2
    << Qt::Key_3
    << Qt::Key_4
    << Qt::Key_5
    << Qt::Key_6
    << Qt::Key_7
    << Qt::Key_8
    << Qt::Key_9
       ;
  QList<int> rmKeys = QList<int>()
    << Qt::Key_Delete
    << Qt::Key_Backspace
       ;
  QList<int> navKeys = QList<int>()
    << Qt::Key_Home
    << Qt::Key_End
    << Qt::Key_Left
    << Qt::Key_Right
    << Qt::Key_Enter
    << Qt::Key_Return
       ;
  QList<int> acceptedKeys = numKeys + navKeys + rmKeys;

  QList<QKeySequence::StandardKey> acceptedSeq = QList<QKeySequence::StandardKey>()
    << QKeySequence::Copy
    << QKeySequence::Cut
    << QKeySequence::Paste
    << QKeySequence::SelectAll
  ;

  // проверяем выход за MIN/MAX
  if ( numKeys.contains(event->key()) ) {
    QString s = text();

    QString selText = selectedText();
    if ( !selText.isEmpty() ) {
      s.remove(selectionStart(), selText.size());
    }
    s.insert(cursorPosition(), getChar(event));
    CoordData tmp = CoordData::fromString(s);

    double dd = tmp.toDecDegree();
    if ( dd < minimum() || (dd > maximum() && !qFuzzyCompare(dd, maximum())) ) {
      return;
    }
    if ( tmp.minute < 0 || tmp.minute > 59 ) { return; }
    if ( tmp.second < 0 || tmp.second > 59 ) { return; }
  }

  // запрещаем удалять спеиальные символы
  if ( rmKeys.contains(event->key()) ) {
    QString s = text();
    int p = cursorPosition();
    if ( event->key() == Qt::Key_Backspace && 0 != p ) { p -= 1; }

    if ( p < s.size() &&  CoordEdit::kSpecSymbols.contains(s.at(p)) ) {
      return;
    }
  }

  if ( Qt::Key_Up == event->key() || Qt::Key_Down == event->key() ) {
    int d = event->key() == Qt::Key_Up ? 1 : -1;

    CoordData tmp = value_;

    switch ( currentSection() ) {
      case kDegreeSection: { tmp.addDegree(d); } break;
      case kMinuteSection: { tmp.addMinute(d); } break;
      case kSecondSection: { tmp.addSecond(d); } break;
      case kNoSection: {
        return;
      }
    }

    setDms(tmp);
  }

  if ( Qt::Key_Tab == event->key() ) {
    switch ( currentSection() ) {
      case kDegreeSection: { setCurrentSection(kMinuteSection); } break;
      case kMinuteSection: { setCurrentSection(kSecondSection); } break;
      case kSecondSection: {  } break;
      case kNoSection: {
        return;
      }
    }
  }
  if ( Qt::Key_Backtab == event->key() ) {
    switch ( currentSection() ) {
      case kDegreeSection: {  } break;
      case kMinuteSection: { setCurrentSection(kDegreeSection); } break;
      case kSecondSection: { setCurrentSection(kMinuteSection); } break;
      case kNoSection: {
        return;
      }
    }
  }

  if ( acceptedKeys.contains(event->key()) ) {
    bool hasSel = hasSelectedText();

    QLineEdit::keyPressEvent(event);

    if ( hasSel && !navKeys.contains(event->key()) ) { cursorToEndOfSection(); }
  }
  foreach ( QKeySequence::StandardKey seq, acceptedSeq ) {
    if ( event->matches(seq) ) {
      QLineEdit::keyPressEvent(event);
      break;
    }
  }
}

void CoordEdit::wheelEvent(QWheelEvent* event)
{
  const int steps = (event->delta() > 0 ? 1 : -1);

  int d = event->modifiers() & Qt::ControlModifier ? steps * 10 : steps;

  CoordData tmp = value_;

  switch ( currentSection() ) {
    case kDegreeSection: { tmp.addDegree(d); } break;
    case kMinuteSection: { tmp.addMinute(d); } break;
    case kSecondSection: { tmp.addSecond(d); } break;
    case kNoSection: {
      return;
    }
  }

  setDms(tmp);

  QLineEdit::wheelEvent(event);
}

void CoordEdit::focusInEvent(QFocusEvent* event)
{
  QLineEdit::focusInEvent(event);

  switch (event->reason()) {
    case Qt::BacktabFocusReason: { setCurrentSection(kSecondSection); } break;
    case Qt::TabFocusReason:     { setCurrentSection(kDegreeSection); } break;
    default: {} break;
  }
}

bool CoordEdit::focusNextPrevChild(bool next)
{
  if ( nextPrevSection(next) == kNoSection ) {
    return QLineEdit::focusNextPrevChild(next);
  }

  return false;
}

void CoordEdit::updateEditor()
{
  int old = cursorPosition();
  setText(dms());
  setCursorPosition(old);
}

QChar CoordEdit::getChar(QKeyEvent* event) const
{
  if ( 0 == event ) { return QChar(); }

  QMap<int,QChar> map;
  map.insert(Qt::Key_0, QChar('0'));
  map.insert(Qt::Key_1, QChar('1'));
  map.insert(Qt::Key_2, QChar('2'));
  map.insert(Qt::Key_3, QChar('3'));
  map.insert(Qt::Key_4, QChar('4'));
  map.insert(Qt::Key_5, QChar('5'));
  map.insert(Qt::Key_6, QChar('6'));
  map.insert(Qt::Key_7, QChar('7'));
  map.insert(Qt::Key_8, QChar('8'));
  map.insert(Qt::Key_9, QChar('9'));

  return map.value(event->key(), QChar());
}

void CoordEdit::getPositions(int* begin, int* end, CoordEdit::Section section) const
{
  if ( 0 == begin ) { return; }
  if ( 0 == end   ) { return; }

  QString s = text();

  int begDeg = 0;
  int endDeg = s.indexOf(QChar(0x00B0));

  int begMin = endDeg + 1;
  int endMin = s.indexOf(QChar('\''));

  int begSec = endMin + 1;
  int endSec = s.indexOf(QChar('"'));

  switch ( section ) {
    case kDegreeSection: { *begin = begDeg; *end = endDeg; } break;
    case kMinuteSection: { *begin = begMin; *end = endMin; } break;
    case kSecondSection: { *begin = begSec; *end = endSec; } break;
    case kNoSection: {} break;
  }
}

CoordEdit::Section CoordEdit::nextPrevSection(bool next) const
{
  Section section = currentSection();

  if ( kNoSection == section ) { return kNoSection; }

  if ( next && kSecondSection == section  ) { return kNoSection; }
  if ( !next && kDegreeSection == section ) { return kNoSection; }

  return static_cast<Section>(section + (next ? 1 : -1));
}

void CoordEdit::cursorToEndOfSection()
{
  CoordEdit::Section s = currentSection();

  int b = 0;
  int e = 0;
  getPositions(&b, &e, s);
  setCursorPosition(e);
}


