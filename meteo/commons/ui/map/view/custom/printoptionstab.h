#ifndef PRINTOPTIONSTAB_H
#define PRINTOPTIONSTAB_H

#include <QWidget>
#include <QButtonGroup>

namespace Ui {
  class PrintOptionsTab;
}

class PrintOptionsTab : public QWidget
{
  Q_OBJECT
public:
  enum ScaleMode {
    NoScale,
    ScaleToPage,
    ScaleToCustomSize
  };

  enum Unit {
    Millimeters,
    Centimeters,
    Inches
  };

  PrintOptionsTab(const QSize& imageSize);
  ~PrintOptionsTab();

  Qt::Alignment alignment() const;
  ScaleMode scaleMode() const;
  bool enlargeSmallerImages() const;
  Unit scaleUnit() const;
  double scaleWidth() const;
  double scaleHeight() const;

private slots:
  void adjustWidthToRatio();
  void adjustHeightToRatio();

private:
  Ui::PrintOptionsTab* ui_;
  QSize imageSize_;
  QButtonGroup scaleGroup_;
  QButtonGroup positionGroup_;
};

#endif /* PRINTOPTIONSTAB_H */
