#ifndef TFILESTREAMOPTIONS_H
#define TFILESTREAMOPTIONS_H

#include <qwidget.h>
//#include <qmap.h>

class QString;
//class QEvent;
//class QLineEdit;
//class QMenu;

namespace Ui{
   class FileStreamOptions;
}

class TFileStreamOptions : public QWidget {
  Q_OBJECT
  public:
    TFileStreamOptions( QWidget * parent = 0);
    ~TFileStreamOptions();

    void showSender();
    void showReceiver();

    bool setPath(const QString& value);

    bool setReceiverFileTempl(const QString& value);
    bool setReceiverDeleteFiles(const QString& value);
    bool setReceiverEncoding(const QString& value);
    bool setReceiverMetaTempl(const QString& value);
    bool setReceiverWaitForRead(int value);

    bool setSenderMsgLimit(int value);
    bool setSenderSizeLimit(int value);
    bool setSenderTimeLimit(int value);
    bool setSenderMask(const QString& value);

    QString getPath();

    QString getReceiverFileTempl();
    bool getReceiverDeleteFiles();
    QString getReceiverEncoding();
    QString getReceiverMetaTempl();
    int getReceiverWaitForRead();

    int getSenderMsgLimit();
    int getSenderSizeLimit();
    int getSenderTimeLimit();
    QString getSenderMask();

private:
    Ui::FileStreamOptions* _ui;

    friend class TStreamOptions;
};


#endif
