#include "loginform.h"
#include "ui_loginform.h"

#include <QKeyEvent>
#include <QMessageBox>

namespace {
  const QRegExp kPasswRegExp("([a-zA-Z0-9\\~\\!\\@\\#\\$\\%\\^\\&\\*\\(\\)\\_\\+\\-\\=\\<\\>\\,\\.\\?\\:\\;\\{\\}\\[\\]]+)");
}

LoginForm::LoginForm(QWidget* p, QString user)
  : QDialog(p),
    ui_( new Ui::LoginForm )
{
  ui_->setupUi(this);
  QObject::connect( ui_->okButton, SIGNAL( released() ), this, SLOT( slotEnter() ) );
  QObject::connect( ui_->cancelButton, SIGNAL( released() ), this, SLOT( slotClose() ) );
  ui_->loginEdit->setText(user);
  ui_->passwordEdit->setPlaceholderText("[Введите пароль]");
  setTabOrder(ui_->loginEdit,ui_->passwordEdit);
  setTabOrder(ui_->passwordEdit,ui_->okButton);
  setTabOrder(ui_->okButton,ui_->cancelButton);
  setTabOrder(ui_->cancelButton,ui_->loginEdit);
  setObjectName("loginform");
  //setFocusPolicy(Qt::StrongFocus);
}

LoginForm::~LoginForm()
{
  delete ui_; ui_ = 0;
}

void LoginForm::slotEnter()
{
  QString login = ui_->loginEdit->text();
  QString password = ui_->passwordEdit->text();
  if ((login == "") || (password == ""))
  {
    QMessageBox::warning(this,tr("Ошибка"),tr("Заполните все поля для авторизации"), tr("Закрыть"));
    return;
  }
  if (false == kPasswRegExp.exactMatch(password))
  {
    QMessageBox::warning(this,tr("Ошибка"),tr("Введен некорректный пароль"), tr("Закрыть"));
    return;
  }
  emit signalEnter();
}

void LoginForm::keyReleaseEvent(QKeyEvent* e)
{
  if( e->key() == Qt::Key_Escape ) {
    e->accept();
    slotClose();
    return;
  }
  else
    if(( e->key() == Qt::Key_Return ) || (e->key() == Qt::Key_Enter)){
      e->accept();
      slotEnter();
      return;
  }
  e->accept();
}


void LoginForm::slotClose()
{
  emit signalClose();
  close();
}
