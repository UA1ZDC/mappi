#include "mtvza.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/funcs/mn_funcs.h>

#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>

//#include <mappi/pretreatment/images/colors.h>
#include <mappi/pretreatment/formats/stream_utility.h>

#include <qimage.h>
#include <qfile.h>

#define MTVZA_FRAME_SIZE 248

const QByteArray kMtvzaSynhro = QByteArray::fromHex("FB386A45");

namespace {
  mappi::po::Instrument* createMtvza(const mappi::conf::Instrument& conf, const mappi::conf::InstrFormat& format, mappi::po::Frame* frame)
  {
    return new mappi::po::Mtvza(conf, format, frame);
  }

  static const bool res = mappi::po::singleton::SatFormat::instance()->
    registerInstrHandler(mappi::InstrFrameType(mappi::conf::kMtvza, mappi::conf::kMhrpt), createMtvza);
}

using namespace mappi;
using namespace po;

Mtvza::Mtvza(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent):
  InstrumentTempl(conf, format),
  _conf(conf),
  _frame(parent)
{ 
  _ch.resize(_conf.chan_cnt());
}

Mtvza::~Mtvza()
{
}


bool Mtvza::parse(const QByteArray& adata, QDateTime* dt, QDateTime*)
{
  trc;
  Q_UNUSED(dt);

  int64_t cur = findStartFrame(adata, 5);
  if (-1 == cur) {
    error_log << QObject::tr("Не найдена синхрострока");
    return false;
  }

  var(cur);

  int64_t next = cur;
  int row = 0;
  int cnt = (uchar)adata.at(cur + 5);
  while (cnt != 1) {
    next = adata.indexOf(kMtvzaSynhro, cur + kMtvzaSynhro.size());
    if (-1 == next) {
      break;
    }
    if (next - cur > 248) {
      cur += 248;
    } else {
      cur = next;
    }
    cnt = (uchar)adata.at(cur + 5);
  }
  var(cur);

  QByteArray data = adata.mid(cur, adata.size() - cur);

  long length = data.size() / MTVZA_FRAME_SIZE / 26;
  uint idx = 0;
  for (int i = 0; i < _ch.size(); i++) {
    _ch[i].setup(length, _conf.channel(idx).samples(), idx + 1);
    _ch[i].setLimits(0, 0xFFFF);
    debug_log << length <<  _conf.channel(i).samples() << _ch.size();
    idx++;
  }   


   // QFile file("1.dat");
  // file.open(QIODevice::WriteOnly);
  // file.write(data);
  // file.close();

  var(data.size());
  int64_t pos = 0;
  QByteArray mtvza;

  int sum  = 0;
    
  pos = 0;
  next = pos;

  QByteArray ba;
  //TODO посм min/max
  // uint min = 0xffffffff;
  // uint max = 0;
 

  while (pos + MTVZA_FRAME_SIZE <= data.size()/* && sum < 6785*/) {
    ++sum;
    next = data.indexOf(kMtvzaSynhro, pos + kMtvzaSynhro.size());
    if (next == -1) {
      debug_log << "off " << next << pos << data.size();
      if (data.size() - pos < 248) {
	break;
      } else {
	next = data.size();
      }
    }

    cnt = (uchar)data.at(pos + 5);

    if (cnt <= 0 /*&& (uchar)data.at(pos + 5) == 0 && (uchar)data.at(pos + 6) == 0*/) {
    } else {

      if ((next - pos) % 248 != 0) {
	//debug_log << "pos=" << next << next - pos << data.size() << (uchar)data.at(next + 4);
      }

      QByteArray frame = data.mid(pos, (next - pos) < 248 ? (next - pos) : 248);
      frame.resize(248);

      int offset =  0; //4 + 2 + 4; //синхро + 0xff + номер пакета + 2 канала (калибровка?)
      for (int curCh = 0; curCh < _conf.chan_cnt(); curCh++) {
	int byteCnt = _conf.channel(curCh).samples() / 52;
	int pixnum = row * 26*2*byteCnt + (cnt - 1)*2*byteCnt;
	if (curCh <= 4) {
	  offset = 4 + 2 + 4 + curCh*2;
	} else if (curCh == 5) {
	  offset = 4 + 2 + 4 + 8;
	} else {
	  offset = 4 + 2 + 4 + curCh*4;
	}
	ushort val = 0;
	for (int byte = 0; byte < byteCnt; byte++) {
	  //debug_log << curCh << pixnum << (offset + byte*2) << offset << byte << row << cnt;
	  val  = ((uchar)frame.at(offset + byte*2));
	  val += ((uchar)frame.at(offset + byte*2 + 1)) << 8;
	  _ch[curCh].set(pixnum++, val);
	}

	for (int byte = 0; byte < byteCnt; byte++) {
	  //debug_log << curCh << pixnum << (offset + byte*2 + 120) << offset << byte << row << cnt;
	  val  = ((uchar)frame.at(offset + byte*2 + 120));
	  val += ((uchar)frame.at(offset + byte*2 + 120 + 1)) << 8;
	  _ch[curCh].set(pixnum++, val);
	}
      }

      if (cnt == 26) {
	++row;
      }
    }
    
    if (next - pos > 248) {
      pos += 248;
    } else {
      pos = next;
    }
  }


  return true;
}

bool Mtvza::parsetest(const QByteArray& adata, const QDateTime& dt)
{
  trc;
  Q_UNUSED(dt);

  int64_t cur = findStartFrame(adata, 5);
  if (-1 == cur) {
    error_log << QObject::tr("Не найдена синхрострока");
    return false;
  }

  var(cur);

  QByteArray data = adata.mid(cur, adata.size() - cur);

  // QFile file("1.dat");
  // file.open(QIODevice::WriteOnly);
  // file.write(data);
  // file.close();

  var(data.size());
  int64_t pos = 0;
  QByteArray mtvza;

  int ch = 4;
  for (ch = 0; ch < 120; ch++) {

  int sum  = 0;
    
  pos = cur + MTVZA_FRAME_SIZE * (26-6 + 1);
  int64_t next = pos;

  QByteArray ba;
  //TODO посм min/max
  uint min = 0xffffffff;
  uint max = 0;

  int cnt = 0;
  while (pos + MTVZA_FRAME_SIZE <= data.size()/* && sum < 6785*/) {
    ++sum;
    next = data.indexOf(kMtvzaSynhro, pos + kMtvzaSynhro.size());

    if ((uchar)data.at(pos + 5) <= 1 /*&& (uchar)data.at(pos + 5) == 0 && (uchar)data.at(pos + 6) == 0*/) {
    } else {

      if ((next - pos) % 248 != 0) {
	//debug_log << "pos=" << next << next - pos << data.size() << (uchar)data.at(next + 4);
      }

      QByteArray frame = data.mid(pos, (next - pos) < 248 ? (next - pos) : 248);
      frame.resize(248);
      mtvza += frame.mid(4 + 2, 120);
      mtvza += frame.mid(4 + 2 + 120, 117);

      for (int kk = 0; kk <= 120; kk += 120) {
	uchar val = 0;//((uchar)data.at(pos + 4 + 2 + 3 + kk) & 0x3) << 4;
	val += (uchar)frame.at(4 + 2 + ch + kk);
	ba.append(uchar(val));//!!
	min = min < val ? min : val;
	max = max > val ? max : val;
	//ba.append(uchar((uchar)data.at(pos + 4 + 2 + ch + 1 + kk)));//!!
      }
      
      // ba.append((uchar)data.at(pos + 4 + 2 + 4  ));//!!
      // ba.append((uchar)data.at(pos + 4 + 2 + 4 + 120));//!!


      //debug_log << (uchar)data.at(pos + 4) << sum;
    }
    
    cnt = (uchar)data.at(pos + 5);
    //debug_log << cnt << sum;

    if (-1 == next) break;
    if (next - pos > 248) {
      pos += 248;
    } else {
      pos = next;
    }
  }

  // for (int idx = 0; idx < 26; idx++) {
  //   ba[idx] = 150;
  // }

  debug_log << "ch=" << ch << min << max << cnt << sum;
  saveImage(ba, QString::number(ch).rightJustified(3, '0'));
  //saveImage1(ba, QString::number(0).rightJustified(4, '0'));
  }

  return true;

  QFile file("2.dat");
  file.open(QIODevice::WriteOnly);
  file.write(mtvza);
  file.close();


  QByteArray mm;
  int remain = 0;
  bool ok = meteo::cif2raw(mtvza, 0, &mm, &remain, 10, false);
  debug_log << ok << remain << mtvza.size() << mm.size();
  
  // return true;

  file.setFileName("3.dat");
  file.open(QIODevice::WriteOnly);
  file.write(mm);
  file.close();
 
  int tt = 96;
  for (int ch = 0; ch < tt; ch++) {
    QByteArray ba;
    pos = 0;
    while (pos + tt*2 <= mm.size()) {
      ushort val = ((uchar)mm.at(pos + ch*2)) << 6 ;
      val += ((uchar)mm.at(pos + ch*2 + 1)) >> 6;
      ba.append(uchar(val >> 2));
      
      pos += tt*2;
    }
    saveImage(ba, QString::number(ch*2).rightJustified(3, '0') + "_");
    
  }


  return true;
}

//! Сохранение данных в виде изображения
void Mtvza::saveImage1(const QByteArray& data, const QString& ch)
{
  if (data.size() == 0) return;


  int cols = 26*4; //26 * 4;
  int rows = data.size() / 2 / cols;

  int ss = cols;
  var(ss);
  int size = ss*rows*4;

  uchar* imData = new uchar[size];

  // int max = 255;
  // int min = 0;

  int kidx = 0;
  for(int idx = 0; idx + 1 < size / 2; idx += 2) {

    imData[kidx] = uchar(data.at(idx)); //b
    imData[kidx + 1] = uchar(data.at(idx+1)*10);//g
    imData[kidx + 2] = uchar(data.at(idx+1)*10);//r
    imData[kidx + 3] = 255;

    kidx += 4;
  }
  
  var(data.size());
  //colors::stretchHist(imData, ss);

  QImage imqt(imData, ss, rows, ss*4, QImage::Format_ARGB32);
  debug_log << "imData" << cols << rows;

  imqt.save(QString("/tmp/img") + ch + ".bmp", "BMP");

  debug_log << "write" << QString("/tmp/img") + ch + ".bmp";

  delete[] imData;
}

//! Сохранение данных в виде изображения
void Mtvza::saveImage(const QByteArray& data, const QString& ch)
{
  if (data.size() == 0) return;


  int cols = 26*2; //26 * 4;
  int rows = data.size() / cols;

  int ss = round(cols / 4.0) * 4;
  var(ss);

  uchar* imData = new uchar[ss*rows];

  // int max = 255;
  // int min = 0;
  int idx = 0;
  for(int row = 0; row < rows; row++) {
    for(int col = 0; col < cols; col++) {
      // if (data.at(idx) > max || data.at(idx) < min) { //-9999 попадает
      //   //debug_log << idx << _data.at(idx) << _min << _max;
      //   imData[idx] = 0;
      // } else {
      //   imData[idx] = uchar((data.at(idx) - min) * 255 / (max - min));
      // }
      imData[idx] = uchar(data.at(row*cols + col));
      idx++;
    }
    if (cols % 4 != 0) { //выравнивание изображения
      for (int jc = 0; jc < 4 - (cols % 4); jc++) {
	imData[idx] = uchar(0); //imData[idx-1];
	idx++;
      }
    }

  }

  var(data.size());
  //colors::equalization(imData, ss*rows, cols, rows);
  //  colors::stretchHist(imData, ss*rows);

  QImage imqt(imData, ss, rows, ss, QImage::Format_Indexed8);
  debug_log << "imData" << cols << rows;
  imqt.setColorCount(256);
 
  QVector<QRgb> palette(256);
  for (uint i=0; i< 256; i++) {
    palette[i] = QRgb((i<<16)+ (i<<8) + i);
  }
  
  imqt.setColorTable(palette);
  imqt.save(QString("/tmp/img") + ch + ".bmp", "BMP");

  debug_log << "write" << QString("/tmp/img") + ch + ".bmp";

  delete[] imData;
}


//! Поиск начала кадра
int64_t Mtvza::findStartFrame(const QByteArray& data, int goodCnt)
{
  int good = 0;
  int pos = data.indexOf(kMtvzaSynhro, 0);
  int oldpos = pos;
  int64_t startpos = -1; 
  //  var(pos);

  while(pos != -1) {
    pos = data.indexOf(kMtvzaSynhro, pos + kMtvzaSynhro.size());
    debug_log << pos << oldpos << (pos - oldpos);
    if (pos == -1) break;

    if (pos - oldpos == MTVZA_FRAME_SIZE) {
      ++good;
      if (startpos == -1) {
	startpos = oldpos;
      }
    } else {
      good = 0;
      startpos = -1;
    }

    oldpos = pos;
    
    if (good >= goodCnt) break;

  }

  return startpos;
}
