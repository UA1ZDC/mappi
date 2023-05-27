#include "vcduel.h"


#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/pretreatment/formats/satformat.h>

#include <cross-commons/debug/tlog.h>

#include <qbytearray.h>
#include <qdatetime.h>
#include <qdir.h>

using namespace mappi;
using namespace po;

#define NEED_VALID_READ_CNT 6  //!< Количество совпадений данных, по которому можно сделать вывод о их верности
#define CCSDS_HEADER_LENGTH 6

namespace {
  mappi::po::Frame* createVcduElectro(const mappi::conf::Frame& conf, Frame* parent)
  {
    return new mappi::po::VcduElectro(conf, parent);
  }

  static const bool res = mappi::po::singleton::SatFormat::instance()->registerFrameHandler(mappi::conf::kVcduElectro, createVcduElectro);
}

static bool first = true; //TODO временно

VcduElectro::VcduElectro(const conf::Frame& conf, Frame* parent): 
  Frame(conf, false, parent)
{
}


  // QDir dir;
  // const QList<int>& id = procid();
  // var(id);
  // for (auto idx: id) {
  //   debug_log << "remove" << Frame::path() + "_" + satName().remove(' ') + "_vcdu." + QString::number(idx);
  //   dir.remove(Frame::path() + "_" + satName().remove(' ') + "_vcdu." + QString::number(idx));
  // }


VcduElectro::~VcduElectro()
{
  //TODO remove files
  //debug_log << "remove tmp files";
  
  QDir dir;
 
  QMapIterator<int, QString> it(_files);
  while (it.hasNext()) {
    it.next();
    dir.remove(it.value());
    debug_log << "remove" << it.value();
  }
}


//TODO надо бы проверку для достоверности vcid

int VcduElectro::processLPart(const QDateTime& dt)
{
  Q_UNUSED(dt);
  //TODO path
  QString apath =// Frame::path() + dt.toString("yyyyMMddhhmm")  + "_" + satName().remove(' ') + "_vcdu.";
    Frame::path() + "_" + satName().remove(' ') + "_vcdu.";

  const QByteArray& data = Frame::data();
  
  int32_t pos = 0;//findStartCCSDS(data); 
  //int32_t posValid = pos;
  //  int vcducnt = 0;

  var(pos);
  var(data.size());

  if (pos == data.size()) {
    return 0;
  }

  QMap<int, int> vcidal;
  QMap<int, QByteArray> vcidData;
  //  const QList<int>& id = procid();

  while ((pos + format().size()) <=  data.size()) {
    uint8_t vcid = (uint8_t)data[pos + 1] & 0x3F;
    //if (id.contains(vcid)) {
      vcidal[vcid] +=1;
      vcidData[vcid].append(data.mid(pos, format().size()));
      //}
    
    pos += format().size();
  }

  QMapIterator<int, QByteArray> it(vcidData);
  while (it.hasNext()) {
    it.next();
    QString name = apath + QString::number(it.key());
    var(name);
    _files.insert(it.key(), name);
    QFile file(name);
    if (first && file.exists()) {
      error_log << QObject::tr("Необходимо удалить старые файлы");
      return false;
    }
    
    file.open(QIODevice::Append);
    file.write(it.value());
    file.close();
    first = false;
  }

  var(vcidal);
 
  return pos;
}


// bool VcduElectro::setFrameDataL(Instrument* instr, const conf::InstrFormat& instrFormat)
// {
//   if (0 == instr || false == instrFormat.has_id()  ||
//       instrFormat.sect_size() != 1) {
//     error_log << QObject::tr("Ошибка описания VCDU пакета Electro");
//     return false;
//   }

//   // _files.insert(21, "/home/maria/satdata/pretreatment/_GOMS-3_vcdu.21");
//   //_files.insert(26, "/home/maria/satdata/pretreatment/_GOMS-3_vcdu.26");
//   // _files.insert(38, "/home/maria/satdata/pretreatment/_GOMS-3_vcdu.38");
//   // _files.insert(41, "/home/maria/satdata/pretreatment/_GOMS-3_vcdu.41");
//   var(_files);
  
//   QString name = _files.value(instrFormat.id());
//   QFile file(name);
//   if (!file.open(QIODevice::ReadOnly)) {
//     error_log << QObject::tr("Ошибка открытия файла") << name;
//     return false;
//   }
  
//   QByteArray data = file.readAll();
//   file.close();
//   var(data.size());

//   int pos = 0;
//   while ((pos + format().size()) <=  data.size()) {
//     int vcducnt = ((uint8_t)data[pos + 2] << 16) + ((uint8_t)data[pos + 3] << 8) + ((uint8_t)data[pos + 4]);
    
//     instr->addNewFrame(vcducnt, data.mid(pos + instrFormat.sect(0).start(), instrFormat.sect(0).size()));
    
//     pos += format().size();
//   }

//   return true;
// }


//noaa20
bool VcduElectro::setFrameDataL(Instrument* instr, const conf::InstrFormat& instrFormat)
{
  if (0 == instr || false == instrFormat.has_id()  ||
      instrFormat.sect_size() != 1) {
    error_log << QObject::tr("Ошибка описания VCDU пакета Electro");
    return false;
  }

  // _files.insert(21, "/home/maria/satdata/pretreatment/_GOMS-3_vcdu.21");
  //_files.insert(26, "/home/maria/satdata/pretreatment/_GOMS-3_vcdu.26");
  // _files.insert(38, "/home/maria/satdata/pretreatment/_GOMS-3_vcdu.38");
  // _files.insert(41, "/home/maria/satdata/pretreatment/_GOMS-3_vcdu.41");
  var(_files);
  
  QString name = _files.value(instrFormat.id());
  QFile file(name);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла") << name;
    return false;
  }
  
  QByteArray data = file.readAll();
  file.close();
  var(data.size());

  int32_t pos = findStartCCSDS(data); //обязательно для китайца
  int cnt = 0;
  int vcducnt = 0;

  var(pos);
  var(data.size());

  if (pos == data.size()) {
    return 0;
  }

  // QFile file("/tmp/vcdu.metop");
  // file.open(QIODevice::WriteOnly);
  // file.write(data);
  // file.close();

  // var(data.size());
  // var(pos);

  //TODO
  if (instr->type().first == mappi::conf::kMsuGs) {
    int pos = 0;
    while ((pos + format().size()) <=  data.size()) {
      int vcducnt = ((uint8_t)data[pos + 2] << 16) + ((uint8_t)data[pos + 3] << 8) + ((uint8_t)data[pos + 4]);
      instr->addNewFrame(vcducnt, data.mid(pos + instrFormat.sect(0).start(), instrFormat.sect(0).size()));
      pos += format().size();
    }
    return true;
  }


  
  bool isStarted = false;

  
  while ((pos + format().size()) <=  data.size()) {

    // debug_log << ((uint8_t)data[pos + 0] >> 6)
    // 	      << ((((uint8_t)data[pos + 0]&0x3f) << 2) + ((uint8_t)data[pos + 1] >> 6))
    // 	      << ((uint8_t)data[pos + 1]&0x3f) 
    // 	      << ((uint8_t)data[pos + 2] << 16) + ((uint8_t)data[pos + 3] << 8) + ((uint8_t)data[pos + 4])
    // 	      << ((uint8_t)data[pos + 5])  	     
    // 	      << ((((uint8_t)data[pos + 6] << 8) + (uint8_t)data[pos + 7]) & 0x7ff);
    
    uint8_t vcid = (uint8_t)data[pos + 1] & 0x3F;
    // debug_log << "vcid" << vcid;
    
    if (vcid == instr->format().id()) {
      
      // debug_log << "cnt" << ((uint8_t)data[pos + 2] << 16) + ((uint8_t)data[pos + 3] << 8) + ((uint8_t)data[pos + 4])
      // 		<< "offset" << ((((uint8_t)data[pos + 6] << 8) + (uint8_t)data[pos + 7]) & 0x7ff);

      vcducnt = ((uint8_t)data[pos + 2] << 16) + ((uint8_t)data[pos + 3] << 8) + ((uint8_t)data[pos + 4]);

      //noaa20 нет vcdu insert zone
      int32_t offset = ((((uint8_t)data[pos + 6] << 8) + (uint8_t)data[pos + 7]) & 0x7ff); //сдвиг до начала заголовка
      
      if (offset == 0x7ff) {	
	//	var(offset);
	if (instr->type().first == mappi::conf::kMsuMrLo ||
	    instr->type().first == mappi::conf::kVirr) {
	  instr->addNewFrame(vcducnt, data.mid(pos + instrFormat.sect(0).start(), instrFormat.sect(0).size()));
	} else {
	  if (isStarted) {
	    instr->addFrame(vcducnt, data.mid(pos + instrFormat.sect(0).start(), instrFormat.sect(0).size()));
	  }
	}
      } else {
	
	//debug_log << "tail " << offset;
	if (isStarted) {
	  instr->addFrame(vcducnt, data.mid(pos + instrFormat.sect(0).start(), offset));
	}

	isStarted = true;
	
	int32_t pp = pos + instrFormat.sect(0).start() + offset;
	while (pp < pos + instrFormat.sect(0).start() + instrFormat.sect(0).size()) {
	  if (pp + CCSDS_HEADER_LENGTH >= pos + instrFormat.sect(0).start() + instrFormat.sect(0).size()) {
	    //остался маленький кусочек, меньше заголовка. считываем, что есть
	    //debug_log << "start" << (instrFormat.sect(0).size() + pos + instrFormat.sect(0).start() ) - pp;
	    instr->addNewFrame(vcducnt, data.mid(pp, (instrFormat.sect(0).size() + pos + instrFormat.sect(0).start() ) - pp));
	    pp += (instrFormat.sect(0).size() + pos + instrFormat.sect(0).start() ) - pp;
	  } else {
	    int32_t length = (((uint8_t)data[pp + 4] << 8) + (uint8_t)data[pp + 4 + 1]) + 1; //в поле хранится число байт минус 1
	    //debug_log << "length = " << length;
	    if (pp + length >= pos + instrFormat.sect(0).start() + instrFormat.sect(0).size()) {
	      //размер ccsds больше куска, считываем до конца
	      //debug_log << "part" << (instrFormat.sect(0).size() + pos + instrFormat.sect(0).start() ) - pp;
	      //instr->addNewFrame(vcducnt, data.mid(pp, instrFormat.sect(0).size() - offset));
	      instr->addNewFrame(vcducnt, data.mid(pp, (instrFormat.sect(0).size() + pos + instrFormat.sect(0).start() ) - pp));
	      pp += (instrFormat.sect(0).size() + pos + instrFormat.sect(0).start() ) - pp;
	    } else {
	      //несколько ccsds, считываем текущий, сдвигаемся на начало следующего
	      //debug_log << "all  " << length + CCSDS_HEADER_LENGTH;
	      instr->addNewFrame(vcducnt, data.mid(pp, length + CCSDS_HEADER_LENGTH));
	      pp += length + CCSDS_HEADER_LENGTH;
	    }

	  }
	  cnt++;
	}
      
	// if (pp != pos + instrFormat.sect(0).start() + instrFormat.sect(0).size()) {
	// 	debug_log << "pp end" << pp << pos + instrFormat.sect(0).start() + instrFormat.sect(0).size();
	// }
      }
    } 
    
    pos += format().size();
  }

  return true;
}

int64_t VcduElectro::findStartCCSDS(const QByteArray& data)
{
  int check=0;   //количество совпадений ожидаемых и фактических значений данных

  int32_t pos = 0;
  uint8_t vers = (uint8_t)data[pos + 0] >> 6;
  uint8_t scid = ((((uint8_t)data[pos + 0]&0x3f) << 2) + ((uint8_t)data[pos + 1] >> 6));
  uint8_t vcid = (uint8_t)data[pos + 1] & 0x3F;
  uint32_t counter = ((uint8_t)data[pos + 2] << 16) + ((uint8_t)data[pos + 3] << 8) + ((uint8_t)data[pos + 4]);
  pos += format().size();
 
  QMap<uint8_t, uint32_t> vcidcnt;
  // if (vcid != 0) {
  vcidcnt.insert(vcid, counter);
  //}

  while (check < NEED_VALID_READ_CNT) {
    if (pos + 5 >=  data.size()) {
      return data.size();
    }

    vcid = (uint8_t)data[pos + 1] & 0x3F;
    counter = ((uint32_t)(uint8_t)data[pos + 2] << 16) + ((uint8_t)data[pos + 3] << 8) + ((uint8_t)data[pos + 4]);

    //debug_log << scid << vcid << counter << vers << flag;

    if ((vers == (uint8_t)data[pos + 0] >> 6) &&
	(scid == (((uint8_t)data[pos + 0]&0x3f) << 2) + ((uint8_t)data[pos + 1] >> 6)) &&
	//(counter + 1 == ((uint32_t)(uint8_t)data[pos + 2] << 16) + ((uint8_t)data[pos + 3] << 8) + ((uint8_t)data[pos + 4])) &&
	(!vcidcnt.contains(vcid) || vcidcnt.value(vcid) + 1 == counter)
	) {
      check++;
    } else {
      check = 0;
    }

    vers = (uint8_t)data[pos + 0] >> 6;
    scid = ((((uint8_t)data[pos + 0]&0x3f) << 2) + ((uint8_t)data[pos + 1] >> 6));
 
    //  if (vcid != 0) {
    vcidcnt[vcid] = counter;
    //}

    pos += format().size();
  }

  // var(vcidcnt);

  if (check < NEED_VALID_READ_CNT) return data.size();

  return pos - (NEED_VALID_READ_CNT + 1) * format().size();
}
