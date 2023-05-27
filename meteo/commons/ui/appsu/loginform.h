#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <qdialog.h>

namespace Ui
{
  class LoginForm;
}

class LoginForm: public QDialog
{
  friend class TAppSu;

  Q_OBJECT

public:

  explicit LoginForm(QWidget *p, QString user);
  ~LoginForm();
signals:
  void signalEnter();
  void signalClose();
protected:
  void keyReleaseEvent( QKeyEvent* e );
private slots:
  void slotEnter();
  void slotClose();

private:
  Ui::LoginForm* ui_;


};



#endif // LOGINFORM_H
