#include "track.h"
#include "print.h"
#include <QtDebug>


namespace mappi {
    namespace antenna {

        static constexpr float PI = 3.1415926535;
        static constexpr float AZIMUT_MAX = (2 * PI);

        QString TrackUtils::info(const QList <MnSat::TrackTopoPoint> &track) {
            QStringList res;
            res.append("");
            MnSat::Angle minAzimut = track.constFirst().getAzimut().value();
            unsigned int minAzimutIndex = 0;
            QDateTime minAzimutTime = track.constFirst().time();

            MnSat::Angle maxAzimut = track.constFirst().getAzimut().value();
            unsigned int maxAzimutIndex = 0;
            QDateTime maxAzimutTime = track.constFirst().time();

            int i = 0;
            for (auto &point : track) {
                if (point.az() < minAzimut.toRad()) {
                    minAzimut = point.getAzimut().value().toDeg();
                    minAzimutTime = point.time();
                    minAzimutIndex = i;
                }
                if (point.az() > maxAzimut.toRad()) {
                    maxAzimut = point.getAzimut().value();
                    maxAzimutTime = point.time();
                    maxAzimutIndex = i;
                }
                i++;
            }

            res.append(
                    QString("Min Azimut Point: [%1] %2T az:%3")
                        .arg(minAzimutIndex)
                        .arg(minAzimutTime.toString("hh:mm:ss.zzz"))
                        .arg(minAzimut.toString())
            );
            res.append(
                    QString("Max Azimut Point: [%1] %2T az:%3")
                        .arg(maxAzimutIndex)
                        .arg(maxAzimutTime.toString("hh:mm:ss.zzz"))
                        .arg(maxAzimut.toString())
            );
            return res.join('\n');
        }

        void TrackUtils::gap_removal(QList <MnSat::TrackTopoPoint> &track) {
            MnSat::TrackTopoPoint *prevPoint = NULL;
            for (auto &point : track) {
                while (point.az() < 0) point.setAzimut(point.az() + AZIMUT_MAX);
                while (point.az() > AZIMUT_MAX) point.setAzimut(point.az() - AZIMUT_MAX);

                if (prevPoint == NULL) {
                    prevPoint = &point;
                    continue;
                }
                float correctedAzimutDeg = point.getAzimut().value().toDeg();
                unsigned int numFullTurns = std::trunc(prevPoint->getAzimut().value().toDeg() / 360.0);

                float rawDeltaAngle = correctedAzimutDeg - std::fmod(prevPoint->getAzimut().value().toDeg(), 360.0);
                float deltaAngles[4], angleOffsets[4] = {0, 360.0, -360.0, -720.0};
                for (int i = 0; i < 4; i++) deltaAngles[i] = std::abs(rawDeltaAngle + angleOffsets[i]);
                unsigned int minAngleIndex = std::distance(
                        std::begin(deltaAngles),
                        std::min_element(std::begin(deltaAngles), std::end(deltaAngles))
                );
                float curDeltaAngle = angleOffsets[minAngleIndex];

                correctedAzimutDeg += curDeltaAngle + numFullTurns * 360.0;
                point.setAzimut(MnSat::Angle::fromDeg(correctedAzimutDeg));
                prevPoint = &point;
            }
        }

        void TrackUtils::correction(QList <MnSat::TrackTopoPoint> &track, float offset) {
            for (auto &point : track) {
                point.setAzimut(point.az() + offset);
                while (point.az() < 0)
                    point.setAzimut(point.az() + AZIMUT_MAX);

                while (point.az() > AZIMUT_MAX)
                    point.setAzimut(point.az() - AZIMUT_MAX);
            }
        }

        bool TrackUtils::adaptation(QList <MnSat::TrackTopoPoint> &track, Profile::range_t azimuth_limits) {
            float limitsRadMin = MnSat::Angle::fromDeg(azimuth_limits.min).toRad();
            float limitsRadMax = MnSat::Angle::fromDeg(azimuth_limits.max).toRad();

            float minAzimuth = track.constFirst().az();
            float maxAzimuth = track.constLast().az();
            for (auto &point : track) {
                if (point.az() < minAzimuth) minAzimuth = point.az();
                if (point.az() > maxAzimuth) maxAzimuth = point.az();
            }
            // Если разница углов превышает возможности антенны - сразу выходим
            if ((maxAzimuth - minAzimuth) > (limitsRadMax - limitsRadMin)) return false;

            // Если диапазон углов трека находится правее левой границы рабочего диапазона антенны
            unsigned int numFullTurnsLeft = 0;
            if(minAzimuth > limitsRadMin) {
                numFullTurnsLeft = std::trunc((minAzimuth - limitsRadMin) / AZIMUT_MAX);
                minAzimuth -= numFullTurnsLeft * AZIMUT_MAX;
                maxAzimuth -= numFullTurnsLeft * AZIMUT_MAX;

                //Если даже после поворотов мы не смогли уложить траекторию - выходим
                if(maxAzimuth > limitsRadMax) return false;
            }

            // Если диапазон углов трека находится левее правой границы рабочего диапазона антенны
            unsigned int numFullTurnsRight = 0;
            if(limitsRadMax > maxAzimuth) {
                numFullTurnsRight = std::trunc((limitsRadMax - maxAzimuth) / AZIMUT_MAX);
                minAzimuth += numFullTurnsRight * AZIMUT_MAX;
                maxAzimuth += numFullTurnsRight * AZIMUT_MAX;

                //Если даже после поворотов мы не смогли уложить траекторию - выходим
                if(minAzimuth < limitsRadMin) return false;
            }

            int numFullTurns = numFullTurnsRight - numFullTurnsLeft;
            if (numFullTurns == 0) return true;

            for (auto &point : track) {
                point.setAzimut(point.az() + numFullTurns * AZIMUT_MAX);
            }
            return true;
        }
    }
}
