#ifndef PRERUN_H
#define PRERUN_H

#include <QDialog>

namespace Ui {
class preRun;
}

class preRun : public QDialog
{
  Q_OBJECT

public:
  explicit preRun(const QString &jobname, QWidget *parent = nullptr);
  ~preRun();



private slots:
  void slotClose();

private:

  void initOptions();


  Ui::preRun *ui;
  QString jobname_;
};

#endif // PRERUN_H
