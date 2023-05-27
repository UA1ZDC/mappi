#ifndef TAPPSU_H
#define TAPPSU_H

#include <qobject.h>
#include <QSettings>

const QString ADMINLOGIN = "root";

const uid_t kMinUid = 0;
const uid_t kMaxUid = 9999;

class LoginForm;

class TAppSu : public QObject
{
  Q_OBJECT

public:

  explicit TAppSu(QString name, QString user = ADMINLOGIN);
  ~TAppSu();
  void init();
//protected:
//  void keyPressEvent( QKeyEvent* e );
private slots:
  void slotEnterPasswdEndExec();
  void slotClose();
private:
  bool isExistsUser();
  bool isExistsFile();
  bool isExistsEnv();
  bool isExistsLibPath();
  void saveGeometryAndState();
  void restoreGeometryAndState();
  bool eventFilter(QObject*, QEvent *event);
private:
  LoginForm* ui_;
  QString appName_;
  QString userName_;
  QSettings* settings_;
};

#endif // TAPPSU_H
