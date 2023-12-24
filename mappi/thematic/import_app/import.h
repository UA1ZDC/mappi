#include <mappi/thematic/algs_calc/dataservstore.h>
#include <mappi/pretreatment/savenotify/savenotify.h>

#include <gdal/gdal_priv.h>

#include <QDateTime>
#include <QImage>
#include <QSharedPointer>

namespace mappi::Import{
  enum ColorChannel{
    RED = 0,
    GREEN = 1,
    BLUE = 2,
    ALPHA = 3
  };
  enum SaveAs{
    RGBA = 0,
    RGB = 1,
    GRAYSCALE = 2
  };
  class GDALImporter{
  public:
    GDALImporter();
    ~GDALImporter();
    void closeDataset();
    void clearImage();
    bool load(const QString &loadPath);
    bool save(const QString& filePath);
    bool saveImage(const QString& filePath);
    bool createSession(const QString &filePath);
    bool saveData(const QString& filePath);
    void setPath(const QString& newPath) { _loadPath = newPath; }
    void setName(const QString& themName) { _them_name = themName.toStdString(); }
    void setSatellite(const QString& satelliteName) { _satellite = satelliteName; }
    void setStart(const QDateTime& dtStart) { _dtStart = dtStart; }
    void setStop(const QDateTime& dtStop) { _dtStop = dtStop; }
    void setFormat(const SaveAs new_format) { _format = new_format; }
    void setInstrument(const mappi::conf::InstrumentType new_instr){ _instr = new_instr; }

    QString getName() const { return QString::fromStdString(_them_name); }
    QString getSatellite() const { return _satellite; }
    QDateTime getStart() const { return _dtStart; }
    QDateTime getStop() const { return _dtStop; }
    QImage getImage() const { return _image; }
    SaveAs getFormat() const { return _format; }
    mappi::conf::InstrumentType getInstrument(){ return _instr; }
    unsigned int getChannelFor(ColorChannel color) const { return _channels[color]; }

    virtual unsigned int getBandsCount() const { return _rasterBandsCount; }
    virtual bool setBand(ColorChannel color, unsigned int idx);

  protected:
    GDALDataset* _ds = nullptr;
    QSharedPointer<mappi::to::DataStore> _store;
    QSharedPointer<mappi::SaveNotify> _notify;
    QImage _image;
    QString _loadPath;
    QDateTime _dtStart;
    QDateTime _dtStop;
    std::string _them_name = "Импорт";
    QString _satellite = "Импорт";
    mappi::conf::InstrumentType _instr = mappi::conf::kAvhrr3;
    unsigned int _channel = 1;
    unsigned int _rasterBandsCount = 0;
    unsigned int _channels[4] = {0, 0, 0, 0};
    SaveAs _format = RGBA;
  };
}