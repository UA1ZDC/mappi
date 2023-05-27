#ifndef TUNIMASSTREAMOPTIONS_H
#define TUNIMASSTREAMOPTIONS_H

#include <qwidget.h>
//#include <qmap.h>

class QString;
//class QEvent;
//class QLineEdit;
//class QMenu;

namespace Ui{
   class UnimasStreamOptions;
}

class TUnimasStreamOptions : public QWidget {
  Q_OBJECT

  public:
    TUnimasStreamOptions( QWidget * parent = 0);
    ~TUnimasStreamOptions();

    void showSender();
    void showReceiver();

    bool setTypeRec(int value);
    bool setModeRec(int value);
    bool setHostRec(const QString& value);
    bool setPortRec(int value);
    bool setSessionTimeoutRec(int value);
    bool setAskTimeoutRec(int value);
    bool setUnprocessedPathRec(const QString& value);
    bool setCompressModeRec(int value);

    int getTypeRec();
    int getModeRec();
    QString getHostRec();
    int getPortRec();
    int getSessionTimeoutRec();
    int getAskTimeoutRec();
    QString getUnprocessedPathRec();
    int getCompressModeRec();

    bool setTypeSend(int value);
    bool setModeSend(int value);
    bool setPortSend(int value);
    bool setAskTimeoutSend(int value);
    bool setCompressModeSend(int value);

    int getTypeSend();
    int getModeSend();
    int getPortSend();
    int getAskTimeoutSend();
    int getCompressModeSend();

private:
    Ui::UnimasStreamOptions* _ui;

    friend class TStreamOptions;
};


#endif
