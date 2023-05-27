#include "clouddata.h"

#include <cross-commons/debug/tlog.h>

namespace zond
{

  QDataStream &operator>>(QDataStream &stream, CloudData &u)
  {
    stream >> u._Nh;
    stream >> u._N;
    stream >> u._h;
    stream >> u._CL;
    stream >> u._CM;
    stream >> u._CH;
    return stream;
  }

  QDataStream &operator<<(QDataStream &stream, const CloudData &u)
  {
    stream << u._Nh;
    stream << u._N;
    stream << u._h;
    stream << u._CL;
    stream << u._CM;
    stream << u._CH;
    return stream;
  }

CloudData::CloudData()
{
    clear();
};

void CloudData::set(CloudDataType type, float value, int quality)
{
    pair(type)->first = value;
    pair(type)->second = quality;
};

void CloudData::setC(float value, int quality)
{
  if( MnMath::isEqual(value, 60) || value < 20){ //CH
  //  if(value == 60 )value =0;
    set(CH, value,quality);
  }
  if(MnMath::isEqual(value, 61) || (value < 30 && value > 19) ){ //CM
//    if(value == 61 )value =0;
    set(CM, value,quality);
  }
  if(MnMath::isEqual(value, 62) ||(value < 40 && value > 29)){ //CL
    //if(value == 62 )value =0;
    set(CL, value,quality);
  }

};

float CloudData::value(CloudDataType type)
{
    int value;
    if (isGood(type)) {
        value = pair(type)->first;
    } else {
        value = 0;
    }
    return value;
};

bool CloudData::isGood(CloudDataType type)
{
    return pair(type)->second <= control::DOUBTFUL;
};

void CloudData::print()
{
  error_log << "N value = " << pair(N)->first << ", quality = " << pair(N)->second;
  error_log << "Nh value = " << pair(Nh)->first << ", quality = " << pair(Nh)->second;
  error_log << "h value = " << pair(h)->first << ", quality = " << pair(h)->second;
    error_log << "CH value = " << pair(CH)->first << ", quality = " << pair(CH)->second;
    error_log << "CM value = " << pair(CM)->first << ", quality = " << pair(CM)->second;
    error_log << "CL value = " << pair(CL)->first << ", quality = " << pair(CL)->second;
};

void CloudData::clear()
{
    for (int i = h; i < C_LAST; ++i) {
        pair(static_cast<CloudDataType>(i))->first = 0;
        pair(static_cast<CloudDataType>(i))->second = control::NO_OBSERVE;
    }
};

QPair<float, int> *CloudData::pair(CloudDataType type)
{
    switch (type) {
    case N:
        return &_N;
    case Nh:
        return &_Nh;
    case h:
        return &_h;
    case CL:
        return &_CL;
    case CM:
        return &_CM;
    case CH:
        return &_CH;
    default:
        return new QPair<float, int>();
    }
}

}//zond
