#include "meteo_convert.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/sigwx.pb.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

namespace meteo {
  namespace decoders {
    bool convert(descr_t descr, int syn, const TMeteoData& data, QList<QByteArray>* ba, QList<sigwx::Header>* ahead);
    bool readHeader(descr_t descr, int syn, const TMeteoData& data, sigwx::Header* header);
    bool readData(sigwx::SigFront* sig, const TMeteoData& data);
    bool readData(sigwx::SigJetStream* sig, const TMeteoData& data);
    bool readData(sigwx::SigCloud* sig, const TMeteoData& data);
    bool readData(sigwx::SigTurbulence* sig, const TMeteoData& data);
    bool readData(sigwx::SigVolcano* sig, const TMeteoData& data);
    bool readData(sigwx::SigTropo* sig, const TMeteoData& data);
    bool readData(sigwx::SigStorm* sig, const TMeteoData& data);
    void setSkyPhen(sigwx::Cloud* cloud, const TMeteoData& data);

    template<class T> bool convertSig(descr_t descr, int syn, const TMeteoData& data,
              sigwx::SigWx* fsig, T* sig,
              QList<QByteArray>* ba, QList<sigwx::Header>* ahead
              )
    {
      if (0 == sig || fsig == 0) return false;

      sigwx::Header* header = sig->mutable_header();
      bool ok = readHeader(descr, syn, data, header);
      if (!ok) return false;

      ok = readData(sig, data);

      if (!fsig->IsInitialized()) {
  error_log << QObject::tr("Заполнены не все обязательные поля");
  return false;
      }

      std::string str;
      fsig->SerializeToString(&str);

      ahead->append(sigwx::Header(*header));
      ba->append(QByteArray(str.data(), str.size()));
      return ok;
    }


  }
}

using namespace meteo;


//! Преобразование SigWx (BUFR) из TMeteoData в proto
bool decoders::toSigwxStructs(const TMeteoData& data, QList<QByteArray>* ba, QList<sigwx::Header>* header)
{
  if (0 == ba || 0 == header) {
    return false;
  }

  bool ok = true;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  descr_t descr = d->descriptor("syn_sign");
  int syn = data.getValue(descr, -1);

  if (-1 == syn) {
    int vs = data.getValue(d->descriptor("vertic_sign"), -1);
    if (vs != -1 && (vs & (0x10))) {
      syn = 64; //тропопауза
    }
  }
  if (-1 == syn) {
    descr = d->descriptor("syn_type");
    syn = data.getValue(descr, -1); //синоптическая характеристика
  }

  if (syn == -1) return true; //пустое сообщение

  ok = convert(descr, syn, data, ba, header);

  return ok;
}


bool decoders::convert(descr_t descr, int syn, const TMeteoData& data, QList<QByteArray>* ba, QList<sigwx::Header>* header)
{
  sigwx::SigWx fsig;

  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  if (descr == d->descriptor("syn_type")) {
    sigwx::SigStorm* sig = fsig.add_storm();
    return convertSig<sigwx::SigStorm>(descr, syn, data, &fsig, sig, ba, header);
  }

  if (descr != d->descriptor("syn_sign")) {
    return false;
  }

  bool ok = false;

  switch (syn) {
  case sigwx::kQFront:    case sigwx::kQFrontAbv:    case sigwx::kWFront:
  case sigwx::kWFrontAbv: case sigwx::kCFront:       case sigwx::kCFrontAbv:
  case sigwx::kOcclus:    case sigwx::kInstabilLine: case sigwx::kInTtropFront:
  case sigwx::kConvergLine: {
    sigwx::SigFront* sig = fsig.add_front();
    ok = convertSig<sigwx::SigFront>(descr, syn, data, &fsig, sig, ba, header);
  }
    break;
  case sigwx::kJetStream: {
    sigwx::SigJetStream* sig = fsig.add_jet();
    ok = convertSig<sigwx::SigJetStream>(descr, syn, data, &fsig, sig, ba, header);
  }
    break;
  case sigwx::kCloud: {
    sigwx::SigCloud* sig = fsig.add_cloud();
    ok = convertSig<sigwx::SigCloud>(descr, syn, data, &fsig, sig, ba, header);
  }
    break;
  case sigwx::kTurb: {
    sigwx::SigTurbulence* sig = fsig.add_turb();
    ok = convertSig<sigwx::SigTurbulence>(descr, syn, data, &fsig, sig, ba, header);
  }
    break;
  case sigwx::kVolcano: {
    sigwx::SigVolcano* sig = fsig.add_volcano();
    ok  = convertSig<sigwx::SigVolcano>(descr, syn, data, &fsig, sig, ba, header);
    descr = d->descriptor("syn_type");
    syn = data.getValue(descr, -1); //синоптическая характеристика
    if (syn != -1) {
      sigwx::SigWx fsig1;
      sigwx::SigStorm* sig = fsig1.add_storm();
      ok |= convertSig<sigwx::SigStorm>(descr, syn, data, &fsig1, sig, ba, header);
    }
  }
    break;
  case sigwx::kTropo: {
    sigwx::SigTropo* sig = fsig.add_tropo();
    ok = convertSig<sigwx::SigTropo>(descr, syn, data, &fsig, sig, ba, header);
  }
    break;
  default: {
    ok = false;
  }
  }

  return ok;
}

//! Выделение данных заголовка
bool decoders::readHeader(descr_t descr, int syn, const TMeteoData& data, sigwx::Header* header)
{
  if (0 == header) return false;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  header->set_descr(descr);
  header->set_value(syn);

  header->set_center(data.getValueCur(d->descriptor("center"), -1));
  header->set_h_lo(data.getValueCur(d->descriptor("h0"), -1));
  header->set_h_hi(data.getValueCur(d->descriptor("h0"), 100, -1));
  if (header->h_lo() == header->h_hi()) {
    header->set_lev_type(meteodescr::kHeightLevel);
  } else {
    header->set_lev_type(meteodescr::kHeightLevel);
  }

  int vtype = -1;
  QDateTime dt1, dt2;
  meteodescr::TimeParam tp_cur;
  d->findTime(data, &tp_cur, &vtype, &dt1, &dt2);
  header->set_dt_beg(dt1.toString("yyyy-MM-dd hh:mm:ss").toStdString());
  header->set_dt_end(dt2.toString("yyyy-MM-dd hh:mm:ss").toStdString());

  return true;
}

//! Фронты
bool decoders::readData(sigwx::SigFront* sig, const TMeteoData& data)
{
  if (0 == sig) return false;

  QList<const TMeteoData*> fd = data.findChildsConst(TMeteoDescriptor::instance()->descriptor("syn_sign"));
  for (int idx = 0; idx < fd.size(); idx++) {
    sigwx::Front* front = sig->add_front();
    front->set_phen(fd.at(idx)->getValueCur(TMeteoDescriptor::instance()->descriptor("syn_sign"), BAD_METEO_ELEMENT_VAL));
    const QList<TMeteoData*>& child = fd.at(idx)->childs();
    for (int chIdx = 0; chIdx < child.size(); chIdx++) {
      float lat = child.at(chIdx)->getValueCur(TMeteoDescriptor::instance()->descriptor("La"), BAD_METEO_ELEMENT_VAL);
      float lon = child.at(chIdx)->getValueCur(TMeteoDescriptor::instance()->descriptor("Lo"), BAD_METEO_ELEMENT_VAL);
      if (lat == BAD_METEO_ELEMENT_VAL || lon == BAD_METEO_ELEMENT_VAL) {
  continue;
      }
      sigwx::FrontPoint* pnt = front->add_pnt();
      pnt->set_lat(lat);
      pnt->set_lon(lon);

      float dd = child.at(chIdx)->getValueCur(TMeteoDescriptor::instance()->descriptor("dsds"), BAD_METEO_ELEMENT_VAL);
      float ff = child.at(chIdx)->getValueCur(TMeteoDescriptor::instance()->descriptor("fsfs"), BAD_METEO_ELEMENT_VAL);
      if (dd != BAD_METEO_ELEMENT_VAL && ff != BAD_METEO_ELEMENT_VAL) {
  pnt->set_dd(dd);
  pnt->set_ff(ff);
      }
    }
  }

  return true;
}

//! Струйные течения
bool decoders::readData(sigwx::SigJetStream* sig, const TMeteoData& data)
{
  if (0 == sig) return false;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  QList<const TMeteoData*> fd = data.findChildsConst(d->descriptor("syn_sign"));
  for (int idx = 0; idx < fd.size(); idx++) {
    sigwx::JetStream* jet = sig->add_jet();
    const QList<TMeteoData*>& child = fd.at(idx)->childs();
    for (int chIdx = 0; chIdx < child.size(); chIdx++) {
      float lat = child.at(chIdx)->getValueCur(d->descriptor("La"), BAD_METEO_ELEMENT_VAL);
      float lon = child.at(chIdx)->getValueCur(d->descriptor("Lo"), BAD_METEO_ELEMENT_VAL);
      if (lat == BAD_METEO_ELEMENT_VAL || lon == BAD_METEO_ELEMENT_VAL) {
  continue;
      }

      sigwx::JetPoint* pnt = jet->add_pnt();
      pnt->set_lat(lat);
      pnt->set_lon(lon);

      float ff = child.at(chIdx)->getValueCur(d->descriptor("ff"), BAD_METEO_ELEMENT_VAL);
      if (ff != BAD_METEO_ELEMENT_VAL) {
  pnt->set_ff(ff);
      }
      float fl = child.at(chIdx)->getValueCur(d->descriptor("fl_lev"), BAD_METEO_ELEMENT_VAL);
      if (fl != BAD_METEO_ELEMENT_VAL) {
  pnt->set_fl(fl);
      }
      if (child.at(chIdx)->childsCount() != 0) {
  const TMeteoData* flch = child.at(chIdx)->child(0);
  if (0 != flch) {
    float fl_abv = flch->getValueCur(d->descriptor("fl_lev"), BAD_METEO_ELEMENT_VAL);
    if (BAD_METEO_ELEMENT_VAL != fl_abv) {
      pnt->set_fl_abv(fl_abv);
    }
    float fl_blw = flch->getValueCur(d->descriptor("fl_lev"), 100, BAD_METEO_ELEMENT_VAL);
    if (BAD_METEO_ELEMENT_VAL != fl_abv) {
      pnt->set_fl_blw(fl_blw);
    }
  }
      }
    }
  }

  return true;
}

//! Заполнение облачности
void decoders::setSkyPhen(sigwx::Cloud* cloud, const TMeteoData& data)
{
  if (0 == cloud) return;

  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  float hLo = data.getValueCur(d->descriptor("h0"), BAD_METEO_ELEMENT_VAL);
  if (hLo == BAD_METEO_ELEMENT_VAL) {
    hLo = data.getValueCur(d->descriptor("fl_lev"), BAD_METEO_ELEMENT_VAL);
  }
  float hHi = data.getValueCur(d->descriptor("h0"), 100, BAD_METEO_ELEMENT_VAL);
  if (hHi == BAD_METEO_ELEMENT_VAL) {
    hHi =  data.getValueCur(d->descriptor("fl_lev"), 100, BAD_METEO_ELEMENT_VAL);
  }

  float Na = data.getValueCur(d->descriptor("Na"), BAD_METEO_ELEMENT_VAL);
  float C  = data.getValueCur(d->descriptor("C"),  BAD_METEO_ELEMENT_VAL);
  float B  = data.getValueCur(d->descriptor("B"),  BAD_METEO_ELEMENT_VAL);
  float Ic = data.getValueCur(d->descriptor("Ic"), BAD_METEO_ELEMENT_VAL);

  if (Na != BAD_METEO_ELEMENT_VAL || C  != BAD_METEO_ELEMENT_VAL ||
      B  != BAD_METEO_ELEMENT_VAL || Ic != BAD_METEO_ELEMENT_VAL) {

    sigwx::SkyPhen* phen = cloud->add_phen();

    phen->set_h_lo(hLo);
    phen->set_h_hi(hHi);

    if (Na != BAD_METEO_ELEMENT_VAL) {
      phen->set_cloud_amount(Na);
    }

    if (C != BAD_METEO_ELEMENT_VAL) {
      phen->set_cloud(C);
    }

    if (B != BAD_METEO_ELEMENT_VAL) {
      phen->set_turb(B);
    }

    if (Ic != BAD_METEO_ELEMENT_VAL) {
      phen->set_ice(Ic);
    }
  }

}

//! Облачность
bool decoders::readData(sigwx::SigCloud* sig, const TMeteoData& data)
{
  if (0 == sig) return false;

  const QList<TMeteoData*>& fd = data.childs();
  //QList<const TMeteoData*> fd = data.findChilds(d->descriptor("syn_sign"));
  for (int idx = 0; idx < fd.size(); idx++) {
    sigwx::Cloud* cloud = sig->add_cloud();

    setSkyPhen(cloud, *fd.at(idx)); //в SWH данных

    const QList<TMeteoData*>& child = fd.at(idx)->childs();
    for (int chIdx = 0; chIdx < child.size(); chIdx++) {
      float lat = child.at(chIdx)->getValueCur(TMeteoDescriptor::instance()->descriptor("La"), BAD_METEO_ELEMENT_VAL);
      float lon = child.at(chIdx)->getValueCur(TMeteoDescriptor::instance()->descriptor("Lo"), BAD_METEO_ELEMENT_VAL);
      if (lat != BAD_METEO_ELEMENT_VAL && lon != BAD_METEO_ELEMENT_VAL) {
  GeoPointPb* pnt = cloud->add_pnt();
  pnt->set_lat_deg(lat);
  pnt->set_lon_deg(lon);
      }

      setSkyPhen(cloud, *child.at(chIdx));//в SWM данных
    }
  }

  return true;
}

//! Турбулентность
bool decoders::readData(sigwx::SigTurbulence* sig, const TMeteoData& data)
{
  if (0 == sig) return false;

  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  QList<const TMeteoData*> td = data.findChildsConst(d->descriptor("syn_sign"));
  for (int idx = 0; idx < td.size(); idx++) {
    sigwx::Turbulence* turb = sig->add_turb();
    float h_lo = td.at(idx)->getValueCur(d->descriptor("h0"), BAD_METEO_ELEMENT_VAL);
    float h_hi = td.at(idx)->getValueCur(d->descriptor("h0"), 100, BAD_METEO_ELEMENT_VAL);
    turb->set_h_lo(h_lo);
    turb->set_h_hi(h_hi);

    int B = td.at(idx)->getValueCur(d->descriptor("B"), BAD_METEO_ELEMENT_VAL);
    if (B != BAD_METEO_ELEMENT_VAL) {
      turb->set_degree(B);
    }

    const QList<TMeteoData*>& child = td.at(idx)->childs();
    for (int chIdx = 0; chIdx < child.size(); chIdx++) {
      float lat = child.at(chIdx)->getValueCur(d->descriptor("La"), BAD_METEO_ELEMENT_VAL);
      float lon = child.at(chIdx)->getValueCur(d->descriptor("Lo"), BAD_METEO_ELEMENT_VAL);
      if (lat == BAD_METEO_ELEMENT_VAL || lon == BAD_METEO_ELEMENT_VAL) {
  continue;
      }
      GeoPointPb* pnt = turb->add_pnt();
      pnt->set_lat_deg(lat);
      pnt->set_lon_deg(lon);
    }

  }

  return true;
}

//! Извержение вулкана
bool decoders::readData(sigwx::SigVolcano* sig, const TMeteoData& data)
{
  if (0 == sig) return false;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  QList<const TMeteoData*> td = data.findChildsConst(d->descriptor("syn_sign"));
  for (int idx = 0; idx < td.size(); idx++) {
    float lat = td.at(idx)->getValue(d->descriptor("La"), BAD_METEO_ELEMENT_VAL);
    float lon = td.at(idx)->getValue(d->descriptor("Lo"), BAD_METEO_ELEMENT_VAL);
    if (lat == BAD_METEO_ELEMENT_VAL || lon == BAD_METEO_ELEMENT_VAL) {
      continue;
    }
    QString name = td.at(idx)->getCode(d->descriptor("syn_name"), false);
    sigwx::Volcano* volcano = sig->add_volcano();
    volcano->mutable_pnt()->set_lat_deg(lat);
    volcano->mutable_pnt()->set_lon_deg(lon);
    if (!name.isEmpty()) {
      volcano->set_name(name.toStdString());
    }
    meteodescr::MeteoDateTime dt(true);
    dt.readUp(*(td.at(idx)), d->descriptor("JJJ"),  d->descriptor("MM"), d->descriptor("YY"),
        d->descriptor("GG"), d->descriptor("gg"), d->descriptor("sec"), 0);
    QDateTime start(QDate(dt.year, dt.month, dt.day), QTime(dt.hour, dt.min, dt.sec));
    if (start.isValid()) {
      volcano->set_dt(start.toString(Qt::ISODate).toStdString());
    }
  }

  return true;
}

//! Тропопауза
bool decoders::readData(sigwx::SigTropo* sig, const TMeteoData& data)
{
  if (0 == sig) return false;

  QList<const TMeteoData*> td = data.findChildsConst(TMeteoDescriptor::instance()->descriptor("vertic_sign"));
  for (int idx = 0; idx < td.size(); idx++) {
    int sign = td.at(idx)->getValueCur(TMeteoDescriptor::instance()->descriptor("stat_sign"), BAD_METEO_ELEMENT_VAL);
    const QList<TMeteoData*>& child = td.at(idx)->childs();
    for (int chIdx = 0; chIdx < child.size(); chIdx++) {
      GeoPointPb* pnt = 0;
      switch (sign) {
      case 3: //мин
  pnt = sig->add_tmin();
  break;
      case 2: //макс
  pnt = sig->add_tmax();
  break;
      default: {
  pnt = sig->add_tropo();
      }
      }
      float lat = child.at(chIdx)->getValueCur(TMeteoDescriptor::instance()->descriptor("La"), BAD_METEO_ELEMENT_VAL);
      float lon = child.at(chIdx)->getValueCur(TMeteoDescriptor::instance()->descriptor("Lo"), BAD_METEO_ELEMENT_VAL);
      float alt = child.at(chIdx)->getValueCur(TMeteoDescriptor::instance()->descriptor("hh0"), BAD_METEO_ELEMENT_VAL);
      if (lat == BAD_METEO_ELEMENT_VAL || lon == BAD_METEO_ELEMENT_VAL) {
  continue;
      }
      pnt->set_lat_deg(lat);
      pnt->set_lon_deg(lon);
      if (alt != BAD_METEO_ELEMENT_VAL) {
  pnt->set_height_meters(alt);
      }
    }
  }

  return true;
}

//! Шторм
bool decoders::readData(sigwx::SigStorm* sig, const TMeteoData& data)
{
  if (0 == sig) return false;

  QList<const TMeteoData*> td = data.findChildsConst(TMeteoDescriptor::instance()->descriptor("syn_type"));
  for (int idx = 0; idx < td.size(); idx++) {
    float lat = td.at(idx)->getValueCur(TMeteoDescriptor::instance()->descriptor("La"), BAD_METEO_ELEMENT_VAL);
    float lon = td.at(idx)->getValueCur(TMeteoDescriptor::instance()->descriptor("Lo"), BAD_METEO_ELEMENT_VAL);
    if (lat == BAD_METEO_ELEMENT_VAL || lon == BAD_METEO_ELEMENT_VAL) {
      continue;
    }
    sigwx::Storm* storm = sig->add_storm();
    storm->mutable_pnt()->set_lat_deg(lat);
    storm->mutable_pnt()->set_lon_deg(lon);
    int sign = td.at(idx)->getValueCur(TMeteoDescriptor::instance()->descriptor("meteo_sign"), BAD_METEO_ELEMENT_VAL);
    if (sign != BAD_METEO_ELEMENT_VAL) {
      storm->set_sign(sign);
    }
    QString name = td.at(idx)->getCode(TMeteoDescriptor::instance()->descriptor("storm_name"), false);
    if (!name.isEmpty()) {
      storm->set_name(name.toStdString());
    }
  }

  return true;
}

