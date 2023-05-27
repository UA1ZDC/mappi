#include "appsu.h"
#include <loginform.h>
#include <ui_loginform.h>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/paths.h>

#include <stdlib.h>
#include <unistd.h>
#include <pty.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>

#include <qapplication.h>
#include <qprocess.h>
#include <qstringlist.h>
#include <qdebug.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qbytearray.h>
#include <qsocketnotifier.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qdesktopwidget.h>
#include <meteo/commons/settings/tadminsettings.h>

#include <cross-commons/debug/tlog.h>

namespace {
  pid_t pid;
  int master;
  int status;
  QString lastMsg;


  void help();
  void signalHandler(int signum);
  void showMsg(const QString& msg);
  void writeErr(const QString& err);

  void showMsg(const QString& msg)
  {
    std::cout << msg.toUtf8().data() << std::endl << std::flush;
    return;
  }

  void writeErr(const QString& err)
  {
    std::cerr << err.toUtf8().data() << std::endl;
    return;
  }

  void help()
  {
    showMsg("Параметры:");
    showMsg(" --path=ПУТЬ                 полный путь к запускаемому приложению");
    showMsg(" --user=ИМЯ ПОЛЬЗОВАТЕЛЯ     имя пользователя, от которого следует выполнить запуск (по умолчанию root)");
    showMsg("Сокращенный формат:");
    showMsg(" -p ПУТЬ                     полный путь к запускаемому приложению");
    showMsg(" -u ИМЯ ПОЛЬЗОВАТЕЛЯ         имя пользователя, от которого следует выполнить запуск (по умолчанию root)");
  }

  void signalHandler(int signum)
  {
    switch (signum)
    {
      case SIGCHLD:
      {
        pid_t wp = waitpid(pid, &status, 0);
        int exitStatus = EXIT_SUCCESS;
        if (wp == pid)
        {

          if (WIFEXITED(status))
          {
            if (1 == WEXITSTATUS(status)){
              writeErr(QObject::tr("Невозможно запустить программу. ") + lastMsg);
              exitStatus = EXIT_FAILURE;
            }
            else if (0 == WEXITSTATUS(status))
            {
              //showMsg("Работа приложения завершена");
            }
          }
        }
        else if (-1 == wp)
        {
          writeErr(QObject::tr("Работа приложения завершена с ошибками"));
          exitStatus = EXIT_FAILURE;
        }
        else
        {
          //showMsg(QObject::tr("Работа приложения завершена"));
        }
        ::exit(exitStatus);
      }
      break;
      default:
      {
        writeErr(QObject::tr("Работа приложения завершена некорректно"));
        if (pid > 0)
        {
          ::kill(pid,signum);
        }
        else
        {
          ::exit(EXIT_FAILURE);
        }
      }
      break;
    }
  }
}

TAppSu::TAppSu(QString name, QString user)
  : QObject(),
    ui_( new LoginForm(0, user) ),
    appName_(name),
    userName_(user),
    settings_(0)
{
  this->setObjectName("appsu");
  lastMsg = "";
  QObject::connect( ui_, SIGNAL( signalEnter() ), this, SLOT( slotEnterPasswdEndExec() ) );
  QObject::connect( ui_, SIGNAL( signalClose() ), this, SLOT( slotClose() ) );
}

void TAppSu::init()
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if ( false == env.contains("HOME") ) {
    settings_ = new QSettings(this->objectName() + ".ini", QSettings::IniFormat);
  }
  else {
    settings_ = new QSettings(env.value("HOME") + "/.meteo/" + this->objectName() + ".ini", QSettings::IniFormat);
  }
  restoreGeometryAndState();
  ui_->show();
  ui_->installEventFilter(this);
}

void TAppSu::saveGeometryAndState()
{
  settings_->setValue(ui_->objectName()+".size",ui_->size());
  settings_->setValue(ui_->objectName()+".pos",ui_->pos());
}

void TAppSu::restoreGeometryAndState()
{
  if (0 != settings_)
  {
    if (false == settings_->value(ui_->objectName()+".size").isNull())
    {
      ui_->resize(settings_->value(ui_->objectName()+".size").toSize());
    }
    else
    {
      ui_->resize(300,100);
    }
    if (false == settings_->value(ui_->objectName()+".pos").isNull())
    {
      ui_->move(settings_->value(ui_->objectName()+".pos").toPoint());
    }
    else
    {
      QRect scr = QApplication::desktop()->screenGeometry();
      ui_->move(scr.center() - ui_->rect().center());
    }
  }
  else
  {
    QRect scr = QApplication::desktop()->screenGeometry();
    ui_->move(scr.center() - ui_->rect().center());
  }
}

bool TAppSu::eventFilter(QObject* obj, QEvent* e)
{
  switch (e->type())
  {
    case QEvent::Resize:
    {
      saveGeometryAndState();
      break;
    }
  case QEvent::Move:
    {
      saveGeometryAndState();
      break;
    }
    default:
      qt_noop();
  }
  return QObject::eventFilter(obj, e);
}

TAppSu::~TAppSu()
{
  delete ui_;
  ui_ = 0;
  delete settings_;
  settings_ = 0;
}

void TAppSu::slotClose()
{
  ::exit(EXIT_SUCCESS);
}

bool TAppSu::isExistsUser()
{
 struct passwd* pw;
 for (uid_t i = kMinUid; i < kMaxUid; i++)
 {
    pw = getpwuid (i);
    if (pw != NULL)
    {
        char* name = pw->pw_name;
        QString user_login = QString::fromUtf8(name);
        if (user_login == userName_)
        {
          return true;
        }
    }
 }
 return false;
}

bool TAppSu::isExistsFile()
{

  QFileInfo file(appName_);
  if (!file.exists()) { return false;}
  if (!file.isFile()) { return false;}
  if (!file.isExecutable()) { return false;}
  return true;
}

bool TAppSu::isExistsEnv()
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if (false == env.contains("HOME")) { return false; }
  QFileInfo file( env.value("HOME") + "/.Xauthority" );
  if ( true == file.exists()) { return true; }
  return false;
}

bool TAppSu::isExistsLibPath()
{
  return QDir( MnCommon::libPath() ).exists();
}

void TAppSu::slotEnterPasswdEndExec()
{
  ui_->hide();
  if (appName_ == "")
  {
    writeErr(tr("Неверно заданы параметры запуска. Приложение будет закрыто"));
    ::exit(EXIT_FAILURE);
  }
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

  QString homedir = env.value("HOME");
  if( true != homedir.isEmpty() ){
    appName_.replace("~/", homedir);
  }
  else
  {
    writeErr(tr("Не установлена переменная окружения HOME. Приложение будет закрыто."));
    ::exit(EXIT_FAILURE);
  }

  if (isExistsFile() == false)
  {
    writeErr(tr("Неверно указан путь к файлу %1. Приложение будет закрыто.").arg(appName_));
    ::exit(EXIT_FAILURE);
  }
  QString login = ui_->ui_->loginEdit->text();
  QString password = ui_->ui_->passwordEdit->text();
  userName_ = login;
  ui_->close();
  if (isExistsUser() == false)
  {
    writeErr(tr("Пользователь %1 не существует. Приложение будет закрыто.").arg(login));
    exit(EXIT_FAILURE);
  }

  if (false == env.contains("XAUTHORITY"))
  {
    if (true == isExistsEnv())
    {
      setenv("XAUTHORITY", QString(env.value("HOME") + "/.Xauthority").toUtf8(), 1);
    }
    else
    {
      writeErr(tr("Не удалось установить переменные окружения. Приложение будет закрыто."));
      ::exit(EXIT_FAILURE);
    }
  }

  QString libEnv = "";
  if (true == isExistsLibPath())
  {
    libEnv = QString("LD_LIBRARY_PATH=" + MnCommon::libPath());
  }
  else
  {
    writeErr(tr("Не удалось установить переменные окружения. Приложение будет закрыто."));
    ::exit(EXIT_FAILURE);
  }

  struct winsize win;
  win.ws_col = 80;
  win.ws_row = 24;
  win.ws_xpixel = 480;
  win.ws_ypixel = 192;
  char name[255];
  ::memset( name, 0, 255 );

  pid = forkpty(&master, name, NULL, &win);
  if(pid == 0)
  {
    auto currentLogin = meteo::TAdminSettings::instance()->getCurrentUserLogin();
    QString cmd = libEnv + " " + appName_ + " --username " + currentLogin;
    //QString cmd2 = "/bin/echo "+password+" | /bin/su -p "+login+" -c "+appName_;
    //debug_log<<cmd2;
    int err = execl("/bin/su","/bin/su", "-p", login.toUtf8().data(), "-c", cmd.toUtf8().data(), NULL );
    //int err = system(cmd2.toStdString().data());
    if (err != 0)
    {
      writeErr(tr("Невозможно запустить программу. %1").arg(err));
    }
  }
  else if (pid == -1) { //неудачно
    writeErr(tr("Не удалось запустить программу"));
    ::exit(EXIT_FAILURE);
  }
  else {
    QString passw = QString(password+"\n");
    ::open(name, O_RDWR | O_NONBLOCK );
    QByteArray arr(1024,0);
    int res = ::read( master, arr.data(), 1024 );
    ssize_t answer = ::write( master, passw.toUtf8().data(), passw.count()+1 );
    Q_UNUSED(answer);
    while (0 < res) {
      std::cout << arr.data();
      arr.fill('\0');
      res = ::read( master, arr.data(), 1024 );
      lastMsg =  QString::fromUtf8(arr.data());
    }
  }
}

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  QApplication app( argc, argv );
//  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  app.setQuitOnLastWindowClosed(true);

  TArg args( argc, argv );
  QString appName, userName;
  if(argc <= 1){
    ::help();
    QString msg = QObject::tr("<center><b>Не заданы параметры запуска.</b></center><br/>")
                + QObject::tr("<b>--help</b>, <b>-h</b> показать справку<br/>")
                + QObject::tr("<center>Формат параметров:</center><br/>")
                + QObject::tr("<b>--path=ПУТЬ</b> полный путь к запускаемому приложению (обязательно для заполнения)<br/><br/>")
                + QObject::tr("<b>--user=ИМЯ ПОЛЬЗОВАТЕЛЯ</b> имя пользователя, от которого следует выполнить запуск (по умолчанию root)<br/><br/>")
                + QObject::tr("<center>Сокращенный формат:</center><br/>")
                + QObject::tr("<b>-p ПУТЬ</b> полный путь к запускаемому приложению (обязательно для заполнения)<br/><br/>")
                + QObject::tr("<b>-u ИМЯ ПОЛЬЗОВАТЕЛЯ</b> имя пользователя, от которого следует выполнить запуск (по умолчанию root)<br/><br/>");
    //QMessageBox::information(0,QObject::tr("Ошибка"), msg, QObject::tr("Закрыть"));
    writeErr(msg);
    exit(EXIT_SUCCESS);
  }
  if((args.contains("help")) || (args.contains("h"))){
      ::help();
      return EXIT_SUCCESS;
  }
  if((args.contains("path")) && (args.value("path") != ""))
  {
      appName = args.value("path");
  }
  else if ((args.contains("p")) && (args.value("p") != ""))
  {
      appName = args.value("p");
  }
  else
  {
    QString msg = QObject::tr("<center><b>Неверно заданы параметры запуска.</b></center><br/>")
                + QObject::tr("<b>--help</b>, <b>-h</b> показать справку<br/>")
                + QObject::tr("<center>Формат параметров:</center><br/>")
                + QObject::tr("<b>--path=ПУТЬ</b> полный путь к запускаемому приложению (обязательно для заполнения)<br/><br/>")
                + QObject::tr("<b>--user=ИМЯ ПОЛЬЗОВАТЕЛЯ</b> имя пользователя, от которого следует выполнить запуск (по умолчанию root)<br/><br/>")
                + QObject::tr("<center>Сокращенный формат:</center><br/>")
                + QObject::tr("<b>-p ПУТЬ</b> полный путь к запускаемому приложению (обязательно для заполнения)<br/><br/>")
                + QObject::tr("<b>-u ИМЯ ПОЛЬЗОВАТЕЛЯ</b> имя пользователя, от которого следует выполнить запуск (по умолчанию root)<br/><br/>");
    //QMessageBox::warning(0,QObject::tr("Ошибка"), msg, QObject::tr("Закрыть"));
    writeErr(msg);
    exit(EXIT_FAILURE);
  }
  if((args.contains("user")) && (args.value("user") != ""))
  {
    userName = args.value("user",ADMINLOGIN);
  }
  else if ((args.contains("u")) && (args.value("u") != ""))
  {
    userName = args.value("u",ADMINLOGIN);
  }
  else
  {
    userName = ADMINLOGIN;
  }

  struct sigaction act;
  act.sa_handler = signalHandler;
  (void) sigemptyset (&act.sa_mask);
  act.sa_flags = 0;

  (void) sigaction (SIGABRT, &act, (struct sigaction *) NULL);
  (void) sigaction (SIGCHLD, &act, (struct sigaction *) NULL);
  (void) sigaction (SIGFPE, &act, (struct sigaction *) NULL);
  (void) sigaction (SIGHUP, &act, (struct sigaction *) NULL);
  (void) sigaction (SIGILL, &act, (struct sigaction *) NULL);
  (void) sigaction (SIGINT, &act, (struct sigaction *) NULL);
  (void) sigaction (SIGQUIT, &act, (struct sigaction *) NULL);
  (void) sigaction (SIGSEGV, &act, (struct sigaction *) NULL);
  (void) sigaction (SIGTERM, &act, (struct sigaction *) NULL);
  (void) sigaction (SIGXCPU, &act, (struct sigaction *) NULL);
  (void) sigaction (SIGXFSZ, &act, (struct sigaction *) NULL);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGCONT, SIG_IGN);

  TAppSu* su = new TAppSu(appName,userName);
  su->init();

  int res = app.exec();
  return res;
}
