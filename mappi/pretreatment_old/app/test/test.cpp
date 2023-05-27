#include <qcoreapplication.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/mathtools/mnmath.h>

#include <mappi/pretreatment/formats/stream_utility.h>

#include <unistd.h>

#include <qfile.h>
#include <qdatastream.h>

#include <qdebug.h>

//возвращает двоичное представление числа (для отладки)
QString db(uint64_t value)
{
  QString value_binary("\0");
  
  if ((value)!=1 && value != 0) { value_binary=db(value/2);}
  char a=value%2+48;
  return value_binary+=a;
}


const uchar synchro_meteor_32[4] = { 0x1A, 0xCF, 0xFC, 0x1D };
int synchro_len_meteor_32 = 4;

// void testFindSynchro(const QByteArray& data, bool invert, QByteArray* syndata)
// {
//   int shift_bit = 0, shift_byte = 0;
//   bool ok; = meteo::po::findSynchro(data, synchro_meteor_32, synchro_len_meteor_32, &shift_bit, &shift_byte, 0, 0xFF, invert);
//   var(ok);
//   if (!ok) {
//     error_log << "Err search synchro";
//   } else {
//     var(shift_byte);
//     var(shift_bit);
//   }

//   int remain = 0;
//   meteo::po::cif2raw(data.right(data.size() - shift_byte), shift_bit, syndata, &remain, 8, invert);
// }

void testFindSynchro(const QString& fileName, bool /*invert*/)
{
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);

  QByteArray data = file.readAll(); 
  QByteArray syndata;
  //  testFindSynchro(data, invert, &syndata);

  file.close();
 
  file.setFileName("test-mn2.1.raw");
  file.open(QIODevice::WriteOnly);
  file.write(syndata);
  file.close();
}

enum DecodeState {
  kFindSync = 1,
  kResync = 2,
  kCheckSync = 3,
  
};

void testManchesterDecode(const QByteArray& data, QByteArray synchroba,
			  int frameSize, int wordSize, QByteArray* decoded)
{
  bool isMidBit = false;
  bool isCheckMidBit = false;

  uint8_t prev = 0;
  uint8_t bit = 0;
  
  uint64_t synchro = 0;
  int synLength = synchroba.size();
  for (int idx = synLength - 1, sh = 0; idx >=0; idx--, sh += 8) {
    synchro += ((uchar)synchroba[idx] << sh);
  }

  uint8_t word = 0;
  uint8_t bitcnt = wordSize;
  uint16_t wordcnt = frameSize - synLength;

  DecodeState state = kFindSync;
  uint32_t shifter = 0;

  int maxcorr = 1;
  int maxerr = 5;
  int threshold = maxcorr;

  var(data.size());

  //  int frsize = 0;

  for (int64_t byte = 0; byte < data.size(); byte++) {
    // if (byte % 1000000 == 0) {
    //   debug_log << byte << data.size() << decoded->size();
    // }
    for (int shift = wordSize - 1; shift >= 0; shift--) {
      bit = ((~(uint8_t)data.at(byte)) >> shift) & 0x1;
      uint8_t diff = bit ^ prev;
      //debug_log << bit << prev << diff << isMidBit << byte << shift << db((uint8_t)data.at(byte)) << db(word);
      prev = bit;
      
      switch (state) {
      case kFindSync:
	//	debug_log << isMidBit << diff;
  
	if (isMidBit && 0 != diff) {
	  shifter = (shifter << 1) | bit;

	  //  debug_log << shifter << synchro << bit; 
	  
	  if (shifter == synchro) {
	    state = kResync;
	    bitcnt = wordSize;
	    wordcnt = frameSize - synLength;
	    word = 0;
	    decoded->append(synchroba);
	    //	    frsize = synLength;
	    isCheckMidBit = false;
	    debug_log << "syn found";
	  }
	  isMidBit = false;
	} else {
	  isMidBit = true;
	}
	break;
      case kResync:
	if (isMidBit) {
	  word = (word << 1) | bit;
	  if (--bitcnt == 0) {
	    decoded->append(word);
	    //	    ++frsize;
	    bitcnt = wordSize;
	    word = 0;
	    if (--wordcnt == 0) {
	      wordcnt = frameSize;
	    }
	  }
	  isMidBit = false;
	} else {
	  isMidBit = true;
	}

	if (isCheckMidBit && 0 != diff) {
	  shifter = (shifter << 1) | bit;
	  if (shifter == synchro) {
	    if (wordcnt != frameSize - synLength || bitcnt != wordSize) {
	      threshold = maxcorr;

	      debug_log << "resync" << threshold << wordcnt << bitcnt;
	      //	      debug_log << frsize;

	      if (frameSize - wordcnt < synLength) {
		decoded->append(synLength - wordcnt);
	      } else if (frameSize - wordcnt > synLength) {
		decoded->append(frameSize - wordcnt + synLength, char(0));
	      }
	      int idx = decoded->size() - synLength;
	      decoded->replace(idx, synLength, synchroba);

	      bitcnt = wordSize;
	      wordcnt = frameSize - synLength;
	      word = 0;
	      isMidBit = false;

	      //frsize = synLength;
	    } else {
	      --threshold;
	      // debug_log << "resync" << threshold;
	    }
	  } else if (wordcnt == frameSize - synLength && bitcnt == wordSize) {
	    threshold = maxcorr;
	  }
	  isCheckMidBit = false;
	} else {
	  isCheckMidBit = true;
	}
	 
	if (threshold == 0) {
	  state = kCheckSync;
	  threshold = maxerr;
	}
	 
	break;
      case kCheckSync:
	if (isMidBit) {
	  word = (word << 1) | bit;
	  if (--bitcnt == 0) {
	    decoded->append(word);
	    //	    ++frsize;
	    bitcnt = wordSize;
	    word = 0;
	    if (--wordcnt == 0) {
	      wordcnt = frameSize;
	    }
	  }
	  isMidBit = false;
	} else {
	  isMidBit = true;
	}

	if (isCheckMidBit && 0 != diff) {
	  shifter = (shifter << 1) | bit;
	  if (shifter == synchro) {
	    if (wordcnt != frameSize - synLength || bitcnt != wordSize) {
	      --threshold;
	      // debug_log << "chsync" << threshold << wordcnt << bitcnt;
	    } else {
	      threshold = maxerr;
	    }
	  } else if (wordcnt == frameSize - synLength && bitcnt == wordSize) {
	    --threshold;
	    // debug_log << "chsync" << threshold;
	  }
	  isCheckMidBit = false;
	} else {
	  isCheckMidBit = true;
	}
	 
	if (threshold == 0) {
	  state = kResync;
	  threshold = maxcorr;
	}

	break;
      }
     
    }
  }
}


void testManchesterDecodeNoaa(const QByteArray& data, QByteArray synchroba,
			      int frameSize, int wordSize, QByteArray* decoded)
{
  bool isMidBit = false;
  bool isCheckMidBit = false;
 
  uint8_t prev = 0;
  uint8_t bit = 0;
  
  uint64_t synchro = 0;
  for (int idx = synchroba.size() - 1, sh = 0; idx >=0; idx--, sh += 8) {
    synchro += ((uint64_t)((uchar)synchroba[idx]) << sh);
  }
  int synLength = 6;
  var(synLength);
  var(synchro);
  uint64_t syncmask = 0x0FFFFFFFFFFFFFFFLL;
  debug_log << db(synchro) << db(syncmask);

  uint16_t word = 0;
  uint8_t bitcnt = wordSize;
  uint16_t wordcnt = frameSize - 6;

  DecodeState state = kFindSync;
  uint64_t shifter = 0;
   int maxcorr = 1;
   int maxerr = 5;
   int threshold = maxcorr;

  var(data.size());
  int oldbyte = 0;

  //  int framebitcnt = 110900 - synchroba.size()*8;

  //  int frsize = 0;

  for (int64_t byte = 0; byte < data.size(); byte++) {
    // if (byte % 1000000 == 0) {
    //   debug_log << byte << data.size() << decoded->size();
    // }
    for (int shift = 7; shift >= 0; shift--) {
      bit = (((uint8_t)data.at(byte)) >> shift) & 0x1;
      uint8_t diff = bit ^ prev;
      // debug_log << bit << prev << diff << isMidBit << byte << shift << db((uint8_t)data.at(byte)) << db(word);
      prev = bit;
      
      switch (state) {
      case kFindSync:
	//debug_log << isMidBit << diff;
  
	if (isMidBit && 0 != diff) {
	  shifter = (shifter << 1) | bit;

	  // debug_log << (shifter & syncmask) << db(synchro) << synchro << bit; 
	  
	  if ((shifter & syncmask) == synchro) {
	    state = kResync;
	    bitcnt = wordSize;
	    wordcnt = frameSize - 6;
	    word = 0;
	    decoded->append(QByteArray::fromHex("0284016f035c019d020f0095"));

	    //	    frsize = synLength;
	    isCheckMidBit = false;
	    debug_log << "syn found" << byte << byte - oldbyte << (byte - oldbyte)*8/10 << decoded->size();
	    oldbyte = byte;
	  }
	  isMidBit = false;
	} else {
	  isMidBit = true;
	}
	break;

      case kResync:
	if (isMidBit) {
	  word = (word << 1) | bit;
	  if (--bitcnt == 0) {
	    decoded->append(uint8_t(word >> 8));
	    decoded->append(uint8_t(word & 0xff));
	    //	    ++frsize;
	    bitcnt = wordSize;
	    word = 0;
	    if (--wordcnt == 0) {
	      wordcnt = frameSize;
	    }
	  }
	  isMidBit = false;
	} else {
	  isMidBit = true;
	}

	if (isCheckMidBit && 0 != diff) {
	  shifter = (shifter << 1) | bit;
	  if ((shifter & syncmask) == synchro) {
	    debug_log << "resync find" << byte;
	    if (wordcnt != frameSize - 6 || bitcnt != wordSize) {
	      threshold = maxcorr;

	      debug_log << "resync" << threshold << wordcnt << bitcnt << frameSize;
	      //	      debug_log << frsize;

	      if (frameSize - wordcnt < 6) {
		decoded->append(2*(6 - (frameSize - wordcnt)), char(0));
	      } else if (frameSize - wordcnt > 6) {
		decoded->append(2*(6 + frameSize - wordcnt), char(0));
	      }
	      int idx = decoded->size() - 12;
	      decoded->replace(idx, 12, QByteArray::fromHex("0284016f035c019d020f0095"));

	      bitcnt = wordSize;
	      wordcnt = frameSize - 6;
	      word = 0;
	      isMidBit = false;

	      //frsize = synLength;
	    } else {
	      --threshold;
	      debug_log << "resync ok" << threshold;
	    }
	  } else if (wordcnt == frameSize - 6 && bitcnt == wordSize) {
	    threshold = maxcorr;
	  }
	  isCheckMidBit = false;
	} else {
	  isCheckMidBit = true;
	}
	 
	if (threshold == 0) {
	  state = kCheckSync;
	  threshold = maxerr;
	}
	 
	break;


      case kCheckSync:
	if (isMidBit) {
	  word = (word << 1) | bit;
	  if (--bitcnt == 0) {
	    decoded->append(uint8_t(word >> 8));
	    decoded->append(uint8_t(word & 0xff));
	    //	    ++frsize;
	    bitcnt = wordSize;
	    word = 0;
	    wordcnt -= 1;
	    if (wordcnt == 0) {
	      wordcnt = frameSize;
	      //state = kFindSync;
	    }
	  }
	  isMidBit = false;
	} else {
	  isMidBit = true;
	}

	if (isCheckMidBit && 0 != diff) {
	  shifter = (shifter << 1) | bit;
	  if ((shifter & syncmask) == synchro) {
	    if (wordcnt != frameSize - 6 || bitcnt != wordSize) {
	      --threshold;
	      debug_log << "chsync find err" << threshold << wordcnt << bitcnt << byte;
	    } else {
	      threshold = maxerr;
	      //debug_log << "chsync find ok" << threshold << wordcnt << bitcnt << byte << byte - oldbyte << (byte - oldbyte)*8/10;
	      oldbyte = byte;
	    }
	  } else if (wordcnt == frameSize - 6 && bitcnt == wordSize) {
	    --threshold;
	    //debug_log << "chsync lost" << threshold << wordcnt << bitcnt << byte << byte - oldbyte << (byte - oldbyte)*8/10;
	    oldbyte = byte;
	  }
	  isCheckMidBit = false;
	} else {
	  isCheckMidBit = true;
	}
	 
	if (threshold == 0) {
	  state = kResync;
	  // state = kFindSync;
	  debug_log << "syn lost " << byte << byte - oldbyte << (byte - oldbyte)*8/10 << decoded->size();
	  threshold = maxcorr;
	}

	break;
      default:{}
      }
    }
  }
}

//для метеора
void testFindSynchroManchester(const QString& fileName)
{
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);

  QByteArray data = file.readAll(); 
  QByteArray decoded;
  QByteArray synchroba = QByteArray::fromHex("1ACFFC1D");
  uint64_t synchro = 0;
  for (int idx = synchroba.size() - 1, sh = 0; idx >=0; idx--, sh += 8) {
    synchro += ((uint64_t)((uchar)synchroba[idx]) << sh);
  }
  int frameSize = 1024;
  int wordSize = 8;

  trc;

   uint64_t syncmask = 0xFFFFFFFF;
  int syncThresh = 1;
  int resyncThresh = 5;

  meteo::ManchesterDecoder<uint8_t> decoder(synchroba, synchro, syncmask, frameSize, wordSize);
  decoder.setThresholds(syncThresh, resyncThresh);
  decoder.decodeInvert(data, &decoded);

  //  testManchesterDecode(data, synchroba, frameSize, wordSize, &decoded);

  file.close();
  trc;

  file.setFileName("test-mn2.decoded");
  file.open(QIODevice::WriteOnly);
  file.write(decoded);
  file.close();
}
void testFindSynchroManchesterNoaa(const QString& fileName)
{
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);

  QByteArray data = file.readAll(); 
  QByteArray decoded;

  QByteArray sync = QByteArray::fromHex("0A116FD719D83C95");
  //101000010001011011111101011100011001110110000011110010010101
  uint64_t synchro = 0;
  for (int idx = sync.size() - 1, sh = 0; idx >=0; idx--, sh += 8) {
    synchro += ((uint64_t)((uchar)sync[idx]) << sh);
  }

  int frameSize = 11090; 
  int wordSize = 10;
  QByteArray synchroba =QByteArray::fromHex("0284016f035c019d020f0095");
  trc;

  uint64_t syncmask = 0x0FFFFFFFFFFFFFFFLL;
  int syncThresh = 1;
  int resyncThresh = 5;
  meteo::ManchesterDecoder<uint16_t> decoder(synchroba, synchro, syncmask, frameSize, wordSize);
  decoder.setThresholds(syncThresh, resyncThresh);
  decoder.decode(data, &decoded);
  
  // testManchesterDecodeNoaa(data, sync, frameSize, wordSize, &decoded);

  file.close();
  trc;

  file.setFileName("test-noaa.decoded");
  file.open(QIODevice::WriteOnly);
  file.write(decoded);
  file.close();
}

//в кажом байте 1 бит инфы, преобразуем в обычные байтыxs
void testBytesToBits(const QString& fileName)
{
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);
  QByteArray data = file.readAll(); 
  file.close();

  QByteArray decoded;
  uint8_t byte = 0;
  debug_log <<  data.size() / 8 << data.size();

  int bit = 0;

  for (int64_t idx = 0; idx < data.size(); idx ++) {
    byte += data.at(idx) << (7-bit);
    bit++;
    if (bit == 8) {
      decoded.append(byte);
      bit = 0;
      byte = 0;
    }
    // debug_log << idx << idx /8 << ll << data.size() / 8;
    
  }

  var(decoded.size());

  file.setFileName("noaa.manch.bin");
  file.open(QIODevice::WriteOnly);
  file.write(decoded);
  file.close();

}


//-----------------------

bool findSynchro(const QString& fileName)
{
  trc;
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName);
    return false;
  }

  QFile resFile(fileName + ".raw");
  if (!resFile.open(QIODevice::WriteOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName + ".raw");
    return false;
  }


  // file.seek(qint64(1)*180*1024*1024 + 2418*192*8 + 48);
  
  uint64_t oneRead = 500*1024*1024;
  uint64_t dataSize =  file.size();
  uint64_t curSize = oneRead;
  if (curSize > dataSize) {
    curSize = dataSize;
  }
  
  uint64_t fullPos = 0;
   
  bool invert = false;
  //std::string sync = "04B3E375";
  //std::string sync = "0218A7A392DD9ABF"; //метеор
  //std::string sync = "1ACFFC1D"; //mhrp
  //std::string sync = "B06556B7"; //mhrp
  std::string sync = "33157B62"; 

  
  QVector<uint8_t> syncraw;
  for (uint idx = 0; idx < sync.size(); idx += 2) {
    syncraw.append(std::stoul(sync.substr(idx, 2), nullptr, 16));    
  }
  int synclen = syncraw.size();

  var(syncraw);

  QByteArray data;
  QByteArray resData;

  int shift_bit = 0, shift_byte = 0;
  bool ok = false;
  
  while (fullPos < dataSize) {
    
    data = file.read(curSize);
    
    if (data.size() <= synclen) {
      return false;
    }
    
    ok = meteo::findSynchro(data, syncraw.constData(), synclen, &shift_bit, &shift_byte, 0, 0xFF, invert);
    var(ok);
    if (ok) {
      debug_log << "shift_byte" << shift_byte;
    }
    fullPos += curSize;
    
    if (dataSize - fullPos < oneRead) {
      curSize = dataSize - fullPos;
    }    
    
  }

  return ok;
}



void parsewith4(const QString& fileName)
{
  // int skip_bit = 388;

  var(fileName);
  
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName);
    return ;
  }

  QFile resFile(fileName + ".4_");
  if (!resFile.open(QIODevice::WriteOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName + ".4");
    return ;
  }

  
  //file.seek(qint64(1)*180*1024*1024 /*+ 2418*192*8*/);
  
  uint64_t oneRead = 500*1024*1024;
  uint64_t dataSize =  file.size();
  
  uint64_t curSize = oneRead;
  if (curSize > dataSize) {
    curSize = dataSize;
  }

  
  QByteArray data;

  uint64_t pos = 0;
  while (pos < curSize/*dataSize*/) {
    
    data = file.read(curSize);

    var(data.size());

    QByteArray res;

    
    for (int num = 0; (num + 4)< data.size(); num += 4) {
      uint8_t src[4];
      src[0] = data.at(num);
      src[1] = data.at(num + 1);
      src[2] = data.at(num + 2);
      src[3] = data.at(num + 3);

      uint8_t byte[4];
      byte[0] =((src[0] & (1<<7)))      + ((src[0] & (1 << 3)) << 3) +
  	       ((src[1] & (1<<7))>> 2) + ((src[1] & (1 << 3)) << 1) +
	       ((src[2] & (1<<7))>> 4) + ((src[2] & (1 << 3)) >> 1) +
	       ((src[3] & (1<<7))>> 6) + ((src[3] & (1 << 3)) >> 3);

      byte[1] = (src[0] & (1<<6) << 1) + ((src[0] & (1 << 2)) << 4) +
  	       ((src[1] & (1<<6))>> 1) + ((src[1] & (1 << 2)) << 2) +
	       ((src[2] & (1<<6))>> 3) + ((src[2] & (1 << 2)) ) +
	       ((src[3] & (1<<6))>> 5) + ((src[3] & (1 << 2)) >> 2);

      byte[2] =((src[0] & (1<<5)) << 2) + ((src[0] & (1 << 1)) << 5) +
  	       ((src[1] & (1<<5)))     + ((src[1] & (1 << 1)) << 3) +
	       ((src[2] & (1<<5))>> 2) + ((src[2] & (1 << 1)) << 1) +
 	       ((src[3] & (1<<5))>> 4) + ((src[3] & (1 << 1)) >> 1);      

      byte[3] =((src[0] & (1<<4)) << 3) + ((src[0] & (1)) << 6) +
  	       ((src[1] & (1<<4))<< 1) + ((src[1] & (1)) << 4) +
	       ((src[2] & (1<<4))>> 1) + ((src[2] & (1)) << 2) +
	       ((src[3] & (1<<4))>> 3) + ((src[3] & (1)));   

      res.append(byte[0]);
      res.append(byte[1]);
      res.append(byte[2]);
      res.append(byte[3]);
    }    

    
    resFile.write(res);
    
    pos += curSize;
    
    if (dataSize - pos < oneRead) {
      curSize = dataSize - pos;
    }    
  }

  file.close();
  resFile.close();
  
}


void shiftBits(const QString& fileName, int skip)
{
  trc;
  var(skip);
  
  int skip_bit = skip; //388;
  
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName);
    return ;
  }

  QFile resFile(fileName + ".shift_" + QString::number(skip) + "_");
  if (!resFile.open(QIODevice::WriteOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName + ".shift");
    return ;
  }

  
  //file.seek(qint64(1)*180*1024*1024 /*+ 2418*192*8*/);
  
  uint64_t oneRead = 500*1024*1024;
  uint64_t dataSize =  file.size();
  
  uint64_t curSize = oneRead;
  if (curSize > dataSize) {
    curSize = dataSize;
  }

  bool invert = true;
  
  QByteArray data;

  uint64_t pos = 0;
  while (pos < curSize/*dataSize*/) {
    
    data = file.read(curSize);
    var(data.size());

    QByteArray res;
    int remain;

    meteo::cif2raw(data, skip_bit, &res, &remain, 8, invert);

    
    resFile.write(res);
    
    pos += curSize;
    
    if (dataSize - pos < oneRead) {
      curSize = dataSize - pos;
    }
  }

  file.close();
  resFile.close();
  
  debug_log << "ok" << pos;
  
}

void removeHeader(const QString& fileName)
{
  int header = 8;
  int frame_size = 192;
  
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName);
    return ;
  }

  QFile resFile(fileName + ".frame");
  if (!resFile.open(QIODevice::WriteOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName + ".shift");
    return ;
  }

  uint64_t oneRead = (500*1024*1024 / frame_size) * frame_size;
  uint64_t dataSize =  file.size();
  
  uint64_t curSize = oneRead;
  if (curSize > dataSize) {
    curSize = dataSize;
  }

  //  bool invert = false;
  
  QByteArray data;

  uint64_t pos = 0;
  uint64_t cur = 0;
  
  while (pos < dataSize) {
    
    data = file.read(curSize);
    var(data.size());

    QByteArray res;


    while ((cur + frame_size) <= (uint64_t)data.size()) {
      res += data.mid(pos + cur + header, frame_size - header);
      cur += frame_size;
    }
    
    
    resFile.write(res);
    
    pos += curSize;
    
    if (dataSize - pos < oneRead) {
      curSize = dataSize - pos;
    }    
  }

  file.close();
  resFile.close();
  

}

bool createFrames(const QString& fileName)
{
  trc;
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName);
    return false;
  }

  QFile resFile(fileName + ".raw");
  if (!resFile.open(QIODevice::WriteOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName + ".raw");
    return false;
  }


  file.seek(qint64(1)*180*1024*1024 + 2418*192*8);
  
  uint64_t oneRead = 500*1024*1024;
  uint64_t dataSize =  file.size();
  uint64_t curSize = oneRead;
  if (curSize > dataSize) {
    curSize = dataSize;
  }
  
  uint64_t fullPos = 0;
   
  bool invert = false;
  //std::string sync = "04B3E375";
  //std::string sync = "0218A7A392DD9ABF"; //метеор
  std::string sync = "1ACFFC1D"; //mhrp

  int format_size = 192; //байта
  
  QByteArray data;
  QByteArray resData;

  uint header = 8;
  int shift_bit = 4; //, shift_byte = 0;
  int pos = header;
  //  bool ok = false;

  int bit = shift_bit;
  
  while (fullPos < dataSize) {
    
    data = file.read(curSize);
        
    while ((pos + format_size) <= data.size()) {
     
      QByteArray syndata;
      int remain = 0;
      meteo::cif2raw(data.mid(pos, format_size), bit, &syndata, &remain, 8, invert);
 
 
    resData.append(syndata);

    pos += format_size;
    bit = shift_bit;   
  }

  }
  // var(cnt);
  //NOTE: если часть потока, то от pos до конца данные будут потеряны
  
  //  resFile.write(resData);


  file.close();
  resFile.close();
  
  return true;
}


uint8_t dofmBit(uint8_t old, uint8_t bits)
{
  uint8_t res = 0;
  switch (old) {
  case 0: //00 - 1 бит оставляем, 2 - инвертируем
    res = (bits & 0x2) + (~bits & 0x1);
    break;
  case 1: //01 - одинаковые инверт, разные оставляем
    if (bits == 0 || bits == 3) { //00, 11
      res = (~bits) & 0x3;
    } else {
      res = bits;
    }
    break;
  case 2: //10 - 1 - инвертируем, 2 бит оставляем
    res = (~bits & 0x2) + (bits & 0x1);
    break;
  case 3:
    //10 - разные инверт, одинаковые оставляем
    if (bits == 0 || bits == 3) { //00, 11
      res = bits;
    } else {
      res = (~bits) & 0x3;
    }
    break;
  default: {}
  }
  
  return res;
}

QByteArray decodeDofm(const QByteArray& ba)
{
  uchar old = 0;
  QByteArray res(ba.size(), 0);
  
  for (int idx = 0; idx < ba.size(); idx ++) {
    uchar resByte = 0;
    
    uchar byte = ba.at(idx);

    //    qDebug() << hex << byte << bin << byte;
    
    for (int num = 6; num >= 0; num -= 2) {
      uchar bits = (byte >> num) & 0x3;
      // uchar bits = ((byte >> num)) & 0x2;
      // bits += (~(byte >> num)) & 0x1;
      uchar resbits = dofmBit(old, bits);
      resByte |= resbits << num;

      //qDebug() << num << bin << old << bits << resbits << resByte << hex << resByte;
      
      old = bits;
    }

        
    res[idx] = resByte;
  }

  return res;
}


QByteArray decodeOfm(const QByteArray& ba)
{
  uchar old = 0;
  QByteArray res(ba.size(), 0);
  
  for (int idx = 0; idx < ba.size(); idx ++) {
    uchar resByte = 0;
    
    uchar byte = ba.at(idx);

    //    qDebug() << hex << byte << bin << byte;
    
    for (int num = 7; num >= 0; num -= 1) {
      uchar bits = (byte >> num) & 0x1;
      // uchar bits = ((byte >> num)) & 0x2;
      // bits += (~(byte >> num)) & 0x1;
      uchar resbits = 0;
      if ((old == 1 && bits == 0) || (old == 0 && bits == 1)) {
	resbits = 1;
      }

      resByte |= resbits << num;

      //qDebug() << num << bin << old << bits << resbits << resByte << hex << resByte;
      
      old = bits;
    }

        
    res[idx] = resByte;
  }

  return res;
}


void testDecodeDofm(const QString& fileName)
{
  uint64_t oneRead = 500*1024*1024;
  
  if (fileName.isEmpty()) {
    error_log << QObject::tr("Не задан файл с потоком");
    return;
  }
  
  
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName);
    return;
  }

  QFile resFile(fileName + ".cif");
  if (!resFile.open(QIODevice::WriteOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName + ".cif");
    return;
  }

 
  uint64_t dataSize = file.size();

  //  dataSize = 10;
  
  uint64_t curSize = oneRead;
  if (curSize > dataSize) {
    curSize = dataSize;
  }

  QByteArray data;

  uint64_t pos = 0;
  while (pos < dataSize) {
    
    data = file.read(curSize);
    var(data.size());

    // QByteArray res = decodeOfm(data);
    QByteArray res = decodeDofm(data);
    resFile.write(res);
    
    pos += curSize;
    
    if (dataSize - pos < oneRead) {
      curSize = dataSize - pos;
    }    
  }

  file.close();
  resFile.close();
}

//-------------------------------

int main(int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");

  int flag = 0, opt;
  bool invert = false;

  while ((opt = getopt(argc, argv, "123456789ih")) != -1) {
    switch (opt) {
    case '1':
      flag = 1;
      break;
    case '2':
      flag = 2;
      break;
    case '3':
      flag = 3;
      break;
    case '4':
      flag = 4;
      break;
    case '5':
      flag = 5;
      break;
    case '6':
      flag = 6;
      break;
    case '7':
      flag = 7;
      break;
    case '8':
      flag = 8;
      break;
    case '9':
      flag = 9;
      break;      
    case 'i':
      invert = true;;
      break;
    case 'h':
      info_log<<"Usage:"<<argv[0]<<"-1|-2|-3|-4|-5|-6|-7|-8|-9 [-i] <fileName>\n"
	"-1 - find MN2 synchro, save raw\n"
	"-2 - manchester decode, find MN2 synchro, save raw\n"
	"-3 - manchester decode, find NOAA synchro, save raw\n"
	"-4 - bytes to bits\n"
	"-5 - раскодирование ДОФМ\n"
	"-i - need invert bits\n"
	"fileName - raw satellite data\n";
      exit(0);
      break;
    default: /* '?' */
      error_log<<"Option"<<opt<<"not realized";
      exit(-1);
    }
  }

  if (flag == 0) {
    error_log<<"Need select test with option. \""<<argv[0]<<"-h\" for help";
    exit(-1);
  }

  QString fileName;
  if (flag <= 9) {
    if (optind >= argc) {
      error_log<<"Need point file name. \""<<argv[0]<<"-h\" for help";
      exit(-1);
    } else {
      fileName = argv[optind];
    }
  }

  QCoreApplication app(argc, argv);

  switch (flag) {
  case 1:
    testFindSynchro(fileName, invert);
    break;

  case 2:
    testFindSynchroManchester(fileName);
    break;

  case 3:
    testFindSynchroManchesterNoaa(fileName);
    break;

  case 4:
    testBytesToBits(fileName);
    break;

  case 5:
    testDecodeDofm(fileName);
    break;
  case 6:
    //  for (int skip = 1; skip <= 32; skip++ ) {
      shiftBits(fileName, 0);
      // }
    
    break;
  case 7:
    removeHeader(fileName);
    break;
  case 8:
    findSynchro(fileName);
    break;
  case 9:
    parsewith4(fileName);
    break;            
  default: {}
  }

  return 0;
}
