#include "msugs.h"
#include <mappi/pretreatment/images/filters.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/debug/tmap.h>
#include <cross-commons/app/paths.h>

#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/formats/stream_utility.h>
#include <mappi/pretreatment/frames/frametypes.h>

#include <qimage.h>

const QByteArray kMsuGsSync = QByteArray::fromHex("0218A7A392DD9ABF");
#define MSUGS_FRAME_SIZE 15210
#define MSUGS_DATA_OFFSET 200
#define MSUGS_DATA_SIZE  15000
#define MSUGS_VCID_IR 41

static const TMap<int, int> vcid2ch = TMap<int, int>() 
  << QPair<int, int>(21, 0)
  << QPair<int, int>(26, 1)
  << QPair<int, int>(38, 2);


struct VisElMargins {
  int overlap = 0;   //!< перекрытие правой и левой половинок
  int halfshift = 0; //!< врзвышение правой половинки над левой
  int top = 0;       //!< градиент плюс пустота сверху
  int right = 0;     //!< пустота справа

  VisElMargins() {}
  VisElMargins(int ol, int hs, int t, int r) :
    overlap(ol), halfshift(hs), top(t), right(r)
  {}
};
//плюс - вниз
//границы для исходного до переворота
static const TMap<int, VisElMargins> margins = TMap<int, VisElMargins>()
     << QPair<int, VisElMargins>(21, VisElMargins(18,  6, 1138 - 2 , 270 + 3))
     << QPair<int, VisElMargins>(26, VisElMargins(57 - 1, 11 , 4702, 220))
     << QPair<int, VisElMargins>(38, VisElMargins(35,  4, 2923 - 3, 246 + 6));

namespace {
  mappi::po::Instrument* createMsuGs(const mappi::conf::Instrument& conf, const mappi::conf::InstrFormat& format, mappi::po::Frame* frame)
  {
    return new mappi::po::MsuGs(conf, format, frame);
  }

  static const bool res = mappi::po::singleton::SatFormat::instance()->
	registerInstrHandler(mappi::InstrFrameType(mappi::conf::kMsuGs, mappi::conf::kCaduElectro), createMsuGs);
}

using namespace meteo;
using namespace mappi;
using namespace po;

MsuGs::MsuGs(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent):
  InstrumentTempl(conf, format),
  _conf(conf),
  _frame(parent)
{ 
  _ch.resize(_conf.chan_cnt());
}

MsuGs::~MsuGs()
{
}

bool MsuGs::parse(const QByteArray& data, QDateTime* dtStart, QDateTime*)
{
  QDateTime dt = *dtStart;
  
  trc;
  if (format().id() == MSUGS_VCID_IR) {
    return parseIR(data, dt);
  }

  
  int ch = vcid2ch.value(format().id());
   
  int pos = data.indexOf(kMsuGsSync, 0);
  if (-1 == pos) {
    error_log << QObject::tr("Не найдена синхрострока");
    return false;
  }

  // QFile file1("msugs.38.data");
  // file1.open(QIODevice::Append);
  // file1.write(data);
  // file1.close();
    
  var(data.size());
  var(pos);
  
  QByteArray cif;
  while (pos + MSUGS_FRAME_SIZE <= data.size()) {
    cif.append(data.mid(pos + MSUGS_DATA_OFFSET, MSUGS_DATA_SIZE));
    pos += MSUGS_FRAME_SIZE;
  }
  
  int shift_bit = 0;
  int remain;
  QVector<ushort> raw;
  bool ok = meteo::cif2raw(cif, shift_bit, &raw, &remain, false);
  var(remain);
  
  cif.clear();
  var(ok);
  var(raw.size());

  
  const VisElMargins& m = margins.value(format().id());

  int dsamples = 12000;
  uint length = m.top + _conf.channel(ch).samples();
  var(length);
  raw.resize(length * dsamples);//TODO можно ещё в cif не тащить пустой кусок

  _ch[0].setup(_conf.channel(ch).samples(), _conf.channel(ch).samples(), ch + 1);
  _ch[0].setLimits(0, 0x3FF);
  
  debug_log << "channel" << format().id() << ch;
  
  // for (int idx = 0, chi = 0; idx < raw.size(); idx++, chi++) {
  //   _ch[ch]->set(chi, raw.at(idx));
  // }

  int chidx = 0;
  int half = dsamples / 2;

  for (int row = 0; row < _conf.channel(ch).samples(); row++) {
    for (int col = m.right; col < half - m.overlap; col++) {
      int idx = dsamples * (length - 1 - row) + (dsamples - 1) - col*2 - 1; 
      if (idx + 1 < raw.size() && idx + 1 > 0) {
	_ch[0].set(chidx, raw.at(idx + 1));
      }
      chidx++;
    }
    for (int col = 0; col <  _conf.channel(ch).samples() - (half - m.overlap - m.right); col++) {
      int idx = dsamples * (length - 1 - row + m.halfshift) + (dsamples - 1) - col*2 - 1 - 2; //"-2" - образаем один байт на стыке
      if (idx < raw.size() && idx > 0) {
	_ch[0].set(chidx, raw.at(idx));
      }
      chidx++;
    }
  }
  
 
  debug_log << "size" << raw.size() << chidx;

  
  return true;
}


bool MsuGs::parseIR(const QByteArray& data, const QDateTime& )
{
  int syncpos = data.indexOf(kMsuGsSync, 0);
  if (-1 == syncpos) {
    error_log << QObject::tr("Не найдена синхрострока");
    return false;
  }

  uint64_t pos = syncpos;
  var(pos);

  // for (int idx = 0; idx < 10; idx++) {
  //   syncpos = data.indexOf(kMsuGsSync, syncpos + kMsuGsSync.size());
  //   var(syncpos);
  // }
  
  bool ok = true;
  int start  = 140 + pos;
  int length = 240;
  int offset = length*6 + 140;
  for (int idx = 0; idx < 7; idx++) {
    // if (idx == 0)
    {
      ok &= parseIR(data, idx, start, length, offset);
    }
    start += length;
  }

  return ok;
}

bool MsuGs::parseIR(const QByteArray& data, int chidx, uint64_t start, uint64_t size, uint64_t offset)
{
  QByteArray cif;
  uint64_t pos = start;

  var((uint64_t)data.size());
  
  while (pos + size <= (uint64_t)data.size() / 4) { //TODO только один из 4х
    cif.append(data.mid(pos, size));
    pos += size + offset;
    //var(pos);
  }
  var(cif.size());

  // QFile file("msugs.41.3.cif");
  // file.open(QIODevice::WriteOnly);
  // file.write(cif);
  // file.close();
  
  int shift_bit = 0;
  int remain;
  QVector<ushort> raw;
  bool ok = meteo::cif2raw(cif.mid(0, cif.size()), shift_bit, &raw, &remain, false);

  cif.clear();

  // QFile file1("msugs.41.3.raw");
  // file1.open(QIODevice::WriteOnly);
  // for (int id = 0; id < raw.size(); id++) {
  //   file1.putChar((char)(raw.at(id) >> 8));
  //   file1.putChar(char(raw.at(id) & 0xff));
  // }
  // file1.close();

  
  var(ok);
  var(raw.size());

  int partsCnt = 16;
  int hole = 20;
  int ch = chidx + 3;
  uint samples = (183 + hole)*partsCnt;//27648; //13824; //_conf.channel(ch).samples();
  uint length = raw.size() / (192*18) /2; //ширина полосы * кол-во и одна из половинок. Должно получиться 3400
  var(length);

  _ch[chidx].setup(length, samples, ch + 1);
  _ch[chidx].setLimits(0, 0x3FF);
  
  debug_log << "channel" << format().id() << ch;

  uint64_t rows = 2*length; //6800;
  uint columns = 192;
  uint64_t idx = 0;
  uint64_t parts = raw.size() / columns /rows; // количество полос
  var(parts);

  int shift = 0;//60;

  for (uint pp = 1; pp < parts - 1; pp++) {
    for (uint rrow = 0; rrow < rows; rrow++) {
      int slope = 0;
      if (pp <= 8) {
      	slope = 0;
      } else if (pp == 9) {
      	slope = 8;
      }

      int ddd = 8 + pp*2;
      if (pp >= 10) {
      	ddd = 8 + (parts - pp)*2;
      }
      if (pp >= 10) {
      	slope = 192 / ddd + 1;
      }
      
      for (uint rcol = 5; rcol < columns - 4; rcol ++) {
  	uint64_t ridx = pp*rows*columns + rrow*columns + rcol;
  	int crow = 0;
  	int ccol = 0;
	int cshift = 0;


	if (pp < 9) {
	  if ((rcol - 5 - 3) % ddd == 0) {
	    slope += 1;
	    // if (rrow == 1) {
	    //   debug_log << pp << ddd << slope;
	    // }
	  }
	} else if (pp == 9) {
	 
	} else if (pp > 9) {
	  if ((rcol - 5 - 3) % ddd == 0) {
	    slope -= 1;
	    // if (rrow == 1) {
	    //   debug_log << pp << ddd << slope;
	    // }

	  }
	}
	//slope = 0;
  	if (rrow < 3400) {
	  ccol = samples - ((rcol - 5) + (columns - 9 + hole)*(pp-1) + cshift - 100) - 1;
	  //crow = rrow + shift + slope;
	  crow = 3399 - rrow + shift - slope;
	} else {
	  ccol = samples - ((rcol - 5) + (columns - 9 + hole)*(pp-1) + /*100 +*/ cshift) - 1;
	  //crow = 6799 - rrow + 20 + shift + slope;
	  crow = rrow - partsCnt + shift - slope - 3400;
  	}

	//	if ((/*pp >=1 &&*/ pp <= 6) || (pp == 7 && rrow < 3400))
	  {
	    if (crow >= 0 && (uint)crow < length)
	    {
	      idx = crow*samples + ccol;
	      _ch[chidx].set(idx, raw.at(ridx));
	    }
	  }
      }
	
      }


    //debug_log << pp << shift;
    }
 
  raw.clear();

    
  return true;
}

// //исходные
// bool MsuGs::parseIR(const QByteArray& data, int chidx, uint64_t start, uint64_t size, uint64_t offset)
// {
//   QByteArray cif;
//   uint64_t pos = start;

//   var((uint64_t)data.size());
  
//   while (pos + size <= (uint64_t)data.size()) {
//     cif.append(data.mid(pos, size));
//     pos += size + offset;
//     //var(pos);
//   }
//   var(cif.size());
 
//   int shift_bit = 0;
//   int remain;
//   QVector<ushort> raw;
//   bool ok = meteo::cif2raw(cif.mid(0, cif.size()), shift_bit, &raw, &remain, false);

//   cif.clear();

//   // QFile file1("msugs.41.3.raw");
//   // file1.open(QIODevice::WriteOnly);
//   // for (int id = 0; id < raw.size(); id++) {
//   //   file1.putChar((char)(raw.at(id) >> 8));
//   //   file1.putChar(char(raw.at(id) & 0xff));
//   // }
//   // file1.close();

  
//   var(ok);
//   var(raw.size());

//   if (0 != _ch[chidx]) {      
//     delete _ch[chidx];
//   }
//   int hole = 20;
//   int ch = chidx + 3;
//   uint samples = (183 + hole)*18 * 4;//27648; //13824; //_conf.channel(ch).samples();
//   uint length = raw.size() / (192*18 *4);
//   var(length);

//   _ch[chidx] = new Channel(length, samples, ch + 1);
//   _ch[chidx]->setLimits(0, 0x3FF);
  
//   debug_log << "channel" << format().id() << ch;

//   uint64_t rows = 6800;
//   uint columns = 192;
//   uint64_t idx = 0;
//   uint64_t parts = raw.size() / columns /rows;
//   var(parts);

//   for (uint pp = 0; pp < parts; pp++) {  
//     for (uint rrow = 0; rrow < rows; rrow++) {
   
//       for (uint rcol = 5; rcol < columns - 4; rcol ++) {
//   	uint64_t ridx = pp*rows*columns + rrow*columns + rcol;
//   	int crow = 0;
//   	int ccol = 0;

// 	 ccol = (rcol - 5) + (columns - 9 + hole)*pp;
// 	 crow = rrow;
// 	 idx = crow*(183+hole)*18*4 + ccol;
// 	 _ch[chidx]->set(idx, raw.at(ridx));
//       }
//     }

//     //debug_log << pp << shift;
//   }
  
//   raw.clear();
  
//   return true;
// }


// bool MsuGs::parse(const QByteArray& data, const QDateTime& )
// {
//   trc;
//   // QFile file1("msugs_all.xx.data");
//   // file1.open(QIODevice::WriteOnly);
//   // file1.write(data);
//   // file1.close();
//   // return true;
//   const QByteArray kSync = QByteArray::fromHex("0218A7A392DD9ABF");

//   int ch = vcid2ch.value(format().id());
//   debug_log << "channel" << format().id() << ch;

  
//   QMap<int, int> range;
//   int max = 0;

//   var(data.size());
  
//   int pos = data.indexOf(kSync, 0);
//   var(pos);
//   if (-1 == pos) {
//     error_log << QObject::tr("Не найдена синхрострока");
//      return false;
//   }
//   QList<QByteArray> res;
//   int start = pos;
//   while (pos != -1) {
//     pos =  data.indexOf(kSync, pos + kSync.size());
//     if (pos == -1) {
//       break;
//     }
    
//     range[pos-start] += 1;
//     if (pos-start > max) {
//       max = pos-start;
//     }

//     //debug_log << pos - start;
    
//     res.append(data.mid(start, pos - start));    
//     start = pos;
//   }

//   var(range);
//   var(max);
  
//   QFile file(QString("msugs.") + QString::number(format().id()) + ".data");
//   file.open(QIODevice::WriteOnly);
//   for (int idx = 0; idx < res.size(); idx++) {
//     int rem = max - res.at(idx).size();
//     Q_UNUSED(rem);
//     // if (rem != 0) {
//     //   QByteArray fill(rem, char(0));
//     //   file.write(fill);
//     //   // file.write(res.at(idx).mid(150-rem, res.at(idx).size() - (150-rem) - 10));
//     // } else {
//     //   //if (res.at(idx).size() == 13290) {
//     //   //file.write(res.at(idx).mid(150, res.at(idx).size() - 150 - 10));
//     //   //}
//     // }

//     //к макс
//     // file.write(res.at(idx).mid(200, res.at(idx).size() - 200 - 10));
//     // QByteArray fill(rem, char(0));
//     // file.write(fill);

    
//     // if (res.at(idx).size() == 13290) {
//     //   file.write(res.at(idx).mid(200, res.at(idx).size() - 200 - 10));
//     file.write(res.at(idx).mid(0, res.at(idx).size()));
//     // }
    
//   } 

//   file.close();

//   debug_log << "line" << max - 200 - 10;
  
//   return true;
// }


