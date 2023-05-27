#include "viirs.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/debug/tmap.h>
#include <cross-commons/app/paths.h>

#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/formats/stream_utility.h>
#include <mappi/pretreatment/frames/m_ccsds.h>

extern "C" {
#include <libaec.h>
}

#include <QDebug>

int uses_decompress(const char *input, char *output, int inputLen, int outputLen);


namespace {
  mappi::po::Instrument* createViirs(const mappi::conf::Instrument& conf, const mappi::conf::InstrFormat& format, mappi::po::Frame* frame)
  {
    return new mappi::po::Viirs(conf, format, frame);
  }

  static const bool res = mappi::po::singleton::SatFormat::instance()->
	registerInstrHandler(mappi::InstrFrameType(mappi::conf::kViirs, mappi::conf::kCadu), createViirs);
  static const bool res1 = mappi::po::singleton::SatFormat::instance()->
	registerInstrHandler(mappi::InstrFrameType(mappi::conf::kViirs, mappi::conf::kCaduClipped), createViirs);
}

using namespace meteo;
using namespace mappi;
using namespace po;

static const TMap<int, ViirsChannelSettings> apid2ch = TMap<int, ViirsChannelSettings>()
  //M
  << QPair<int, ViirsChannelSettings>(804, { 0,   "M1", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 2, 3, 3, 2, 1}, false})
  << QPair<int, ViirsChannelSettings>(803, { 1,   "M2", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 2, 3, 3, 2, 1}, false})
  << QPair<int, ViirsChannelSettings>(802, { 2,   "M3", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 2, 3, 3, 2, 1}, false})
  << QPair<int, ViirsChannelSettings>(800, { 3,   "M4", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 2, 3, 3, 2, 1}, false})
  << QPair<int, ViirsChannelSettings>(801, { 4,   "M5", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 2, 3, 3, 2, 1}, false})
  << QPair<int, ViirsChannelSettings>(805, { 5,   "M6", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 1, 1, 1, 1, 1},  true})
  << QPair<int, ViirsChannelSettings>(806, { 6,   "M7", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 2, 3, 3, 2, 1}, false})
  << QPair<int, ViirsChannelSettings>(809, { 7,   "M8", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 1, 1, 1, 1, 1}, false})
  << QPair<int, ViirsChannelSettings>(807, { 8,   "M9", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 1, 1, 1, 1, 1}, false})
  << QPair<int, ViirsChannelSettings>(808, { 9,  "M10", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 1, 1, 1, 1, 1}, false})
  << QPair<int, ViirsChannelSettings>(810, {10,  "M11", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 1, 1, 1, 1, 1}, false})
  << QPair<int, ViirsChannelSettings>(812, {11,  "M12", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 1, 1, 1, 1, 1}, false})
  << QPair<int, ViirsChannelSettings>(811, {12,  "M13", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 2, 3, 3, 2, 1},  true})
  << QPair<int, ViirsChannelSettings>(816, {13,  "M14", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 1, 1, 1, 1, 1},  true})
  << QPair<int, ViirsChannelSettings>(815, {14,  "M15", {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 1, 1, 1, 1, 1},  true})
  << QPair<int, ViirsChannelSettings>(814, {15,  "M16",  {640, 368, 592, 592, 368, 640}, 16, 3200, {1, 1, 1, 1, 1, 1},  true})
  //DNB, DNB MGS, DNB LGS
  << QPair<int, ViirsChannelSettings>(821, {16,  "DNB", {784, 488, 760, 760, 488, 784}, 16, 4064, {1, 1, 1, 1, 1, 1}, false})
  << QPair<int, ViirsChannelSettings>(822, {17, "DNBM", {784, 488, 760, 760, 488, 784}, 16, 4064, {1, 1, 1, 1, 1, 1}, false})
  << QPair<int, ViirsChannelSettings>(823, {18, "DNBL", {784, 488, 760, 760, 488, 784}, 16, 4064, {1, 1, 1, 1, 1, 1}, false})
  //I
  << QPair<int, ViirsChannelSettings>(818, {19,   "I1", {1280, 736, 1184, 1184, 736, 1280}, 32, 6400, {1, 1, 1, 1, 1, 1}, false})
  << QPair<int, ViirsChannelSettings>(819, {20,   "I2", {1280, 736, 1184, 1184, 736, 1280}, 32, 6400, {1, 1, 1, 1, 1, 1}, false})
  << QPair<int, ViirsChannelSettings>(820, {21,   "I3", {1280, 736, 1184, 1184, 736, 1280}, 32, 6400, {1, 1, 1, 1, 1, 1}, false})
  << QPair<int, ViirsChannelSettings>(813, {22,   "I4", {1280, 736, 1184, 1184, 736, 1280}, 32, 6400, {1, 1, 1, 1, 1, 1},  true})
  << QPair<int, ViirsChannelSettings>(817, {23,   "I5", {1280, 736, 1184, 1184, 736, 1280}, 32, 6400, {1, 1, 1, 1, 1, 1},  true});

const int kCalibrApid = 825;
//const int kEngineeringApid = 826;
// <Packet apid="827" apidShortName="DNB_HGA" description="Day Night Band - High Gain Stage A" totalGroupCount="17"/>
// <Packet apid="828" apidShortName="DNB_HGB" description="Day Night Band - High Gain Stage B" totalGroupCount="17"/>

	// <RDRName productID="RVIRT" csn="VIIRS-TELEMETRY-RDR" sensor="VIIRS" typeID="TELEMETRY" numAPIDs="1" numPktTrackers="338" granSize="602">
	// 	<Packet apid="768" apidShortName="HK" description="Housekeeping Telemetry" totalGroupCount="1"/>
	// </RDRName>


Viirs::Viirs(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent):
  InstrumentTempl(conf, format),
  _conf(conf),
  _frame(parent)
{ 
  _ch.resize(_conf.chan_cnt());
}

Viirs::~Viirs()
{
}

float Viirs::fovStep(int channel) const
{
  var(channel);
  if (channel < 15) return 0.035175; 
  if (channel < 19) return 0.0277;
  return 0.017587;
}


//bool Viirs::parse(const QByteArray& dataAll, const QDateTime& adt)
bool Viirs::parse(const QList<QByteArray>& dataAll, QDateTime* dtStart, QDateTime* dtEnd)
  
{
  if(dataAll.size() < 1) {
    return false;
  }
  
  trc;
  
  var(format().id());
  var(dataAll.size());

  int idx = -1;
  QDateTime dt;
  dt = *dtStart;
  
  while (++idx < dataAll.size()) {
    const QByteArray& data = dataAll.at(idx);
    // var(data.size());

    CCSDSHeader header;
    bool ok = header.parse(data, QDate(1958, 1, 1));
    if (!ok) {
      continue;
    }
    
    if (data.size() < header.size + 16) {
      warning_log << QObject::tr("small size");
      continue;
    }   
    
    if (!apid2ch.contains(header.apid)) {
      if (header.apid == kCalibrApid) {
	readCalibrData(header, data);
      }
      continue;
    }

    //    continue; //TODO debug
    
    int channel = apid2ch.value(header.apid).channel;
    
    //debug
    // uint viirsSeq = ((uint8_t)data[header.size + 0] << 24) + ((uint8_t)data[header.size + 1] << 16) + 
    //                 ((uint8_t)data[header.size + 2] << 8) +  (uint8_t)data[header.size + 3];

    dt.setDate(QDate(1958, 1, 1));
    dt = dt.addDays( ((uint8_t)data[header.size + 4] << 8) +  (uint8_t)data[header.size + 5]);
    dt.setTime(QTime(0,0));
    dt = dt.addMSecs(((uint8_t)data[header.size + 6] << 24) + ((uint8_t)data[header.size + 7] << 16) + 
		     ((uint8_t)data[header.size + 8] << 8) +  (uint8_t)data[header.size + 9]);

    
    // int vers = (uint8_t)data[header.size + 12];
    // int instr = (uint8_t)data[header.size + 13];

    // debug_log << header
    // 	      << channel
    //  	      << dt.toString("yyyy-MM-ddThh:mm:ss.zzz");
      // << viirsSeq << vers << instr;
    
    // debug_log << "apid cnt" << cnt << apid;

    
    if (header.hasSecondary && header.seqFlag == 0x1) {
      
      _band[channel].setStart(header.apid, header.dts, header.pckCnt, header.cnt);
      
    } else {
      //debug_log << "parse" << channel;
      _band[channel].parseScienceData(dt, header.cnt, data.mid(header.size + 16));
    }    
  }

  // const QList<ViirsCalibrScan>& cb = *_calBand.data();
  // debug_log << cb.size();
  // for (int cidx = 0; cidx < 10; cidx++) {
  //   const ViirsCalibrScan& cs = cb.at(cidx);
  //   var(cs.pck.size());
    
  //   QMapIterator<int, ViirsCalibrChannel> it(cs.pck);
  //   while (it.hasNext()) {
  //     it.next();
  //     debug_log << "band = " << it.key()
  // 		<< "\nspace" << it.value().space
  // 		<< "\nbb " << it.value().blackBody
  // 		<< "\nsolar " << it.value().solarDiffuser;
  //   }
  // }
  
  fillDeletedLines();

  recoverDiffChannels(4,  3, 1);//M5, M4
  recoverDiffChannels(2,  4, 1); //M3, M4
  recoverDiffChannels(1,  2, 1); //M2, M3
  recoverDiffChannels(0,  1, 1); //M1, M2
  recoverDiffChannels(7,  9, 1); //M8, M10
  recoverDiffChannels(10, 9, 1); //M11, M10
  recoverDiffChannels(22, 11, 2); //I4, M12
  recoverDiffChannels(13, 14, 1); //M14, M15
  recoverDiffChannels(23, 14, 2); //I5, M15
  recoverDiffChannels(20, 19, 1); //I2, I1
  recoverDiffChannels(21, 20, 1); //I3, I2

 
  for (auto ch :  _band.keys()) {
    if (ch < 16 && ch > 18) {
      _ch[ch].setLimits(0, 0xFFF);
      _ch[ch].setMask(0xFFF);
    } else if (ch == 16) {
      _ch[ch].setLimits(0, 0xFFFF);
      _ch[ch].setMask(0x3FFF);
    } else {
      _ch[ch].setLimits(0, 0x1FFF);
      _ch[ch].setMask(0x1FFF);
    }
    _ch[ch].setNum(ch+1);
    _ch[ch].setName(apid2ch.value(_band.value(ch).apid()).name);
    QVector<ushort> all = _band.value(ch).alldata();
    int h = all.size() / _conf.channel(ch).samples();
    // debug_log << ch <<  "wxh=" << _conf.channel(ch).samples() << h << all.size();
    if (all.size() == 0) {
      continue;
    }
    _ch[ch].setData(all, h, _conf.channel(ch).samples());       
  }
  

  *dtStart = _band.value(0).data().first().dt;
  dtStart->setTimeSpec(Qt::UTC);

  *dtEnd = _band.value(0).data().last().pck.last().dt();
  dtEnd->setTimeSpec(Qt::UTC);

  
  var(*dtStart);
  var(*dtEnd);
  var(dt);
  
  _frame->setDateTime(*dtStart,
		      *dtEnd);
		      // _band.value(0).data().last().pck.last().dt());
		      // _band.value(0).data().last().dt);
  
  return true;
}

//M5 = M4 + M5 - (2^14 - 1)
bool Viirs::recoverDiffChannels(int diffNum, int baseNum, int step)
{
  if (!_band.contains(diffNum) ||  !_band.contains(baseNum)) {
    return false;
  }

  QList<ViirsDetectorsScan> base = _band.value(baseNum).data();
  QList<ViirsDetectorsScan>* ch  = _band[diffNum].data();
  
  int offsetBase = 0;
  int offsetCh = 0;

  //TODO сдвинуть до валидного времени
  
  if (base.first().dt < ch->first().dt) { 
    offsetBase++;
    while (offsetBase < base.size() && base.at(offsetBase).dt != ch->last().dt) {
      offsetBase++;
    }
  } else if (base.first().dt > ch->first().dt) {
    offsetCh++;
    while (offsetCh < ch->size() && base.first().dt != ch->at(offsetCh).dt){
      offsetCh++;
    }
  }

  debug_log << "diff offset" << offsetCh << offsetBase << diffNum << baseNum;
  
  if (offsetBase == base.size() || offsetCh == ch->size()) {
    return false;
  }
 
  int size = ch->size() - offsetCh;
  for (int idx = offsetCh; idx < size; idx++) {
    if (idx / step >= base.size()) {
      *ch = ch->mid(offsetCh, idx);
      break;
    }
    for (int pckIdx = 0; pckIdx < ch->at(idx).pck.size(); pckIdx++) {
      QVector<ushort>* chScan = (*ch)[idx].pck[pckIdx].data();
      const QVector<ushort>& baseScan = base.at(idx).pck.at(pckIdx / step).data();
      for (int valIdx = 0; valIdx < chScan->size(); valIdx++) {
	(*chScan)[valIdx] = (int)chScan->at(valIdx) + baseScan.at(valIdx / step) - 0x3FFF;
      }
      
    }
    
  }

  return true;
}

void Viirs::fillDeletedLines()
{
  bool flag = false;
  if (pretrConfig().image_size() != 0) {
    for (const auto &imconf : pretrConfig().image()) {
      if ("\\w+" == imconf.ch_rx())
	{
	  for (auto filter : imconf.filter()) {
	    if (filter == conf::kFillEmpty) {
	      flag = true;
	      break;
	    }
	  }
	}
    }
  }

  if (flag == false) {
    return;
  }
  
  for (auto ch :  _band.keys()) {
    QList<ViirsDetectorsScan>* det = _band[ch].data();
    for (int bIdx = 0; bIdx < det->size(); bIdx++) {
      (*det)[bIdx].interpolateLines(_band.value(ch).apid());
    }    
  }
}


void Viirs::readCalibrData(const CCSDSHeader& header, const QByteArray& data)
{
  //debug
  // uint viirsSeq = ((uint8_t)data[header.size + 0] << 24) + ((uint8_t)data[header.size + 1] << 16) + 
  //                 ((uint8_t)data[header.size + 2] <<  8) +  (uint8_t)data[header.size + 3];

  QDateTime dt;
  dt.setDate(QDate(1957, 1, 1));
  dt = dt.addDays( ((uint8_t)data[header.size + 4] <<  8) +  (uint8_t)data[header.size + 5] - 1);
  dt.setTime(QTime(0,0));
  dt = dt.addMSecs(((uint8_t)data[header.size + 6] << 24) + ((uint8_t)data[header.size + 7] << 16) + 
		   ((uint8_t)data[header.size + 8] <<  8) +  (uint8_t)data[header.size + 9]);
  
  // int vers = (uint8_t)data[header.size + 12];
  // int instr = (uint8_t)data[header.size + 13];

  int offset = header.size + 16;
  

  // debug_log << "apid cnt" << cnt << apid;
  
  
  if (header.hasSecondary && header.seqFlag == 0x1) {

    uint8_t dnbSeq = (((uint8_t)data[offset + 0] & 0x3) << 4) + ((uint8_t)data[offset + 1] >> 4);
    
    // debug_log << header
    // 	      << dt.toString("yyyy-MM-ddThh:mm:ss.zzz")
    // 	      << viirsSeq
    //   // << vers << instr
    // 	      << dnbSeq;
  
    
    _calBand.setStart(header.dts, header.pckCnt, header.cnt, dnbSeq);
    
  } else {
    //    int band = ((uint8_t)data[offset + 73] >> 2) & 0x1f;

    
    // debug_log << header
    // 	      << dt.toString("yyyy-MM-ddThh:mm:ss.zzz")
    // 	      << viirsSeq
    //   // << vers << instr
    // 	      << (uint8_t)data[offset + 2] << band << (uint8_t)data[offset + 3];
  
    //debug_log << "parse" << channel;
    _calBand.parseScienceData(dt, header.cnt, data.mid(header.size + 16));
    
  }
    
}


//---- ViirsScan

bool ViirsScan::parseSegments(int apid, const QByteArray& detectorData)
{
  const ViirsChannelSettings& sett = apid2ch.value(apid);
    
  uint16_t offset = 72; //сдвиг до данных
  uint16_t chsumOff = 0;
  uint16_t fillBytes = 0;
  uint32_t syncWord = 0;
  
  //  var(detectorData.size());
  if (detectorData.size() < offset + 8) {
    warning_log << QObject::tr("small size (< 76)");
    _scan.append(QVector<ushort>(sett.totalWidth, 0));
    return false;
  }
  
  //debug_log << "band" << (uint8_t)detectorData[2] << sett.channel;

  uint32_t syncPattern = ((uint8_t)detectorData[4] << 24) + ((uint8_t)detectorData[5] << 16) +
                         ((uint8_t)detectorData[6] <<  8) + ((uint8_t)detectorData[7]);


  QVector<ushort> scan;
  for (int idx = 0; idx < 6; idx++) {
    if (detectorData.size() < offset + 4) {
      warning_log << QObject::tr("small size (< %1").arg(offset + 4);
      _scan.append(QVector<ushort>(sett.totalWidth, 0));
      return false;
    }    
    
    fillBytes  = (((uint8_t)detectorData[offset + 0]) /*+ (uint8_t)detectorData[offset + 1]*/) / 8; //число добавленных пустых байт для выравнивания
    chsumOff   = ((uint8_t)detectorData[offset + 2] << 8) + (uint8_t)detectorData[offset + 3];
    // debug_log << offset + 16 << chsumOff << fillBytes << (uint8_t)detectorData[offset + 0] << (uint8_t)detectorData[offset + 1]
    // 	      << (uint8_t)detectorData[offset + 2] << (uint8_t)detectorData[offset + 3];
       
    offset += 4;

    if (chsumOff == 0 || detectorData.size() < offset + chsumOff + 4) {
      warning_log << QObject::tr("small size (< 76)");
      _scan.append(QVector<ushort>(sett.totalWidth, 0));
      return false;
    }

    
    if (chsumOff == 8) {
      //warning_log << QObject::tr("empty segment");
      scan.append(QVector<ushort>(sett.zoneWidth[idx], 0));
      
    } else {
    
    
      syncWord = ((uint8_t)detectorData[offset + chsumOff + 0] << 24) + ((uint8_t)detectorData[offset + chsumOff + 1] << 16) +
	((uint8_t)detectorData[offset + chsumOff + 2] << 8) + (uint8_t)detectorData[offset + chsumOff + 3];
    
    if (syncWord != syncPattern) {
      debug_log << "error sync" << idx << syncPattern << syncWord << apid;
    }
    
    
    QByteArray seg = detectorData.mid(offset, chsumOff - 4); 
    
    uint resSize = sett.zoneWidth[idx] * sett.oversampleZone[idx]*2;
    QVector<ushort> one = decompress(seg.left(seg.size() - fillBytes), resSize, sett.oversampleZone[idx]);
    scan.append(one);
    
  }
  
    // if (apid == 821) {
    //   debug_log << "seg" << idx;
    //   for (int ii = 0; ii < one.size(); ii++) {
    // 	std::cout << one.at(ii) << " ";
    //   }
    //   std::cout << "\n";
    // }

    
    offset += chsumOff + 4;
  }

  
  _scan.append(scan);

  // var(offset);
  
  return true;
}

QVector<ushort> ViirsScan::decompress(const QByteArray& segment, uint resSize, int oversample)
{
  QByteArray res(resSize, '0');
  // Decompress
  uses_decompress(segment.data(), res.data(), segment.size(), resSize);
  
  QVector<ushort> scan;

  ushort val = 0;
  for (uint idx = 0; idx < resSize; idx += oversample*2) {
    if (oversample == 3) {
      val = (((uint8_t)res[idx + 0] << 8) + (uint8_t)res[idx + 1] +
	     ((uint8_t)res[idx + 2] << 8) + (uint8_t)res[idx + 3] +
	     ((uint8_t)res[idx + 4] << 8) + (uint8_t)res[idx + 5]) / oversample;	
    } else if (oversample == 2) {
      val = (((uint8_t)res[idx + 0] << 8) + (uint8_t)res[idx + 1] +
	     ((uint8_t)res[idx + 2] << 8) + (uint8_t)res[idx + 3]) / oversample;
    } else {
      val = ((uint8_t)res[idx + 0] << 8) +  (uint8_t)res[idx + 1];
    }
        
    scan.append(val);    
  }
 
  return scan;
}

//---- ViirsDetectorsScan

bool ViirsDetectorsScan::addPacket(const QDateTime& adt, int apid, const QByteArray& detectorData)
{
  ViirsScan scan(adt);
  bool ok = scan.parseSegments(apid, detectorData);
  pck.prepend(scan);
  return ok;
}

void ViirsDetectorsScan::addEmpty(int apid, int cnt)
{
  const ViirsChannelSettings& sett = apid2ch.value(apid);
  
  for (int idx = 0; idx < cnt; idx++) {
    ViirsScan scan(sett.totalWidth);
    pck.prepend(scan);
  }
}


//интерполирование, чтоб закрыть пропуски и совместить полосы
void ViirsDetectorsScan::interpolateLines(int apid)
{
  //var(apid);
  const ViirsChannelSettings& sett = apid2ch.value(apid);
  int height = pck.size();
  

  int cutSize = 0;
  if (height == 32) {
    cutSize = 4;
  } else {
    cutSize = 2;
  }
  float coef = 3/5.;//коэффициент растяжения y = y'*coef

  interpolateZone(coef, cutSize, cutSize, height, 0, sett.zoneWidth[0]);
  interpolateZone(coef, cutSize, cutSize, height, pck.at(1).data().size() - sett.zoneWidth[0], sett.zoneWidth[0]);

  cutSize /= 2;
  coef = 7/8.;
  
  interpolateZone(coef, cutSize, 0, height, sett.zoneWidth[0], sett.zoneWidth[1]);
  interpolateZone(coef, cutSize, 0, height, pck.at(1).data().size() - sett.zoneWidth[1] - sett.zoneWidth[0], sett.zoneWidth[1]); 
}

void ViirsDetectorsScan::interpolateZone(float coef, int cutSize, int resCutSize, int height, int posStart, int width)
{
  QList<QVector<ushort>> zone; //интерполированный кусочек полосы
  
  //h = 32 - 2*4 = 24 (2*4 - черные полосы) -> h'=40 (и по 4 отбрасываем) => 32
  //для второй зоны растягиваем ровно по полосе, без отбрасывания
  for (int idx = 0; idx < height; idx++) {
    QVector<ushort> line(width);
    
    int y_ = idx + resCutSize;
    float y = (y_ * coef) + cutSize; //умножаем на коэффициент растяжения добавляем сдвиг на черную полосу (чтоб отсчет от данных)
    int y1 = floor(y);
    int y2 = ceil(y);
    
    for (int x = 0; x < width; x++) {
      if (y1 == y2 || y2 >= height - cutSize) {
	line[x] = pck.at(y1).data().at(x + posStart);
      } else {
	line[x] = (y2-y)/(y2-y1) * pck.at(y1).data().at(x + posStart) + (y-y1)/(y2-y1) * pck.at(y2).data().at(x + posStart);
      }
    }
    zone.append(line);
  }
  
  for (int idx = 0; idx < height; idx++) {
    for (int x = 0; x < width; x++) {
      (*pck[idx].data())[x + posStart] = zone.at(idx).at(x);
    }
  }
}


//---- ViirsCalibrScan

bool ViirsCalibrScan::addPacket(const QDateTime& /*adt*/, const QByteArray& cdata)
{
  uint16_t offset = 76; //сдвиг до данных
  uint16_t chsumOff = 0;
  uint16_t fillBytes = 0;
  uint32_t syncWord = 0;
  
  if (cdata.size() < offset + 4) {
    warning_log << QObject::tr("small size (< %1").arg(offset + 4);
    return false;
  } 
  
  int spv = (uint8_t)cdata[3]; //samples per view
  int band = ((uint8_t)cdata[73] >> 2) & 0x1f;
  uint32_t syncPattern = ((uint8_t)cdata[4] << 24) + ((uint8_t)cdata[5] << 16) +
                         ((uint8_t)cdata[6] <<  8) + ((uint8_t)cdata[7]);
  

  //  var(cdata.size());
  int cnt = 0;
  
  while (cdata.size() > offset + 4) {
    cnt++;
    for (uint didx = 0; didx < 3; didx++) {

      if (cdata.size() < offset + 4) {
	warning_log << QObject::tr("small size (< %1").arg(offset + 4);
	return false;
      }    
	  
      fillBytes  = (((uint8_t)cdata[offset + 0]) /*+ (uint8_t)cdata[offset + 1]*/) / 8; //число добавленных пустых байт для выравнивания
      chsumOff   = ((uint8_t)cdata[offset + 2] << 8) + (uint8_t)cdata[offset + 3];
      
      offset += 4;
      
      if (chsumOff == 0 || cdata.size() < offset + chsumOff + 4) {
	warning_log << QObject::tr("small size (< 76)");
	return false;
      }
      
      syncWord = ((uint8_t)cdata[offset + chsumOff + 0] << 24) + ((uint8_t)cdata[offset + chsumOff + 1] << 16) +
	((uint8_t)cdata[offset + chsumOff + 2] << 8) + (uint8_t)cdata[offset + chsumOff + 3];
      
      if (syncWord != syncPattern) {
	debug_log << "error sync" << didx << syncPattern << syncWord;
      }
      
      
      QByteArray segment = cdata.mid(offset, chsumOff - 4 - fillBytes);
      //debug_log << offset << chsumOff << fillBytes << segment.size() << spv * 2;
      
      uint resSize = spv * 2;
      QByteArray res(resSize, '0');
      uses_decompress(segment.data(), res.data(), segment.size(), resSize);

      QVector<ushort> one;
      for (uint idx = 0; idx < resSize; idx += 2) {
	one.append( ((uint8_t)res[idx + 0] << 8) +  (uint8_t)res[idx + 1]);
      }

      if (didx == 0) {
	pck[band].space.append(one);
      } else if (didx == 1) {
	pck[band].blackBody.append(one);
      } else if (didx == 2) {
	pck[band].solarDiffuser.append(one);
      }
      

      
      offset += chsumOff + 4;
    }
    
  }
  // var(offset);
  // var(cnt);
  
  return true;  
}



//---- 

// Decompress using USES algorithm
int uses_decompress(const char *input, char *output, int inputLen, int outputLen)
{
    struct aec_stream strm;
    strm.bits_per_sample = 15;
    strm.block_size = 8;
    strm.rsi = 128;
    strm.flags = AEC_DATA_MSB | AEC_DATA_PREPROCESS;
    strm.next_in = (const unsigned char *)input;
    strm.avail_in = inputLen;
    strm.next_out = (unsigned char *)output;
    strm.avail_out = outputLen * sizeof(char);
    aec_decode_init(&strm);
    aec_decode(&strm, AEC_FLUSH);
    if (aec_decode_init(&strm) != AEC_OK)
        return 1;
    if (aec_decode(&strm, AEC_FLUSH) != AEC_OK)
        return 1;
    aec_decode_end(&strm);

    return 0;
}

