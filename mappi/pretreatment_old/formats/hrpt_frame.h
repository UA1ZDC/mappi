#ifndef HRPT_FRAME_H
#define HRPT_FRAME_H

//#include <stdint.h>

#define INVALID_CALIBR_VALUE -9999 //!< Значение калиброванных данных, означающее ошибку (помеху)
// #define NEED_VALID_READ_CNT 10  //!<Количество совпадений данных, по которому можно сделать вывод о их верности

// #define IMPROVE_SAT_ID      3   //!< Индекс спутника, с которого появляются HIRS/4 и MHS

// #define HRPT_FRAME_SIZE    (11090) //!<Размер major кадра HRPT
// #define EARTH_DATA_BEGIN   (750*2) //!<Смещение до начала Earth Data
// #define EARTH_DATA_OFFSET  ( EARTH_DATA_BEGIN + 100*2 ) //!< Смещение до следующего Earth Data

// #define HRPT_ID_BEGIN      (6*2)
// #define HRPT_ID_SIZE       (6*2)


#define AVHRR3_IR_CNT         (3) //!<Количество ИК каналов AVHRR/3
#define AVHRR3_VIZ_CNT        (3) //!<Количество каналов видимого диапазона AVHRR/3
//#define AVHRR3_CHAN_SAMPLES (2048)//!<Кол-во измерений одной строки сканирования AVHRR/3
#define AVHRR3_PRT_CNT        (4) //!<Количество PRT AVHRR/3

// #define AIP_MINOR_MAX      79  //!<Максимальное значение номера кадра AIP
// #define TIP_MINOR_MAX      319 //!<Максимальное значение номера кадра TIP

// #define HIRS_CHANNEL_COUNT 20 //!<Количество каналов HIRS
// #define MHS_CHANNEL_CNT    5  //!<Количество каналов MHS
// #define MHS_PRT_CNT        5  //!<<Количество PRT MHS

// #define AMSUA1_CHANNEL_CNT 13 //!<Количество каналов AMSU-A1
// #define AMSUA2_CHANNEL_CNT 2  //!<Количество каналов AMSU-A2
// #define AMSUB_CHANNEL_CNT  5  //!<Количество каналов AMSU-B
// #define AMSUA11_PRT_CNT (5)   //!<Количество PRT AMSU-A1-1
// #define AMSUA12_PRT_CNT (5)   //!<Количество PRT AMSU-A1-2
// #define AMSUA1_PRT_CNT (AMSUA11_PRT_CNT + AMSUA12_PRT_CNT) //!<Количество PRT AMSU-A1
// #define AMSUA2_PRT_CNT   7 //!<Количество PRT AMSU-A2
// #define AMSUB_PRT_CNT    7 //!<Количество PRT AMSU-B

// #define HRPT_SCAN_VEL (6.0) //!<скорость 6 строк/сек

// #define APT_CHANNEL_CNT 2


//! Работа с потоком HRPT
//namespace Hrpt {
  
  // //!Соответствует ID в формате HPRT
  // union HrptId {
  //   uint16_t M[2];
  //   //! Для NOAA HRPT
  //   struct { 
  //     uint16_t ch3x  :1; //!< 0=AVHRR Ch3B, 1=AVHRR Ch3A
  //     uint16_t input :1;
  //     uint16_t stabil:1;
  //     uint16_t addr  :4;
  //     uint16_t frame :2;
  //     uint16_t sync  :1;
  //   uint16_t  :6;
  //   uint16_t  :16;
  //   } S;
  // };
  
  // //! Соответствует ID и Time Code в формате HPRT
  // struct Header {
  //   HrptId id;
  //   uint16_t timeCode[4];
  //   Header() {
  //     memset(this, 0, sizeof(Header));
  //   }
  // };
  

  //! Описание формата кадра HRPT
  // struct FrameDescr {
  //   float velocity; //!< Скорость передачи данных (строк/сек)
  //   uint size;      //!< Размер кадра (байт)
  //   uint idBegin;   //!< Смещение от начала кадра до данных ID (байт)
  //   uint idOffset;  //!< Смещение от конца данных ID до начала данных ID следующего кадра
  //   FrameDescr(){
  //     velocity =0.;
  //     size = 0;
  //     idBegin = 0;
  //     idOffset = 0;
  //   };
  //   //  FrameDescr(float v, uint s, uint idB, uint idOff): velocity(v), size(s), idBegin(idB), idOffset(idOff) {}
  //   void set(float v, uint s, uint idB, uint idOff)
  //   { 
  //     velocity =v;
  //     size =s;
  //     idBegin = idB;
  //     idOffset = idOff;
  //   }
  //   bool isEmpty() {
  //     if (velocity == 0 || size == 0 || idOffset == 0) return true;
  //     return false;
  //   }
  // };


 
//};
  
#endif //HRPT_FRAME_H
