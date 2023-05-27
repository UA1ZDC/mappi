#include "modis.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/formats/stream_utility.h>
#include <mappi/pretreatment/frames/m_ccsds.h>


#define DAY_GROUP 0b000
#define NIGHT_GROUP 0b001

namespace {
    mappi::po::Instrument *
    createModis(const mappi::conf::Instrument &conf, const mappi::conf::InstrFormat &format, mappi::po::Frame *frame) {
        return new mappi::po::Modis(conf, format, frame);
    }

    static const bool res = mappi::po::singleton::SatFormat::instance()->
            registerInstrHandler(mappi::InstrFrameType(mappi::conf::kModisTerra, mappi::conf::kCadu), createModis);
    static const bool res1 = mappi::po::singleton::SatFormat::instance()->
            registerInstrHandler(mappi::InstrFrameType(mappi::conf::kModisAqua, mappi::conf::kCadu), createModis);
}

using namespace meteo;
using namespace mappi;
using namespace po;


Modis::Modis(const mappi::conf::Instrument &conf, const conf::InstrFormat &format, Frame *parent) :
        InstrumentTempl(conf, format),
        _conf(conf),
        _frame(parent) {
    _ch.resize(_conf.chan_cnt());

    _lines = 0;

}


Modis::~Modis() {
    clearChannels();
}

bool Modis::parse(const QList <QByteArray> &dataAll, QDateTime *dtStart, QDateTime *dtEnd) {


    int idx = -1;
    // QDateTime dt;
    // dt = *dtStart;

    bool setStart = false;


    int estimLength = 1354 * 10;//dataAll.size() / 300; //NOTE это примерно, число строк получится больше реального
    //var(estimLength);
    for (int ch = 0; ch < 2; ch++) {
        _ch[ch].setup(estimLength * 4, _conf.samples() * 4, ch + 1);
        _ch[ch].setLimits(0, 0xFFFF);
    }
    for (int ch = 2; ch < 7; ch++) {
        _ch[ch].setup(estimLength * 2, _conf.samples() * 2, ch + 1);
        _ch[ch].setLimits(0, 0xFFFF);
    }
    for (int ch = 7; ch < 38; ch++) {
        _ch[ch].setup(estimLength, _conf.samples(), ch + 1);
        _ch[ch].setLimits(0, 0xFFFF);
    }

    int night_count = 0;
    int day_count = 0;
    while (++idx < dataAll.size()) {
        const QByteArray &data = dataAll.at(idx);

        CCSDSHeader header;
        bool ok = header.parse(data, QDate(1958, 1, 1));
        if (!ok) {
            continue;
        }

        if (data.size() < header.size + 16) {
            warning_log << QObject::tr("small size");
            continue;
        }

        if (header.apid != 64) {
            //var(header);
            continue;
        }
        // var(data.size());

        //    var(header.apid);
        //debug


        ModisHeader modisHeader;
        modisHeader.parse(data.mid(6, data.size() - 6)); //6 - заголовок ccsdsd без secondary

        if (modisHeader.packet_type == DAY_GROUP) {
            if (data.size() - 6 < 636) {
                debug_log << QObject::tr("Skipping day packet of size %1. Should be larger than 616").arg(data.size());
                continue;
            }
            day_count++;
            processDayPacket(data.mid(6, data.size() - 6), header, modisHeader);

        } else if (modisHeader.packet_type == NIGHT_GROUP) {
            if (data.size() - 6 < 270) {
                debug_log << QObject::tr("Skipping night packet of size %1. Should be larger than 270").arg(data.size());
                continue;
            }

            night_count++;
            processNightPacket(data.mid(6, data.size() - 6), header, modisHeader);
        } else {
            //debug_log << QObject::tr("Cannot determine packet type: %1").arg(modisHeader.packet_type);
        }

        QDateTime curDt = modisHeader.dt();
        if (curDt.isValid()) {
            if (false == setStart) {
                setStart = true;
                *dtStart = curDt;
            }
            *dtEnd = modisHeader.dt();
        }
        //    debug_log << curDt << *dtStart << *dtEnd << setStart;


    }


    var(day_count);
    var(night_count);
    var(idx);

    //  _ch[idx].setup(_conf.channel(ch).samples(), _conf.channel(ch).samples(), ch + 1);

    for (int ch = 0; ch < 2; ch++) {
        _ch[ch].resize(_lines * 4 * _conf.samples() * 4);
    }

    for (int idx = 0; idx < 5; idx++) {
        int ch = idx + 2;
        _ch[ch].resize(_lines * 2 * _conf.samples() * 2);
    }

    for (int idx = 0; idx < 31; idx++) {
        int ch = idx + 2 + 5;
        _ch[ch].resize(_lines * _conf.samples());
    }


    //var(*dtStart);
    //var(*dtEnd);
    _frame->setDateTime(*dtStart, *dtEnd);

    return true;
}


QVector <ushort> Modis::bytesTo12bits(const QByteArray &bytes, unsigned int offset, unsigned int words_length) {
    QVector <ushort> words(417, 0);
    unsigned int byte_length = std::min(bytes.size(), 624) - offset;
    unsigned int bpos = offset;
    unsigned int wpos = 0;
    // Compute how many we can repack using the "fast" way
    unsigned int repack_fast = byte_length - (byte_length % 3);
    unsigned int repack_slow = byte_length % 3;

    // Repack what can be repacked fast
    for (unsigned int i = 0; i < repack_fast; i += 3){
        words[wpos++] =  bytes[bpos + 0] << 4 | bytes[bpos + 1] >> 4;
        words[wpos++] = (bytes[bpos + 1] & 0b1111) << 8 | bytes[bpos + 2];
        bpos += 3;
    }
    // Repack remaining using a slower method
    uint16_t shifter = 0;
    int inshifter = 0;
    for (unsigned int i = 0; i < repack_slow; i++){
        for (int b = 7; b >= 0; b--){
            shifter = (shifter << 1 | ((bytes[bpos] >> b) & 1)) & 4096;
            inshifter++;
            if (inshifter == 12)
            {
                words[wpos++] = shifter;
                inshifter = 0;
            }
        }
        bpos++;
    }
    words.resize(words_length);
    return words;
}

uint16_t Modis::compute_crc(const QVector<ushort> &data)
{
    uint16_t crc = 0;
    size_t size = data.size();
    for (size_t i = 0; i < size; i++) crc += data[i];
    crc >>= 4;
    return crc;
}


void Modis::processNightPacket(const QByteArray &data, const CCSDSHeader &ccheader, const ModisHeader &header) {
    // Filter out calibration packets
    if (header.type_flag == 1 || header.earth_frame_data_count > 1354 /*|| header.mirror_side > 1*/) {
        return;
    }

    int position = header.earth_frame_data_count;
    if (position == 0 && ccheader.seqFlag == 1 && lastScanCount != header.scan_count) {
        _lines += 10;
    }

    lastScanCount = header.scan_count;

    QVector<ushort> raw = bytesTo12bits(data, 0, 172);
    // Check CRC
    //if (compute_crc(raw) != raw.last()) return;

    // 28 1000m channels
    for (int i = 0; i < 16; i++) {
        _ch[14 + 7 + i].set((_lines + 9) * 1354 + position, raw[0   + i] << 4);
        _ch[14 + 7 + i].set((_lines + 8) * 1354 + position, raw[17  + i] << 4);
        _ch[14 + 7 + i].set((_lines + 7) * 1354 + position, raw[34  + i] << 4);
        _ch[14 + 7 + i].set((_lines + 6) * 1354 + position, raw[51  + i] << 4);
        _ch[14 + 7 + i].set((_lines + 5) * 1354 + position, raw[68  + i] << 4);
        _ch[14 + 7 + i].set((_lines + 4) * 1354 + position, raw[85  + i] << 4);
        _ch[14 + 7 + i].set((_lines + 3) * 1354 + position, raw[102 + i] << 4);
        _ch[14 + 7 + i].set((_lines + 2) * 1354 + position, raw[119 + i] << 4);
        _ch[14 + 7 + i].set((_lines + 1) * 1354 + position, raw[136 + i] << 4);
        _ch[14 + 7 + i].set((_lines + 0) * 1354 + position, raw[153 + i] << 4);
    }
}


void Modis::processDayPacket(const QByteArray &data, const CCSDSHeader &ccheader, const ModisHeader &header) {
    // Filter out calibration packets
    if (header.type_flag == 1 || header.earth_frame_data_count > 1354) {
        return;
    }

    int position = header.earth_frame_data_count - 1;
    if (position == 0 && ccheader.seqFlag == 1 && lastScanCount != header.scan_count) {
        _lines += 10;
    }
    lastScanCount = header.scan_count;
    QVector <ushort> raw = bytesTo12bits(data, 0, 416);

    // Check CRC
    //if (compute_crc(raw) != raw.last()) return;

    if (ccheader.seqFlag == 1) {
        // Contains IFOVs 1-5

        // Channel 1-2 (250m)
        for (uint c = 0; c < 2; c++) {
            for (int i = 0; i < 4; i++) {
                for (int y = 0; y < 4; y++) {
                    _ch[c].set(((_lines + 9) * 4 + (3 - y)) * (1354 * 4) + position * 4 + i, raw[0   + (c * 16) + (i * 4) + y] << 4);
                    _ch[c].set(((_lines + 8) * 4 + (3 - y)) * (1354 * 4) + position * 4 + i, raw[83  + (c * 16) + (i * 4) + y] << 4);
                    _ch[c].set(((_lines + 7) * 4 + (3 - y)) * (1354 * 4) + position * 4 + i, raw[166 + (c * 16) + (i * 4) + y] << 4);
                    _ch[c].set(((_lines + 6) * 4 + (3 - y)) * (1354 * 4) + position * 4 + i, raw[249 + (c * 16) + (i * 4) + y] << 4);
                    _ch[c].set(((_lines + 5) * 4 + (3 - y)) * (1354 * 4) + position * 4 + i, raw[332 + (c * 16) + (i * 4) + y] << 4);
                }
            }
        }

        // Channel 3-7 (500m)
        for (uint c = 0; c < 5; c++) {
            for (int i = 0; i < 2; i++) {
                for (int y = 0; y < 2; y++) {
                    _ch[c + 2].set(((_lines + 9) * 2 + (1 - y)) * (1354 * 2) + position * 2 + i, raw[32  + (c * 4) + (i * 2) + y] << 4);
                    _ch[c + 2].set(((_lines + 8) * 2 + (1 - y)) * (1354 * 2) + position * 2 + i, raw[115 + (c * 4) + (i * 2) + y] << 4);
                    _ch[c + 2].set(((_lines + 7) * 2 + (1 - y)) * (1354 * 2) + position * 2 + i, raw[198 + (c * 4) + (i * 2) + y] << 4);
                    _ch[c + 2].set(((_lines + 6) * 2 + (1 - y)) * (1354 * 2) + position * 2 + i, raw[281 + (c * 4) + (i * 2) + y] << 4);
                    _ch[c + 2].set(((_lines + 5) * 2 + (1 - y)) * (1354 * 2) + position * 2 + i, raw[364 + (c * 4) + (i * 2) + y] << 4);
                }
            }
        }

        // 28 1000m channels
        for (uint i = 0; i < 31; i++) {
            _ch[i + 7].set((_lines + 9) * 1354 + position, raw[52  + i] << 4);
            _ch[i + 7].set((_lines + 8) * 1354 + position, raw[135 + i] << 4);
            _ch[i + 7].set((_lines + 7) * 1354 + position, raw[218 + i] << 4);
            _ch[i + 7].set((_lines + 6) * 1354 + position, raw[301 + i] << 4);
            _ch[i + 7].set((_lines + 5) * 1354 + position, raw[384 + i] << 4);
        }
    } else if (ccheader.seqFlag == 2) {
        // Contains IFOVs 6-10

        // Channel 1-2 (250m)
        for (int c = 0; c < 2; c++) {
            for (int i = 0; i < 4; i++) {
                for (int y = 0; y < 4; y++) {
                    _ch[c].set(((_lines + 4) * 4 + (3 - y)) * (1354 * 4) + position * 4 + i, raw[0   + (c * 16) + (i * 4) + y] << 4);
                    _ch[c].set(((_lines + 3) * 4 + (3 - y)) * (1354 * 4) + position * 4 + i, raw[83  + (c * 16) + (i * 4) + y] << 4);
                    _ch[c].set(((_lines + 2) * 4 + (3 - y)) * (1354 * 4) + position * 4 + i, raw[166 + (c * 16) + (i * 4) + y] << 4);
                    _ch[c].set(((_lines + 1) * 4 + (3 - y)) * (1354 * 4) + position * 4 + i, raw[249 + (c * 16) + (i * 4) + y] << 4);
                    _ch[c].set(((_lines + 0) * 4 + (3 - y)) * (1354 * 4) + position * 4 + i, raw[332 + (c * 16) + (i * 4) + y] << 4);
                }
            }
        }

        // Channel 3-7 (500m)
        for (int c = 0; c < 5; c++) {
            for (int i = 0; i < 2; i++) {
                for (int y = 0; y < 2; y++) {
                    _ch[c + 2].set(((_lines + 4) * 2 + (1 - y)) * (1354 * 2) + position * 2 + i, raw[32  + (c * 4) + (i * 2) + y] << 4);
                    _ch[c + 2].set(((_lines + 3) * 2 + (1 - y)) * (1354 * 2) + position * 2 + i, raw[115 + (c * 4) + (i * 2) + y] << 4);
                    _ch[c + 2].set(((_lines + 2) * 2 + (1 - y)) * (1354 * 2) + position * 2 + i, raw[198 + (c * 4) + (i * 2) + y] << 4);
                    _ch[c + 2].set(((_lines + 1) * 2 + (1 - y)) * (1354 * 2) + position * 2 + i, raw[281 + (c * 4) + (i * 2) + y] << 4);
                    _ch[c + 2].set(((_lines + 0) * 2 + (1 - y)) * (1354 * 2) + position * 2 + i, raw[364 + (c * 4) + (i * 2) + y] << 4);
                }
            }
        }

        // 28 1000m channels
        for (int i = 0; i < 31; i++) {
            _ch[i + 7].set((_lines + 4) * 1354 + position, raw[52  + i] << 4);
            _ch[i + 7].set((_lines + 3) * 1354 + position, raw[135 + i] << 4);
            _ch[i + 7].set((_lines + 2) * 1354 + position, raw[218 + i] << 4);
            _ch[i + 7].set((_lines + 1) * 1354 + position, raw[301 + i] << 4);
            _ch[i + 7].set((_lines + 0) * 1354 + position, raw[384 + i] << 4);
        }
    }
}


float Modis::fovStep(int channel) const {
    if (channel < 2) return 0.020315; //_conf.fovstep() / 4;
    if (channel < 7) return 0.040630; //_conf.fovstep() / 2;
    return _conf.fovstep();
}

float Modis::velocity(int channel) const {
    if (channel < 3) return _conf.velocity() * 4;
    if (channel < 8) return _conf.velocity() * 2;
    return _conf.velocity();
}
