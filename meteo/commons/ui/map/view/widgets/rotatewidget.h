#ifndef ROTATEWIDGET_H
#define ROTATEWIDGET_H

#include <QDialog>

namespace Ui {
  class RotateSettings;
}

namespace meteo {
namespace map {

class RotateWidget : public QDialog
{
  Q_OBJECT
public:
  explicit RotateWidget(QWidget *parent = 0);
  ~RotateWidget();

  void setStep(int step);
  int step();
  void setMeridian(int mer);
  int meridian();

  void init();

signals:
  void needSaveStep(int);
  void needSaveMeridian(int);

public slots:

protected:
  void keyPressEvent(QKeyEvent* e);
  void closeEvent(QCloseEvent* event );
private slots:
  void activateBtns();
  void deactivateBtns();
  void slotSave();
  void slotCancel();

private:
  bool canClose();
  int showSimpleQuestion(const QString& questionTitle, const QString& questionText);
private:
  Ui::RotateSettings* ui_;
  bool dontSave_;
  bool isChanged_;
};

}
}
#endif // ROTATEWIDGET_H
