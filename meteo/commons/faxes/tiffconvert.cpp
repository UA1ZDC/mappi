#include "tiffconvert.h"
#include <sstream>

#include <cross-commons/debug/tlog.h>

#include <QByteArray>
#include <QString>

#include <tiffio.h>
#include <tiffio.hxx>

namespace {
  int samplesPerPixel() { return 1; }
}

namespace meteo {
namespace internal {

int imageWidth(int lpm)
{
  return 216000/lpm;
}

int imageLength(int sz, int lpm)
{
  return 8*(sz/imageWidth(lpm));
}

bool saveAsTiff(const QByteArray& rawData, int lpm, const QString& outputFileName)
{
  if (rawData.isEmpty() == true) {
    error_log << QString::fromUtf8("Ошибка. Для записи передан пустой массив");
    return false;
  }
  if ((lpm == 60 || lpm == 90 || lpm == 120) == false) {
    error_log << QString::fromUtf8("Ошибка. Некратное значение LPM: %1").arg(lpm);
    return false;
  }
  if (outputFileName.isEmpty() == true) {
    error_log << QString::fromUtf8("Ошибка. Пустое имя выходного файла");
    return false;
  }

  TIFF* tfile = TIFFOpen(outputFileName.toUtf8().data(), "w");
  if (tfile == nullptr) {
    error_log << QString::fromUtf8("Ошибка открытия файла %1 для записи").arg(outputFileName);
    return false;
  }

  int width = imageWidth(lpm);
  int length = imageLength(rawData.size(), lpm);

  TIFFSetField(tfile, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(tfile, TIFFTAG_IMAGELENGTH, length);
  TIFFSetField(tfile, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
  TIFFSetField(tfile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
  TIFFSetField(tfile, TIFFTAG_SAMPLESPERPIXEL, ::samplesPerPixel());
  TIFFSetField(tfile, TIFFTAG_ROWSPERSTRIP, length);
  TIFFSetField(tfile, TIFFTAG_ORIENTATION, ORIENTATION_RIGHTTOP);

  TIFFWriteEncodedStrip(tfile, 0, const_cast<char*>(rawData.data()), rawData.size());
  TIFFClose(tfile);
  return true;
}

bool saveAsTiff(const QByteArray& rawData, int lpm, QByteArray* output)
{

  if (rawData.isEmpty() == true) {
    error_log << QString::fromUtf8("Ошибка. Для записи передан пустой массив");
    return false;
  }
  if ((lpm == 60 || lpm == 90 || lpm == 120) == false) {
    error_log << QString::fromUtf8("Ошибка. Некратное значение LPM: %1").arg(lpm);
    return false;
  }
  if ( nullptr == output ){
    error_log << QObject::tr("output равен nullptr, запись выходного файла невозможна.");
    return false;
  }

  std::ostringstream tiffOutputStream;

  TIFF* tfile = TIFFStreamOpen("DUMMY", &tiffOutputStream);
  if (tfile == nullptr) {
    error_log << QString::fromUtf8("Ошибка создания потока для записи");
    return false;
  }
  int width = imageWidth(lpm);
  int length = imageLength(rawData.size(), lpm);

  TIFFSetField(tfile, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(tfile, TIFFTAG_IMAGELENGTH, length);
  TIFFSetField(tfile, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
  TIFFSetField(tfile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
  TIFFSetField(tfile, TIFFTAG_SAMPLESPERPIXEL, ::samplesPerPixel());
  TIFFSetField(tfile, TIFFTAG_ROWSPERSTRIP, length);
  TIFFSetField(tfile, TIFFTAG_ORIENTATION, ORIENTATION_RIGHTTOP);

  TIFFWriteEncodedStrip(tfile, 0, const_cast<char*>(rawData.data()), rawData.size());
  TIFFClose(tfile);
  const std::string data = tiffOutputStream.str();
  *output = QByteArray(data.c_str(), data.size());
  return true;
}

} // internal
} // meteo
