#ifndef MAPPI_PRETREATMENT_FRAMES_CCSDS_H
#define MAPPI_PRETREATMENT_FRAMES_CCSDS_H

#include <cross-commons/debug/tlog.h>

#include <stdint.h>
#include <QDateTime>
#include <cstdint>
#include <cmath>

#define HEADER_SIZE 6


class QByteArray;

#define CCSDS_PRIMARY_HEADER_LENGTH 6

namespace mappi {
  namespace po {
    
    struct CCSDSHeader {
      uint8_t size = HEADER_SIZE;
      uint8_t raw[HEADER_SIZE];
      // primary header
      uint8_t version = 0;
      bool indicator = 0;
      bool hasSecondary = false;
      uint16_t apid = 0;
      uint8_t seqFlag = 0;
      uint16_t cnt = 0;
      uint16_t length = 0;
      // secondary header
      QDateTime dts;
      uint8_t pckCnt = 0;
      
      bool parse(const QByteArray& data, const QDate& date);      
    };

    TLog& operator<<(TLog& out, const CCSDSHeader& header);
    
    bool getCCSDSLength(const QByteArray& data, uint16_t* length);
    bool getCCSDSLength(const QByteArray& data, uint32_t pos, uint16_t* length);

    int32_t getCCSDSCnt(const QByteArray& data);
  }
}

#endif
