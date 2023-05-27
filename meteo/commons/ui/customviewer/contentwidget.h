#ifndef CONTENTWIDGET_H
#define CONTENTWIDGET_H

//#include <meteo/commons/proto/msgconf.pb.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include "decode/decodeviewer.h"
#include <qbytearray.h>
#include <qdialog.h>
#include <qstring.h>
#include <qmap.h>
#include <meteo/commons/ui/previewwidget/viewimagepreview.h>
#include <meteo/commons/proto/msgcenter.pb.h>


namespace Ui{
  class ContentWidget;
}

class FaxViewer;
namespace meteo {
  class DecodeViewer;
}

namespace meteo {

class ContentWidget : public QDialog
{
  Q_OBJECT
public:
  explicit ContentWidget(QWidget *parent, const QString& id_columni = QString("_id") );
  ~ContentWidget();

  bool init(meteo::tlg::MessageNew* tlg, const QMap<QString, QString>& info);

public slots:
  //void slotReloadContent(const QList<u_int64_t>& messagesId);

private slots:
  void slotLoadMsg();
  void slotOpenFile();
  void slotSaveFile();
  void slotEditMsg();
  void slotSaveMsg();

private:
  bool loadBinMessage();
  bool loadTextMessage();
  bool loadImage(const QByteArray& arr);
  void loadFaxImage();
  void loadFile();

//  void loadDecodedContent(const QString& msgid);
  void loadDecodedContent(const QByteArray& tlg, meteo::TypeTelegram type, const QString& dt);
  QString replaceContent(const QByteArray& arr, bool flag);

  QString decodeMsg(const QByteArray& raw) const;

  bool extractFiles(meteo::msgcenter::GetTelegramResponse* reply);

private:
  Ui::ContentWidget* ui_;
  meteo::ViewImagePreview* imagePreview_;
  meteo::DecodeViewer* decodeview_;
  meteo::tlg::MessageNew* message_;
  QMap<QString, QString> info_;
  QString file_name_;
  QString id_column_;
};

}

#endif // CONTENTWIDGET_H
