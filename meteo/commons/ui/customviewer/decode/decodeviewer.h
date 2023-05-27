#ifndef METEO_NOVOST_UI_MSGVIEWER_DECODE_DECODEVIEWER_H
#define METEO_NOVOST_UI_MSGVIEWER_DECODE_DECODEVIEWER_H

#include <qwidget.h>
#include <qmenu.h>
#include <qmap.h>
#include <meteo/commons/proto/tgribformat.pb.h>

template<typename T> class QList;
class QString;
class QTreeWidgetItem;
class NS_PGBase;
class QTableWidgetItem;

namespace rpc {
  class SelfCheckedController;
} // rpc

class TMeteoData;

namespace grib {
  class TGribData;
}

namespace Ui {
  class DecodeViewer;
}

namespace meteo {

enum TypeTelegram {
  GRIB        = 0,
  BUFR        = 1,
  ALPHANUM    = 2,
  ALPHANUMGPH = 3
};

class DecodeViewer : public QWidget
{
  Q_OBJECT

public:
  explicit DecodeViewer(QWidget* parent = nullptr);
  virtual ~DecodeViewer() override;
  void init(const QByteArray& tlg, TypeTelegram type, const QString& dt);

protected:
  virtual void hideEvent(QHideEvent* event) override;

private:
  const QList<TMeteoData> getDecodedContent(const QByteArray& tlg, TypeTelegram type, const QString& dt);
  const QList<grib::TGribData> getDecodedGrib(const QByteArray& tlg);
  void parseDecodedContent(const TMeteoData& content);
  void parseDecodedContentRecursively(const TMeteoData& content) const;
  void parseDecodedGrib(const grib::TGribData& content) const;
  void loadReplaceDescriptors(const TMeteoData& content);
  void loadCodeText();
  void showError(const QString& errorMessage);
  QSize tableSize() const;
  void saveTableGeometry();
  void restoreTableGeomerty();

private slots:
  void  hideRow(bool on);
  void slotColumnsBtnClicked();
  void slotMenuActionTriggered();

private:
  Ui::DecodeViewer* ui_;
  QTableWidgetItem* headerItem_;
  QMenu columnsVisibilityMenu_;

signals:
  void reload();
};

}

#endif
