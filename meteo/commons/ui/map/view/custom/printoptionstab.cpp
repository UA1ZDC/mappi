#include "printoptionstab.h"
#include <QButtonGroup>
#include <QGridLayout>
#include <QToolButton>
#include <QDebug>
#include <ui_printoptionstab.h>

static inline double unitToInches(PrintOptionsTab::Unit unit)
{
  if (unit == PrintOptionsTab::Inches) {
    return 1.;
  }
  else if (unit == PrintOptionsTab::Centimeters) {
    return 1 / 2.54;
  }
  else {
    return 1 / 25.4;
  }
}

PrintOptionsTab::PrintOptionsTab(const QSize& imageSize)
  : QWidget(),
    ui_(new Ui::PrintOptionsTab)
{
  ui_->setupUi(this);
  imageSize_ = imageSize;
  ui_->positionFrame->setStyleSheet(
        "QFrame {"
        "	background-color: palette(mid);"
        "	border: 1px solid palette(dark);"
        "}"
        "QToolButton {"
        "	border: none;"
        "	background: palette(base);"
        "}"
        "QToolButton:hover {"
        "	background: palette(alternate-base);"
        "	border: 1px solid palette(highlight);"
        "}"
        "QToolButton:checked {"
        "	background-color: palette(highlight);"
        "}"
        );
  QGridLayout* layout = new QGridLayout(ui_->positionFrame);
  layout->setMargin(0);
  layout->setSpacing(1);
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      QToolButton* button = new QToolButton(ui_->positionFrame);
      button->setFixedSize(40, 40);
      button->setCheckable(true);
      layout->addWidget(button, row, col);
      if( row == 1 && col == 1){
        button->setChecked(true);
      }
      Qt::Alignment alignment;
      if (row == 0) {
        alignment = Qt::AlignTop;
      }
      else if (row == 1) {
        alignment = Qt::AlignVCenter;
      }
      else {
        alignment = Qt::AlignBottom;
      }
      if (col == 0) {
        alignment |= Qt::AlignLeft;
      }
      else if (col == 1) {
        alignment |= Qt::AlignHCenter;
      }
      else {
        alignment |= Qt::AlignRight;
      }
      positionGroup_.addButton(button, int(alignment));
    }
  }

  scaleGroup_.addButton(ui_->mNoScale, NoScale);
  scaleGroup_.addButton(ui_->mScaleToPage, ScaleToPage);
  scaleGroup_.addButton(ui_->mScaleTo, ScaleToCustomSize);

  ui_->mScaleToPage->setChecked(true);

  connect(ui_->printWidth, SIGNAL(valueChanged(double)), SLOT(adjustHeightToRatio()));
  connect(ui_->printHeight, SIGNAL(valueChanged(double)),SLOT(adjustWidthToRatio()));
  connect(ui_->printKeepRatio, SIGNAL(toggled(bool)),SLOT(adjustHeightToRatio()));
}

PrintOptionsTab::~PrintOptionsTab()
{
  delete ui_; ui_ = 0;
}

Qt::Alignment PrintOptionsTab::alignment() const
{
  int id = positionGroup_.checkedId();
  return Qt::Alignment(id);
}

PrintOptionsTab::ScaleMode PrintOptionsTab::scaleMode() const
{
  return PrintOptionsTab::ScaleMode(scaleGroup_.checkedId());
}

bool PrintOptionsTab::enlargeSmallerImages() const
{
  return ui_->enlargeSmallerImages->isChecked();
}

PrintOptionsTab::Unit PrintOptionsTab::scaleUnit() const
{
  return PrintOptionsTab::Unit(ui_->printUnit->currentIndex());
}

double PrintOptionsTab::scaleWidth() const
{
  return ui_->printWidth->value() * unitToInches(scaleUnit());
}

double PrintOptionsTab::scaleHeight() const
{
  return ui_->printHeight->value() * unitToInches(scaleUnit());
}

void PrintOptionsTab::adjustWidthToRatio()
{
  if (!ui_->printKeepRatio->isChecked()) {
    return;
  }
  double width = imageSize_.width() * ui_->printHeight->value() / imageSize_.height();
  ui_->printWidth->blockSignals(true);
  ui_->printWidth->setValue(width ? width : 1.);
  ui_->printWidth->blockSignals(false);
}

void PrintOptionsTab::adjustHeightToRatio()
{
  if (!ui_->printKeepRatio->isChecked()) {
    return;
  }
  double height = imageSize_.height() * ui_->printWidth->value() / imageSize_.width();
  ui_->printHeight->blockSignals(true);
  ui_->printHeight->setValue(height ? height : 1.);
  ui_->printHeight->blockSignals(false);
}
