#pragma once

#include "drive.h"
#include <cross-commons/debug/tlog.h>
#include <sat-commons/satellite/tracktopopoint.hpp>
#include <qbytearray.h>
#include <QtDebug>


namespace mappi {

    namespace antenna {
        class Protocol {
        public :
            enum command_t {
                DIRECT = 1,
                TRACKER,
                STOP,
                TRAVEL_MODE,
                FEEDHORN = 16,
                FEEDHORN_OUTPUT = 17
            };

        public :
            Protocol() {}

            virtual ~Protocol() {}

            virtual bool isText() const = 0;

            virtual bool isAsyncExec() const = 0; // флаг определяет способ выполнения команды
            virtual bool hasVelocity() const = 0; // флаг определяет наличие угловых скоростей
            virtual bool hasTime() const = 0; // флаг определяет наличие времени
            virtual int mtu() const = 0;          // message transport unit
            virtual const char *type() const = 0;

            // синхронное выполнение команды
            virtual QByteArray pack(int id, const MnSat::TrackTopoPoint& point) = 0;

            [[deprecated("Use TrackTopoPoint version instead.")]]
            QByteArray pack(
                    int id,
                    float azimut,
                    float elevat,
                    float azimut_speed = 0,
                    float elevat_speed = 0,
                    QDateTime time = QDateTime::fromMSecsSinceEpoch(0)
            ) {
                return pack(id, MnSat::TrackTopoPoint(azimut, elevat, azimut_speed, elevat_speed, time));
            };

            // асинхронное выполнение команд
            virtual QByteArray packAzimut(int id, const MnSat::TrackTopoPoint& point) = 0;

            [[deprecated("Use Use TrackTopoPoint version instead.")]]
            QByteArray packAzimut(int id, float value, float speed = 0) {
                return packAzimut(id, MnSat::TrackTopoPoint(value, 0, speed, 0));
            };

            virtual QByteArray packElevat(int id, const MnSat::TrackTopoPoint& point) = 0;

            [[deprecated("Use Use TrackTopoPoint version instead.")]]
            QByteArray packElevat(int id, float value, float speed = 0) {
                return packElevat(id, MnSat::TrackTopoPoint(0, value, 0, speed));
            };

            virtual bool unpack(const QByteArray &buf, Drive *azimut, Drive *elevat, QString *error = nullptr) = 0;

            virtual bool split(const QByteArray &buf) const = 0;

            virtual bool split(const QByteArray &buf, int mtu) const = 0;

        };

    }

}
