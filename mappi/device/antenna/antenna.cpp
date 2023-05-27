#include "antenna.h"
#include "vkaprotocol.h"
#include "niitvprotocol.h"
#include "anglespeedprotocol.h"


namespace mappi {

    namespace antenna {
        Antenna::Antenna(QObject *parent /*=*/) :
                QObject(parent),
                protocol_(nullptr),
                transport_(nullptr),
                azimutCorrect_(0),
                txTimeout_(0),
                azimut_(Drive::makeAzimut()),
                elevat_(Drive::makeElevat()) {
        }

        Antenna::~Antenna() {
            delete protocol_;
            protocol_ = nullptr;
        }

        bool Antenna::init(const Configuration &conf) {
            azimutCorrect_ = conf.azimutCorrect;
            txTimeout_ = conf.serialPort.txTimeout;

            profile_ = conf.profile;
            switch (profile_.type) {
                case conf::kVkaAntenna :
                    protocol_ = new VkaProtocol;
                    break;

                case conf::kNiitvAntenna :
                    protocol_ = new NiitvProtocol;
                    break;

                case conf::kAngleSpeedAntenna :
                    protocol_ = new AngleSpeedProtocol;
                    break;

                default :
                    return false;
            }

            info_log << protocol_->type();

            transport_ = new Transport(protocol_, this);
            QObject::connect(transport_, &Transport::recv, this, &Antenna::onRecv);

            return transport_->init(conf);
        }

        bool Antenna::open() {
            return transport_->open();
        }

        void Antenna::close() {
            azimut_ = Drive::makeAzimut();
            elevat_ = Drive::makeElevat();
            transport_->close();
        }

        void Antenna::snapshot(Drive *azimut, Drive *elevat, Feedhorn *feedhorn) {
            if (azimut) {
                *azimut = azimut_;
                // if (withoutCorrect == false)
                //   azimut->self += azimutCorrect_;
            }

            if (elevat) *elevat = elevat_;

            if (feedhorn) *feedhorn = feedhorn_;
        }

        //alredyCorrect - сейчас всегда true. В траектории уже скорректировано, в тесте антенны работаем относительно нуля антенны
        void Antenna::setPosition(MnSat::TrackTopoPoint point, bool alreadyCorrect){
            float az_deg = point.getAzimut().value().toDeg();
            float el_deg = point.getElevation().value().toDeg();
            if (!profile_.isValidRange(az_deg, el_deg)) {
                error_log << QObject::tr("Превышение диапазона: [%1, %2]")
                        .arg(az_deg)
                        .arg(el_deg);
                return;
            }

            azimut_.dst = az_deg + (alreadyCorrect ? 0 : azimutCorrect_);
            point.setAzimut(MnSat::Angle::fromDeg(azimut_.dst));
            elevat_.dst = el_deg;

            if (protocol_->isAsyncExec()) {
                if (!transport_->send(protocol_->packAzimut(Protocol::DIRECT, point))) {
                    azimut_.state = Drive::UNKNOWN;
                    return;
                }

                lock_.wait(txTimeout_);
                if(!transport_->send(protocol_->packElevat(Protocol::DIRECT, point))){
                    elevat_.state = Drive::UNKNOWN;
                    return;
                };
                return;
            }
            if (!transport_->send(protocol_->pack(Protocol::DIRECT, point))){
                azimut_.state = Drive::UNKNOWN;
            }
        }

        void Antenna::stop() {
            MnSat::TrackTopoPoint empty_point;
            if (protocol_->isAsyncExec()) {
                if (!transport_->send(protocol_->packAzimut(Protocol::STOP, empty_point))) {
                    azimut_.state = Drive::UNKNOWN;
                    return;
                }

                lock_.wait(txTimeout_);
                transport_->send(protocol_->packElevat(Protocol::STOP, empty_point));
            } else if (!transport_->send(protocol_->pack(Protocol::STOP, empty_point)) )
                azimut_.state = Drive::UNKNOWN;
        }

        void Antenna::move(float dsa, float dse) {
            // WARNING непонятна разница между move и setPosition как для Vka так и для Niitv
            warning_log << QObject::tr("move не поддерживается");

            if (!profile_.isValidDSA(dsa) || !profile_.isValidDSE(dse)) {
                return;
            }
        }

        void Antenna::travelMode() {
            MnSat::TrackTopoPoint empty_point;

            if (profile_.hasTravelMode)
                transport_->send(protocol_->pack(Protocol::TRAVEL_MODE, empty_point));
            else
                warning_log << QObject::tr("travel mode не поддерживается");
        }

        void Antenna::feedhorn(bool turnOn) {
            if (!profile_.hasFeedhornControl) {
                warning_log << QObject::tr("Управление облучателем не поддерживается");
                return;
            }

            if (feedhorn_.isTurnOn() == turnOn)
                return;

            //передается на месте азимута
            MnSat::TrackTopoPoint feedhorn_point(turnOn ? 0.01 : 0.00, 0);
            if (transport_->send(protocol_->pack(Protocol::FEEDHORN, feedhorn_point)))
                feedhorn_.turnOn(turnOn);
            else
                azimut_.state = Drive::UNKNOWN;
        }

        void Antenna::feedhornOutput(Feedhorn::output_t n, bool turnOn) {
            if (!profile_.hasFeedhornControl) {
                warning_log << QObject::tr("Управление дополнительными выходами облучателя не поддерживается");
                return;
            }

            if (feedhorn_.isActiveOutput(n) == turnOn)
                return;

            MnSat::TrackTopoPoint feedhorn_point(turnOn ? 0.01 : 0.00, (n / 100.));
            if (0 == transport_->send(protocol_->pack(Protocol::FEEDHORN_OUTPUT, feedhorn_point)))
                azimut_.state = Drive::UNKNOWN;
            else
                feedhorn_.setActiveOutput(n, turnOn);
        }

        void Antenna::onRecv(const QByteArray &buf) {
            QString error;
            if (!protocol_->unpack(buf, &azimut_, &elevat_, &error))
                error_log << error;
        }

    }

}
