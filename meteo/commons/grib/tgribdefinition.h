#ifndef TGRIB_DEFINITION
#define TGRIB_DEFINITION

#include <google/protobuf/message.h>

namespace grib {
  //! Источник сохраняемой информации (номер издания GRIB)
  enum SourceType {
    kGribUnk = 0,
    kGrib1  = 1, //!< GRIB
    kGrib2  = 2, //!< GRIB2
  };

  //! Состояние
  enum StatType {
    kReceived = 0, //!< Количество полученных сводок
    kDecoded  = 1, //!< Количество раскодированных
    kSaveOk   = 2, //!< Успешное сохранение
    kSaveErr  = 3, //!< Ошибка сохранения            
    kRepeated = 4, //!< Повторная сводка
  };

  struct TProductDefinition {
    uint16_t type; //!< Номер образца определения продукции (т. 4.0)
    google::protobuf::Message* definition; //!< Разобранный образец в соотв-ии с type
    TProductDefinition():type(0xffff),definition(0) {}
    ~TProductDefinition() { if (!definition) delete definition; definition = 0; }
  };
  struct TGridDefinition {
    uint16_t type; //!< Номер шаблона сетки ш.3.N (0xffff - нет сетки; 0x8000 - предопределена центром)
    google::protobuf::Message* definition; //!< Разобранные данные в соответствии с type
    TGridDefinition() { definition = 0; type = 0xffff; }
    ~TGridDefinition() { if (!definition) delete definition; definition = 0; }
  };
}

#endif
