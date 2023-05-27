#include "niitvprotocol.h"


namespace mappi {

    namespace antenna {

        static const float TRANSMISSION_RATIO = 8191;

        static const qint8 MASK_ANGLE_LOW = 0x3F;
        static const qint8 MASK_ANGLE_HIGH = 0x7F;

        static const qint8 MASK_NIBBLE_LOW = 0x0F;
        static const qint8 MASK_NIBBLE_HIGH = 0xF0;

        static const qint8 CMD_TRACKER = 0x00;
        static const qint8 CMD_STOP = 0x10;
        static const qint8 CMD_DIRECT = 0x20;
        static const qint8 CMD_CURRENT_POSITION = 0x40;    // не поддерживается API, альтернатива CMD_STOP

        static const QByteArray AZIMUT_PACK = QByteArray::fromHex("800000");
        static const QByteArray ELEVAT_PACK = QByteArray::fromHex("C00000");


        NiitvProtocol::NiitvProtocol() {
        }

        NiitvProtocol::~NiitvProtocol() {
        }

        QByteArray NiitvProtocol::pack(int id, const MnSat::TrackTopoPoint& point) {
            return (packAzimut(id, point) + packElevat(id, point));
        }

        QByteArray NiitvProtocol::packAzimut(int id, const MnSat::TrackTopoPoint& point) {
            QByteArray buf = AZIMUT_PACK;
            toPack(id, point.getAzimut().value().toDeg(), TRANSMISSION_RATIO / 720, buf);
            // qDebug() << "pack azimut:" << buf.toHex();

            return buf;
        }

        QByteArray NiitvProtocol::packElevat(int id, const MnSat::TrackTopoPoint& point) {
            QByteArray buf = ELEVAT_PACK;
            toPack(id, point.getElevation().value().toDeg(), TRANSMISSION_RATIO / 360, buf);
            // qDebug() << "pack elevat:" << buf.toHex();

            return buf;
        }

        bool NiitvProtocol::unpack(const QByteArray &buf, Drive *azimut, Drive *elevat, QString *error /*=*/) {
            float factor = 720 / TRANSMISSION_RATIO;
            Drive *drive = azimut;

            if ((buf[0] & 0xC0) == 0xC0) {
                factor = 360 / TRANSMISSION_RATIO;
                drive = elevat;
            }

            if (drive != nullptr) {
                drive->state = Drive::OK;
                ++drive->seq;

                // crc
                if (crc(buf) != 0) {
                    drive->state = Drive::CRC_NOT_VALID;
                    if (error) *error = QObject::tr("CRC not valid");

                    debug_log << buf.toHex();
                    return false;
                }

                // sensor angel fail
                if ((buf[2] & 0x20) == 0x20) {
                    drive->state = Drive::SENSOR_ANGLE_FAIL;
                    if (error) *error = QObject::tr("sensor angle fail");

                    debug_log << buf.toHex();
                    return false;
                }

                // тайну обработки невалидной CRC знает только Юра
                drive->self = getAngle(buf, factor);
                // debug_log << drive->toString();
            }

            return true;
        }

        bool NiitvProtocol::split(const QByteArray &buf) const {
            return (((buf[0] & 0x80) == 0x80) &&
                    ((buf[1] & 0x80) == 0x00) &&
                    ((buf[2] & 0x80) == 0x00)
            );
        }

        bool NiitvProtocol::split(const QByteArray &, int) const {
            return true;
        }

        void NiitvProtocol::toPack(int id, float v, float factor, QByteArray &buf) const {
            switch (id) {
                case Protocol::DIRECT :
                    buf[2] = CMD_TRACKER;
                    setAngle(v, factor, buf);
                    break;

                    // case Protocol::TRAVEL_MODE :
                case Protocol::TRACKER :
                case Protocol::STOP :

                default :
                    buf[2] = CMD_STOP;
                    break;
            }

            buf[2] = buf[2] | crc(buf);
        }

        float NiitvProtocol::getAngle(const QByteArray &buf, float factor) const {
            qint16 v = (buf[1] & MASK_ANGLE_HIGH) << 6;
            v |= buf[0] & MASK_ANGLE_LOW;
            v = v << 3;                   // сохранение знака

            return ((v / 8) * factor);      // деление на 8 - компенсация << 3
        }

        void NiitvProtocol::setAngle(float angle, float factor, QByteArray &buf) const {
            qint16 v = angle * factor;
            // qDebug() << QString("%1 %2")
            //   .arg(v)
            //   .arg(v, 16, 2, QChar('0'));

            buf[0] = buf[0] | (v & MASK_ANGLE_LOW);
            buf[1] = buf[1] | ((v >> 6) & MASK_ANGLE_HIGH);       // сохранение знака
        }

        qint8 NiitvProtocol::crc(const QByteArray &buf) const {
            qint8 res = 0;
            for (int i = 0; i < buf.size(); ++i) {
                res += (buf.at(i) >> 4) & MASK_NIBBLE_LOW;
                res += buf.at(i) & MASK_NIBBLE_LOW;
            }

            return ((0 - res) & MASK_NIBBLE_LOW);
        }

    }

}
