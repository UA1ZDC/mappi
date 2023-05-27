#include "anglespeedprotocol.h"

namespace mappi {

    namespace antenna {

        static const QString DLE("5555");
        static const QString ETX("\r\n");
        static const QString RESPONSE_PATTERN("^(5555)(\\d{2})(\\d{4})([+-]\\d{5})([+-]\\d{5})(\\d{4})(.*)$");
        static const int PAYLOAD = 22;

        AngleSpeedProtocol::AngleSpeedProtocol() : seq_(0) {
        }

        AngleSpeedProtocol::~AngleSpeedProtocol() {
        }

        QByteArray AngleSpeedProtocol::pack(int id, const MnSat::TrackTopoPoint& point) {
            if (seq_ == 100) seq_ = 0;

            QString payload = QString("%1%2%3%4%5%6%7")
                    .arg(seq_++, 2, 10, QChar('0'))                // seq пакета
                    .arg(id, 2, 10, QChar('0'))                    // id команды
                    .arg(point.seconds())                          // данные 1
                    .arg(angleToString(point.getAzimut()))         // данные 2
                    .arg(angleToString(point.getElevation()))      // данные 3
                    .arg(velocityToString(point.getAzimut()))      // данные 4
                    .arg(velocityToString(point.getElevation()));  // данные 5

            QString packet = QString("%1%2%3%4")
                    .arg(DLE)                           // "5555"
                    .arg(payload)                       // полезная нагрузка
                    .arg(crc(payload))                  // LRC
                    .arg(ETX)                           // "\r\n"
                    .toLocal8Bit();
            return packet.toLocal8Bit();
        }

        QString AngleSpeedProtocol::velocityToString(const MnSat::AngleWithVelocity& angle){
            float v = std::max((float)-99.999, std::min(angle.velocity().toDeg(), (float)99.999)); //clip 5 знаков
            QChar numSign = QChar(0 <= v ? '+' : '-');
            int velX1000 = int(fabs(v) * 1000); // точность 3 знака после запятой, без разделителя
            return QString("%1%2")
                    .arg(numSign)
                    .arg(velX1000, 5, 10, QChar('0'));
        }

    }

}
