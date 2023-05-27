#include "rotatewidget.h"
#include "ui_rotatesettings.h"

#include <qevent.h>
#include <qdialog.h>
#include <qmessagebox.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

RotateWidget::RotateWidget(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::RotateSettings),
  dontSave_(false),
  isChanged_(false)
{
  ui_->setupUi(this);
  ui_->saveButton->setEnabled(false);
  ui_->cancelButton->setEnabled(false);
}

void RotateWidget::init()
{
  connect(ui_->lonSpin,SIGNAL(valueChanged(int)),this,SLOT(activateBtns()));
  connect(ui_->curStep,SIGNAL(valueChanged(int)),this,SLOT(activateBtns()));
  connect(ui_->saveButton, SIGNAL(clicked()), this, SLOT(slotSave()));
  connect(ui_->closeButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui_->cancelButton, SIGNAL(clicked()), this, SLOT(slotCancel()));
}

RotateWidget::~RotateWidget()
{
  delete ui_; ui_ = 0;
}

void RotateWidget::setStep(int step)
{
  ui_->curStep->setValue(step);
}

int RotateWidget::step()
{
  return ui_->curStep->value();
}

void RotateWidget::setMeridian(int mer)
{
  ui_->lonSpin->setValue(mer);
}

int RotateWidget::meridian()
{
  return ui_->lonSpin->value();
}

void RotateWidget::activateBtns()
{
  ui_->saveButton->setEnabled(true);
  ui_->cancelButton->setEnabled(true);
  isChanged_ = true;
}

void RotateWidget::deactivateBtns()
{
  ui_->saveButton->setEnabled(false);
  ui_->cancelButton->setEnabled(false);
  isChanged_ = false;
}

void RotateWidget::closeEvent(QCloseEvent* event )
{
  if (false == canClose())
  {
    event->ignore();
    return;
  }
  event->accept();
}

void RotateWidget::keyPressEvent(QKeyEvent* e)
{
  if( e->key() == Qt::Key_Escape ) {
    e->accept();
    close();
    return;
  }
  else if ((e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return ))
  {
    e->accept();
    slotSave();
    return;
  }
  e->accept();
}

void RotateWidget::slotSave()
{
  if ( false == isChanged_ )
  {
    return;
  }
  else
  {
    emit needSaveStep(step());
    emit needSaveMeridian(meridian());
    deactivateBtns();
  }
  return;
}

void RotateWidget::slotCancel()
{
  dontSave_ = true;
  close();
}

bool RotateWidget::canClose()
{
  if (true == dontSave_)
  {
    return true;
  }
  if (true == isChanged_)
  {
    int answer = showSimpleQuestion(tr("Настройки изменены"),tr("Вы хотите принять изменения?"));
    if( 0  == answer )
    {
      emit needSaveStep(step());
      emit needSaveMeridian(meridian());
    }
  }
  return true;
}

int RotateWidget::showSimpleQuestion(const QString& questionTitle, const QString& questionText)
{
  return QMessageBox::question( this,
                                questionTitle,
                                questionText,
                                tr("Принять"),
                                tr("Отмена"));
}

}
}
