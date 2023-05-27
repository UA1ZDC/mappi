#ifndef METEO_NOVOST_UI_TLGEDITOR_LIB_TLGEDITORWINDOW_H
#define METEO_NOVOST_UI_TLGEDITOR_LIB_TLGEDITORWINDOW_H

#include <qmap.h>
#include <qmainwindow.h>

namespace Ui {
class TlgEditorWindow;
}

namespace rpc {
class Channel;
} // rpc

namespace meteo {
class DecodeViewer;
} // meteo

namespace meteo {

//!
class TlgEditorWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit TlgEditorWindow(QWidget *parent = 0);
  ~TlgEditorWindow();

  //!
  bool setRawTlg(const QByteArray& raw);
  //!
  QByteArray getRawTlg() const;
  //!
  bool openFile(const QString& fileName);

public slots:
  void slotNewTlg();
  void slotOpenFile();
  void slotSaveFile();
  void slotSaveAsFile();
  void slotRecodeTlg();
  void slotReloadFile();
  void slotSendTlg();

  void slotInsertSpecSymb();

  void slotUpdateDecodeTlg();

protected:
  virtual void closeEvent(QCloseEvent* event);

private:
  bool checkModified(const QString& text, const QString& btn);
  QString detectCodec(const QByteArray& raw) const;

private:
  Ui::TlgEditorWindow* ui_;
  DecodeViewer* decodeview_;

  QString fileName_;
  QString codecName_;
  QByteArray rawData_;

  QMap<QAction*,QString> actSymbols_;
};

} // meteo

#endif // METEO_NOVOST_UI_TLGEDITOR_LIB_TLGEDITORWINDOW_H
