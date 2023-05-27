#ifndef METEO_COMMONS_UI_CUSTOM_COORDEDIT_H
#define METEO_COMMONS_UI_CUSTOM_COORDEDIT_H

#include <qlineedit.h>
#include <meteo/commons/ui/map/view/mapview.h>

//!
class CoordData
{
public:
  int degree;
  int minute;
  int second;

  static bool checkFormat(const QString& s);
  static CoordData fromString(const QString& s, double min = 0, double max = 360);

  CoordData(int d = 0, int m = 0, int s = 0) : degree(d), minute(m), second(s), min_(0), max_(360) {}

  void dmsFromString(const QString& s);

  double toDecDegree() const;
  void setDecDegree(double dd);

  void addDegree(int d);
  void addMinute(int m);
  void addSecond(int s);

  void setDegree(int d) { setDMS(d, minute, second); }
  void setMinute(int m) { setDMS(degree, m, second); }
  void setSecond(int s) { setDMS(degree, minute, s); }
  void setDMS(int d, int m, int s);

  double minimum() const { return min_; }
  void setMinimum(double decDegree);

  double maximum() const { return max_; }
  void setMaximum(double decDegree);

  bool checkMinimum() const;
  bool checkMaximum() const;

private:
  double min_;
  double max_;
};

inline bool operator==(const CoordData& d1, const CoordData& d2)
{ return d1.degree == d2.degree && d1.minute == d2.minute && d1.second == d2.second; }



//!
class CoordEdit : public QLineEdit
{
  Q_OBJECT

  Q_ENUMS(Section)

  Q_PROPERTY(Section currentSection READ currentSection WRITE setCurrentSection)
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)

  static const QString kDisplayFormat;
  static const QList<QChar> kSpecSymbols;

public:
  enum Section {
    kNoSection = -1,
    kDegreeSection = 1,
    kMinuteSection = 2,
    kSecondSection = 3,
  };

  explicit CoordEdit(QWidget *parent = 0);
  virtual ~CoordEdit();

  Section currentSection() const;
  void setCurrentSection(Section section);

  double decDegree() const;
  void setDecDegree(double dd);

  QString dms() const { return kDisplayFormat.arg(value_.degree).arg(value_.minute).arg(value_.second); }
  bool setDms(const CoordData& dms);

  double minimum() const { return value_.minimum(); }
  void setMinimum(double decDegree) { value_.setMinimum(decDegree); }

  double maximum() const { return value_.maximum(); }
  void setMaximum(double decDegree) { value_.setMaximum(decDegree); }

public slots:
  void slotTextChanged(const QString& text);

protected:
  virtual void keyPressEvent(QKeyEvent *event);
  virtual void wheelEvent(QWheelEvent *event);
  virtual void focusInEvent(QFocusEvent *event);
  virtual bool focusNextPrevChild(bool next);

  void updateEditor();

private:
  QChar getChar(QKeyEvent* event) const;
  void getPositions(int* begin, int* end, Section section) const;
  Section nextPrevSection(bool next) const;
  void cursorToEndOfSection();

private:
  CoordData value_;
};

#endif // METEO_COMMONS_UI_CUSTOM_COORDEDIT_H
