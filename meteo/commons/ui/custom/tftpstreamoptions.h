#ifndef TFTPSTREAMOPTIONS_H
#define TFTPSTREAMOPTIONS_H

#include <qwidget.h>
//#include <qmap.h>

class QString;
//class QEvent;
//class QLineEdit;
//class QMenu;

namespace Ui{
   class FtpStreamOptions;
}

class TFtpStreamOptions : public QWidget {
  Q_OBJECT

  public:
    TFtpStreamOptions( QWidget * parent = 0);
    ~TFtpStreamOptions();

    void showSender();
    void showReceiver();

    bool setPath(const QString& value);

    bool setSrvHost(const QString& value);
    bool setSrvPort(int value);
    bool setSrvUser(const QString& value);
    bool setSrvPass(const QString& value);

    bool setReceiverFileTempl(const QString& value);
    bool setReceiverDeleteFiles(const QString& value);
    bool setReceiverEncoding(const QString& value);
    bool setReceiverMetaTempl(const QString& value);
    bool setReceiverWaitForRead(int value);
    bool setReceiverSessionTimeout(int value);
    bool setReceiverUnprocessedPath(const QString& value);

    bool setSenderMsgLimit(int value);
    bool setSenderSizeLimit(int value);
    bool setSenderTimeLimit(int value);
    bool setSenderMask(const QString& value);

    QString getPath();

    QString getSrvHost();
    int getSrvPort();
    QString getSrvUser();
    QString getSrvPass();

    QString getReceiverFileTempl();
    bool getReceiverDeleteFiles();
    QString getReceiverEncoding();
    QString getReceiverMetaTempl();
    int getReceiverWaitForRead();
    int getReceiverSessionTimeout();
    QString getReceiverUnprocessedPath();

    int getSenderMsgLimit();
    int getSenderSizeLimit();
    int getSenderTimeLimit();
    QString getSenderMask();

  private:
    Ui::FtpStreamOptions* _ui;

    friend class TStreamOptions;
};


#endif
