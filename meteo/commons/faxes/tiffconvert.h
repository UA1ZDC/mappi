#ifndef METEO_COMMONS_FAXES_TIFFCONVERT_H
#define METEO_COMMONS_FAXES_TIFFCONVERT_H

class QByteArray;
class QString;

namespace meteo {
namespace internal {

bool saveAsTiff(const QByteArray& rawData, int lpm, const QString& outputFileName);
bool saveAsTiff(const QByteArray& rawData, int lpm, QByteArray* output);

int imageWidth(int lpm);
int imageLength(int sz, int lpm);

} // internal
} // meteo

#endif // METEO_MASLO_FAXES_TIFFCONVERT_H
