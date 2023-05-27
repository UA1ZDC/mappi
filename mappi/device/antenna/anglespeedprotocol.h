#pragma once

#include "vkaprotocol.h"


namespace mappi {

    namespace antenna {

        class AngleSpeedProtocol : public VkaProtocol {
        public :
            AngleSpeedProtocol();

            virtual ~AngleSpeedProtocol();
            virtual bool hasVelocity() const { return true; }
            virtual bool hasTime() const { return true; }
            virtual const char *type() const { return "angle velocity protocol"; }

            virtual QByteArray pack(int id, const MnSat::TrackTopoPoint& point);

        private :
            QString velocityToString(const MnSat::AngleWithVelocity& angle);
            quint8 seq_;
        };

    }

}
