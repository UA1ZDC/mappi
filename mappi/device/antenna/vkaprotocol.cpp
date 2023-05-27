#include "vkaprotocol.h"
#include <qregexp.h>


namespace mappi {

    namespace antenna {

        static const QString DLE("5555");
        static const QString ETX("\r\n");
        static const QString RESPONSE_PATTERN("^(5555)(\\d{2})(\\d{4})([+-]\\d{5})([+-]\\d{5})(\\d{4})(.*)$");
        static const int PAYLOAD = 22;


        VkaProtocol::VkaProtocol() :
                seq_(0) {
        }

        VkaProtocol::~VkaProtocol() {
        }

        QByteArray VkaProtocol::pack(int id, const MnSat::TrackTopoPoint& point) {
            if (seq_ == 100)
                seq_ = 0;

            QString payload = QString("%1%2%3%4%5")
                    .arg(seq_++, 2, 10, QChar('0'))                   // seq пакета
                    .arg(id, 2, 10, QChar('0'))                       // id команды
                    .arg(0, 6, 10, QChar('0'))                        // данные 1
                    .arg(angleToString(point.getAzimut().value()))    // данные 2
                    .arg(angleToString(point.getElevation().value()));// данные 3
            QString packet = QString("%1%2%3%4")
                    .arg(DLE)                           // "5555"
                    .arg(payload)                       // полезная нагрузка
                    .arg(crc(payload))                  // LRC
                    .arg(ETX);                           // "\r\n"
            return packet.toLocal8Bit();
        }

        QByteArray VkaProtocol::packAzimut(int id, const MnSat::TrackTopoPoint& point) {
            // WARNING протокол не поддерживает асинхронное выполнение
            Q_UNUSED(id)
            Q_UNUSED(point)

            return QByteArray();
        }

        QByteArray VkaProtocol::packElevat(int id, const MnSat::TrackTopoPoint& point) {
            // WARNING протокол не поддерживает асинхронное выполнение
            Q_UNUSED(id)
            Q_UNUSED(point)

            return QByteArray();
        }

        bool VkaProtocol::unpack(const QByteArray &buf, Drive *azimut, Drive *elevat, QString *error /*=*/) {
            QString str(buf);

            int pos = 0;
            QRegExp rx(RESPONSE_PATTERN);
            if ((rx.indexIn(str, pos)) == -1) {
                if (error) *error = QObject::tr("format fail");
                return false;
            }

            QString state = rx.cap(3);
            quint16 state_ctrl = state.left(2).toInt(nullptr, 16);

//            debug_log << "Data from controller:";
//            debug_log << "seq:   " << rx.cap(2);
//            debug_log << "state: " << state_ctrl << state.right(2).toInt(nullptr, 16);
//            debug_log << "azimut:" << rx.cap(4);
//            debug_log << "elevat:" << rx.cap(5);

            QString tail = rx.cap(7);
            if ((DLE != rx.cap(1)) || (!tail.contains(ETX))) {
                if (error) *error = QObject::tr("match DLE, ETX fail");
                return false;
            }

            if (tail.replace("\r\n", "") != crc(str.mid(DLE.length(), PAYLOAD))) {
                if (azimut != nullptr) azimut->state = Drive::CRC_NOT_VALID;
                if (elevat != nullptr) elevat->state = Drive::CRC_NOT_VALID;

                if (error) *error = QObject::tr("CRC fail");
                return false;
            }

            if (azimut != nullptr) {
                azimut->state = Drive::OK;
                ++azimut->seq;

                quint16 st = state_ctrl && 0xA000;
                switch (st) {
                    case 0x8000 :
                        azimut->state = Drive::FAIL;
                        break;

                    case 0x4000 :
                        azimut->state = Drive::SENSOR_ANGLE_FAIL;
                        break;

                    default :
                        // тайну обработки невалидной CRC знает только Юра
                        azimut->self = getAngle(rx.cap(4));
                        break;
                }
            }

            if (elevat != nullptr) {
                elevat->state = Drive::OK;
                ++elevat->seq;

                quint16 st = state_ctrl && 0x5000;
                switch (st) {
                    case 0x2000 :
                        elevat->state = Drive::FAIL;
                        break;

                    case 0x1000 :
                        elevat->state = Drive::SENSOR_ANGLE_FAIL;
                        break;

                    default :
                        // тайну обработки невалидной CRC знает только Юра
                        elevat->self = getAngle(rx.cap(5));
                        break;
                }
            }

            // debug_log << azimut->toString() << elevat->toString();

            return true;
        }

        bool VkaProtocol::split(const QByteArray &buf) const {
            return (buf.indexOf(ETX.toLocal8Bit(), 0) != -1);
        }

        bool VkaProtocol::split(const QByteArray &buf, int mtu) const {
            return (buf.indexOf(ETX.toLocal8Bit(), 0) == mtu - ETX.length());
        }

        float VkaProtocol::getAngle(const QString &buf) const {
            return (buf.toDouble() * 0.01);
        }

        QString VkaProtocol::angleToString(const MnSat::AngleWithVelocity& angle){
            float v = std::max((float)-999.99, std::min(angle.value().toDeg(), (float)999.99)); //clip 5 знаков
            QChar numSign = QChar(0 <= v ? '+' : '-');
            int angleX100 = int(fabs(v) * 100); // точность 2 знака после запятой, без разделителя
            return QString("%1%2")
                    .arg(numSign)
                    .arg(angleX100, 5, 10, QChar('0'));
        }

        QString VkaProtocol::crc(const QString &buf) const {
            QByteArray str(buf.toLocal8Bit());

            unsigned char sum = 0;
            for (int i = 0; i < str.size(); ++i)
                sum += str[i];

            return QString::number((256 - sum), 16)
                    .toUpper();
        }

    }

}
