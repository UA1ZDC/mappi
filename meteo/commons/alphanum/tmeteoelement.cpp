#include "tmeteoelement.h"
#include "metadata.h"

#include <cross-commons/app/paths.h>
#include <commons/mathtools/mnmath.h>
#include <commons/geobasis/geovector.h>
#include <cross-commons/debug/tmap.h>

#include <commons/meteo_data/tmeteodescr.h>
#include <cross-commons/debug/tlog.h>

#include <qfile.h>
#include <qdom.h>

#include <math.h>

//TODO c0c0

#define BAD_VALUE BAD_METEO_ELEMENT_VAL
#define METEO_ELEMENT_FILE MnCommon::etcPath("meteo") + "/decoders/andescriptors.xml"

namespace meteo {
  float getValue_V(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_VV(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_Vgmc(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_fiw(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_R(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_R1(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_T(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_snT(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_ssTw(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_ssTa(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_Tpms(const TMeteoData* data,const QString& strelem, bool* ok);
  //float getValue_snTg(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_sndT(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_U(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_hshs(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_hi(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_tt(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_RR(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_P(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_P1(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_Pt(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_PtC(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_D(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_dt(const TMeteoData* data,const QString& strelem,bool* ok);
  float getValue_ft(const TMeteoData* data,const QString& strelem,bool* ok);
  float getValue_P1hA(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_P1hC(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_n1(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_eR(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_La(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_Lo(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_trash(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_Lagms(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_Logms(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_LaLo_v(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_LaGr(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_LoGr(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_c0c0(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_hshshs(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_zn(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_Tn10(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_Tn100(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_bbbb(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_tt0(const TMeteoData* data,const QString& strelem, bool* ok);
  //  float getValue_dtdh(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_dtableP(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_kq(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_kq1(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_hik(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_snTT(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_dd(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_Ns(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_w(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_W(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_h0(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_status(const TMeteoData* data,const QString& strelem, bool* ok);
  //  float getValue_iff(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_tt906(const TMeteoData* data,const QString& strelem, bool* ok);
  float getValue_ppp(const TMeteoData* data,  const QString& strelem, bool* ok);
  float getValue_gk_p(const TMeteoData* data, const QString& strelem, bool* ok);
  float getValue_gFf(const TMeteoData* data,  const QString& strelem, bool* ok);
  float getValue_gpi(const TMeteoData* data,  const QString& strelem, bool* ok);
  float getValue_gdst(const TMeteoData* data, const QString& strelem, bool* ok);
  float getValue_gic(const TMeteoData* data, const QString& strelem, bool* ok);
  float getValue_bbih(const TMeteoData* data, const QString& strelem, bool* ok);

  bool parse_wind_pilot(const QString& strelem, TMeteoData* data);
  bool parse_P1seq(const QString& name, const QString& strelem, TMeteoData* data);
  bool getValue_ccum(TMeteoData* data,const QString& strelem);
  bool getValue_region(TMeteoData* data,const QString& strelem);
  bool getValue_P0fract(TMeteoData* data, const QString& strelem);

  bool getValue_regionWI(TMeteoData* data, const QString& strelem);
  bool getValue_regionLine(TMeteoData* data, const QString& strelem);
  bool getValue_regionPart(TMeteoData* data, const QString& strelem);
  bool getValue_ddtr(TMeteoData* data, const QString& strelem);
  bool getValue_hshsf(TMeteoData* data, const meteo::Descriptor& cur, TMeteoData* curChild, const QString& strelem);
  bool getValue_qhshshs(TMeteoData* data,const QString& strelem);
}


//загружает большой объём данных
MeteoElement::MeteoElement():
  _data(0)
{
  registerGetValueFunc();
  readSettings( METEO_ELEMENT_FILE);
}

MeteoElement::~MeteoElement() 
{
}

QString MeteoElement::outName(const QString& name) const
{
  if (!_descr.contains(name)) {
    error_log << QObject::tr("Неизвестное внутреннее название дескриптора %1").arg(name);
    return "";
  }
  return _descr.value(name).outname;  
}

//! получение номера дескриптора для сохранения значения
/*! 
  \param name внутреннее имя
  \return номер дескриптора
*/
descr_t MeteoElement::getDescriptor(const QString& name) const
{
  if (!_descr.contains(name)) {
    error_log << QObject::tr("Неизвестное внутреннее название дескриптора %1").arg(name);
    return -1;
  }
  
  return  TMeteoDescriptor::instance()->descriptor(_descr.value(name).outname);
}

//! тип преобразования
meteo::ElementConvertType MeteoElement::convertType(const QString& name) const
{
  if (!_descr.contains(name)) {
    error_log << QObject::tr("Неизвестное внутреннее название дескриптора %1").arg(name);
    return meteo::CONVERT_CNT;
  }
  return _descr.value(name).type;    
}


float MeteoElement::getValue(const QString& name, const QString& strelem, control::QualityControl* qual) const
{
  bool ok;
  *qual = control::NO_CONTROL;

  if (!_descr.contains(name)) {
    *qual = control::MISTAKEN;
    error_log<<QObject::tr("Нет дескриптора %1").arg(name);
    return BAD_VALUE;
  }


  //debug_log << strelem;

  const meteo::Descriptor& cur = _descr.value(name);
  ok = semanticControl(name, strelem);
  if (!ok) {
    if( !strelem.isEmpty() && !strelem.contains('/') && !strelem.contains('-') && 
	strelem != "BAD" && strelem != "VRB") {
#ifdef PARSE_LOG 
      warning_log<<QObject::tr("Не пройден семантический контроль. Дескр=%1(%2) Значение=*%3* Рег.выр=*%4*").
       	arg(cur.outname).arg(name).arg(strelem).arg(cur.rx.pattern()); //TODO расскомментировать
#endif
    }
    *qual = control::NO_OBSERVE;
    return BAD_VALUE;
  }

  float val = BAD_VALUE;
  
  //var(cur.type);

  switch (cur.type) {
  case meteo::SIMPLE_CONVERT:
    val = cur.mul * strelem.toFloat(&ok) + cur.add;
    if (!ok) {
      //ничего, т.к. возможно значение было строковым
    }
    break;
  case meteo::TABLE_CONVERT: {
    val = BAD_VALUE;
    QString code = strelem.trimmed().remove(QRegExp("^0*(?=0$)"));
    //debug_log << strelem << code;
    if (cur.table.contains(code)) {
      val = cur.table.value(code);
      *qual = control::NO_CONTROL;
    } else if (strelem.contains('/')) {
      *qual = control::NO_OBSERVE;
    } else {
      *qual = control::MISTAKEN;
    }

    // if (ok || strelem.contains('/')) {
    //   if (cur.table.contains(code)) {
    // 	val = cur.table.value(code);
    //   }
    //   *qual = control::RIGHT;
    // } else {
    //   error_log << QObject::tr("Ошибка табличного преобразования дескр=%1(%2) code=%3").arg(cur.outname).arg(name).arg(strelem);
    //   *qual = control::MISTAKEN;
    // }
  }
    break;
  case meteo::CUSTOM_CONVERT:
    val = customConvert(name, strelem, &ok);
    if (ok) {
      *qual = control::NO_CONTROL; //отсутствующее значение, тоже верное
    } else {
      *qual = control::NO_OBSERVE;
    }
    break;
  default: 
    val = strelem.toFloat(&ok);
    if (!ok) {
      //ничего, т.к. возможно значение было строковым
    }
  }

#ifdef PARSE_DEBUG_LOG
  debug_log<<name<<"outname="<<cur.outname<<cur.rx.pattern()<<"val="<<val;
#endif

  // if (name == "UNK") {
  //   var(strelem);
  // }
  
  return val;

}

float MeteoElement::customConvert(const QString& name, const QString& strelem , bool* ok) const
{
  if (!_funcs.contains(name)) {
    *ok = false;
    return BAD_VALUE;
  }

  return _funcs.value(name)(_data, strelem, ok);
}

void MeteoElement::readSettings(const QString& fileName)
{  
  QFile file(fileName);
  if ( ! file.open(QIODevice::ReadOnly) ) {
    error_log<<QString("Ошибка открытия файла %1").arg(fileName);
    return;
  }

  QDomDocument* xml = new QDomDocument;
  if (xml->setContent(&file)) {
    readSettings(xml);
  }

  file.close();
  delete xml;
}

void MeteoElement::readSettings(QDomDocument* dom)
{
  QDomNode node = dom->documentElement().firstChild();

  // QHash<QString, QString> tmplist;

  while (!node.isNull()) {
    if (!node.isElement()) {
      node = node.nextSibling();
      continue;
    }
    QDomElement el = node.toElement();
    //debug_log<<el.tagName()<<el.attribute("name")<<el.attribute("num")<<el.attribute("regexp");

    QString name = el.attribute("name");
    meteo::Descriptor d;
    d.outname = el.attribute("outname");
    if ( d.outname.isEmpty() || name.isEmpty()) {
      error_log << QObject::tr("Не верный дескриптор: название=%1 внешнее=%2").arg(name).arg(d.outname);
      node = node.nextSibling();
      continue;
    }
    
    //TMeteoDescriptor::instance()->descriptor(d.outname);
    
    d.rx = QRegExp(el.attribute("regexp"));
    d.type = meteo::ElementConvertType(el.attribute("type").toInt());
    if (d.type == 2) {
      if (!_funcs.contains(name)) {
#ifdef PARSE_LOG 
	warning_log << QObject::tr("Нет функции для %1, внешнее=%2").arg(name).arg(d.outname);
#endif
      }
    }

    d.mul = el.attribute("mul").toFloat();
    if (d.mul == 0) {
      d.mul = 1;
    }

    d.add = el.attribute("add").toFloat();
    //    d.group = el.attribute("group").toUInt();

    if (el.hasChildNodes()) {
      el = el.firstChild().toElement();
      QStringList sList = el.attribute("val").split(";");
      for (int i = 0; i < sList.count(); i++) {
	QString code = QString::number(i);
	float val = sList.at(i).section(',', -1, -1).toFloat();
	if (sList.at(i).contains(',')) {
	  code = sList.at(i).section(',', -2, -2).trimmed();
	}
	d.table.insert(code, val);
      }
    }

    if (_descr.contains(name)) {
      error_log << QObject::tr("Дескриптор %1(%2) уже существует %3(%4)").
	arg(name).arg(d.outname).arg(name).arg(_descr.value(name).outname);
    }

    _descr.insert(name, d);
    
    // if (tmplist.contains(d.outname)) {
    //   debug_log << QString("Descriptor repeated %1(%2 - %3)").arg(d.outname).arg(tmplist[d.outname]).arg(name);
    // } else {
    //   tmplist.insert(d.outname, name);
    // }

    node = node.nextSibling();
  }
}

bool MeteoElement::parseComplex(const QString& name, const QString& str, TMeteoData* data) const
{
  if (name == "wind_pilot") {
    return meteo::parse_wind_pilot(str, data);
  } else if (name == "P1_seqA" || name == "P1_seqC") {
    return meteo::parse_P1seq(name, str, data);
  } else if (name == "Ccum") {
    return meteo::getValue_ccum(data, str);
  } else if (name == "region") {
    return meteo::getValue_region(data, str);
  } else if (name == "P0_fract") {
    return meteo::getValue_P0fract(data, str);
  } else if (name == "dd_tr") {
    return meteo::getValue_ddtr(data, str);
  } else if (name == "hshs_f") {
    const meteo::Descriptor& cur = _descr.value("hgr");
    return meteo::getValue_hshsf(_data, cur, data, str);
  } else if (name == "dt") {
    QString dtstr(str);
    data->setDateTime(QDateTime::fromString(dtstr.replace(' ', 'T'), Qt::ISODate));
    return true;
  } else if (name == "qhshshs") {
    return meteo::getValue_qhshshs(data, str);
  }

  return false;
}

void MeteoElement::registerGetValueFunc(const QStringList& dl, FuncGetValue func)
{
  for (int i = 0; i < dl.count(); i++) {
    _funcs.insert(dl.at(i), func);
  }
}


void MeteoElement::registerGetValueFunc()
{
  QStringList dl;
  dl << "VV" << "VsVs80" <<"VV81" << "VV82"<< "VV83"<< "VV84"<< "VV85"<< "VV86"<< "VV87"<< "VV88";
  registerGetValueFunc(dl, meteo::getValue_V);
  dl.clear();

  _funcs.insert("Vgmc", meteo::getValue_Vgmc);
  
  _funcs.insert("P0P0P0P0", meteo::getValue_P);
  _funcs.insert("P0P0P0P0_avg", meteo::getValue_P);
  _funcs.insert("PPPP",     meteo::getValue_P);
  _funcs.insert("PPPP_avg", meteo::getValue_P);
  _funcs.insert("RRR",    meteo::getValue_R);
  _funcs.insert("RRR333", meteo::getValue_R);
  _funcs.insert("RRR24",  meteo::getValue_R);
  _funcs.insert("RRR1",   meteo::getValue_R);
  _funcs.insert("R1R1R1R1", meteo::getValue_R1);
  _funcs.insert("RRRR",     meteo::getValue_R1);
  _funcs.insert("ssTwTwTw", meteo::getValue_ssTw);
  _funcs.insert("swTbTbTb", meteo::getValue_ssTw);

  //registerGetValueFunc(dl, meteo::getValue_snTg);

  dl << "snTTT" << "snTdTdTd" << "snTxTxTx" << "snTnTnTn" << "snTxTxTx_avg" << "snTnTnTn_avg" 
     << "snTxdTxdTxd" << "snTndTndTnd"<< "snTwTwTw"<< "snTaxTaxTax" << "snTanTanTan"
     << "snTTT24" << "snTwTwTw24" << "snTTT_avg";
  registerGetValueFunc(dl, meteo::getValue_snT);
  dl.clear();

  _funcs.insert("sndT", meteo::getValue_sndT);
  _funcs.insert("ssTaTaTa", meteo::getValue_ssTa);
  _funcs.insert("ssTdTdTd", meteo::getValue_ssTa);

  _funcs.insert("hshs", meteo::getValue_hshs);
  _funcs.insert("htht45", meteo::getValue_hshs);
  _funcs.insert("htht", meteo::getValue_hshs);
     
  _funcs.insert("Shhihihi", meteo::getValue_hi);

  dl<<"tt00"<<"tt01"<<"tt02"<<"tt03"<<"tt04"<<"tt05"<<"tt06"<<"tt07"<<"tt16"<<"tt17";
  registerGetValueFunc(dl, meteo::getValue_tt);
  dl.clear();

  dl<<"RR30"<<"ss31"<<"RR32"<<"RR33"<<"RR34"<<"RR35"<<"RR36"<<"RR37";
  registerGetValueFunc(dl, meteo::getValue_RR);
  dl.clear();

  _funcs.insert("P0P0P0", meteo::getValue_P1);
  _funcs.insert("P1P1P1TempB", meteo::getValue_P1);
  _funcs.insert("P1P1P1WindB", meteo::getValue_P1); 
  _funcs.insert("PtPtPtA", meteo::getValue_Pt);
  _funcs.insert("PmPmPmA", meteo::getValue_Pt);
  _funcs.insert("PtPtPtC", meteo::getValue_PtC);
  _funcs.insert("PmPmPmC", meteo::getValue_PtC);

  dl<<"T0T0Ta0"<<"TtTtTatA"<<"TtTtTatC"<<"T1T1Ta1A"<<"T1T1Ta1B"<<"T1T1Ta1C"<<"T1T1Ta1D";
  registerGetValueFunc(dl, meteo::getValue_T);
  dl.clear();

  dl<<"D0D0"<<"DtDtA"<<"DtDtC"<<"D1D1A"<<"D1D1B"<<"D1D1C"<<"D1D1D";
  registerGetValueFunc(dl, meteo::getValue_D);
  dl.clear();

  dl<<"dtdtfA"<<"dmdmfA"<<"dtdtfC"<<"dmdmfC"<<"d1d1fA"<<"d1d1fB"<<"d1d1fC"<<"d1d1fD" << "d0d0f";
  registerGetValueFunc(dl, meteo::getValue_dt);
  dl.clear();

  dl<<"f0f0f0"<<"ftftftA"<<"fmfmfmA"<<"ftftftC"<<"fmfmfmC"
    <<"f1f1f1A"<<"f1f1f1B"<<"f1f1f1C"<<"f1f1f1D" 
    << "vavaA" << "vavaC" << "vbvbA" << "vbvbC";
  registerGetValueFunc(dl, meteo::getValue_ft);
  dl.clear();

  dl << "ff" << "fff" << "ff2";
  registerGetValueFunc(dl, meteo::getValue_fiw);
  dl.clear();

  _funcs.insert("P1P1h1h1h1A", meteo::getValue_P1hA);
  _funcs.insert("P1P1h1h1h1C", meteo::getValue_P1hC);
  _funcs.insert("n1n1TempB", meteo::getValue_n1);
  _funcs.insert("n1n1WindB", meteo::getValue_n1);
  _funcs.insert("n1n1TempD", meteo::getValue_n1);
  _funcs.insert("n1n1WindD", meteo::getValue_n1);

  _funcs.insert("eReR", meteo::getValue_eR);
  _funcs.insert("LaLaLaLa", meteo::getValue_La);
  _funcs.insert("LoLoLoLoLo", meteo::getValue_Lo);

  _funcs.insert("La_gms", meteo::getValue_Lagms);
  _funcs.insert("Lo_gms", meteo::getValue_Logms);

  dl << "La_v" << "Lo_v" << "La_reg" << "Lo_reg" << "La_c" << "Lo_c";
  registerGetValueFunc(dl, meteo::getValue_LaLo_v);
  dl.clear();

  _funcs.insert("LaLaLaLaLa", meteo::getValue_LaGr);
  _funcs.insert("LoLoLoLoLoLo", meteo::getValue_LoGr);
 
  _funcs.insert("VVVV", meteo::getValue_VV);

  _funcs.insert("c0c0", meteo::getValue_c0c0);

  _funcs.insert("snhshshs", meteo::getValue_hshshs);
  _funcs.insert("/snhshshs/", meteo::getValue_hshshs);
  _funcs.insert("//snhshshs//", meteo::getValue_hshshs);

  _funcs.insert("zn1", meteo::getValue_zn);  
  _funcs.insert("zn1D", meteo::getValue_zn);

  _funcs.insert("TnTnTn", meteo::getValue_Tn10);  
  _funcs.insert("TnTnTnD", meteo::getValue_Tn10);  

  _funcs.insert("TnTn", meteo::getValue_Tn100);  
  _funcs.insert("TnTnD", meteo::getValue_Tn100);  

  dl << "bbbb" << "HHHH" << "HHHH20" << "MIDHHHH" << "MINHHHH" << "MAXHHHH"
     << "HHHHvb" << "HHHHs" << "HHHHks" << "HHHHnb" << "HHHHv" << "HHHHn";
  registerGetValueFunc(dl, meteo::getValue_bbbb);
  dl.clear();

  dl << "tt0" << "ttb" << "dtableT" << "TTt" << "dt_dh";
  registerGetValueFunc(dl, meteo::getValue_tt0);
  dl.clear();

  //  _funcs.insert("dt_dh", meteo::getValue_dtdh);

  _funcs.insert("dtableP", meteo::getValue_dtableP);

  dl << "kQQQ" << "MIDkQQQ" <<  "MINkQQQ" << "MAXkQQQ" << "kVVVs" << "kVVVks" << "kQQQo" << "kQQQb" 
     << "kQQQa" << "kQQQo1" << "kQQQb1" << "kQQQa1" << "kQQQsb" << "kFFF";
  registerGetValueFunc(dl, meteo::getValue_kq);
  dl.clear();

  _funcs.insert("QQQ", meteo::getValue_kq1);

  _funcs.insert("HiHiHiK", meteo::getValue_hik);
  
  dl << "snT0T0_avg" << "snTxTx_avg" << "snTnTn_avg" << "T_T_" 
     << "T_dT_d" << "TsTs" << "TFTF_max" << "TFTF_min" << "snTg_Tg_" << "snTgTg" << "snTnTn";
  registerGetValueFunc(dl, meteo::getValue_snTT);
  dl.clear();

  _funcs.insert("UU", meteo::getValue_U);

  dl << "dd" << "dw1dw1" << "dw2dw2" << "d0d0" << "dndn" << "dsds";
  registerGetValueFunc(dl, meteo::getValue_dd);
  dl.clear();

  _funcs.insert("NsNsNs", meteo::getValue_Ns);
  _funcs.insert("Nsw", meteo::getValue_Ns);

  _funcs.insert("ww", meteo::getValue_w);
  _funcs.insert("W1", meteo::getValue_W);
  _funcs.insert("W2", meteo::getValue_W);

  _funcs.insert("h0h0h0h0", meteo::getValue_h0);

  _funcs.insert("Tpms", meteo::getValue_Tpms);

  _funcs.insert("tt906", meteo::getValue_tt906);

  //  _funcs.insert("iff", meteo::getValue_iff);

  _funcs.insert("ppp", meteo::getValue_ppp);

  _funcs.insert("national", meteo::getValue_trash);
  _funcs.insert("status", meteo::getValue_status);

  _funcs.insert("gk_p", meteo::getValue_gk_p);
  _funcs.insert("gFf", meteo::getValue_gFf);
  _funcs.insert("gpi", meteo::getValue_gpi);
  _funcs.insert("gF_st", meteo::getValue_gdst);
  _funcs.insert("ggamma", meteo::getValue_gdst);
  _funcs.insert("gpi1", meteo::getValue_gdst);
  _funcs.insert("gic", meteo::getValue_gic);
  _funcs.insert("BBiH", meteo::getValue_bbih);
}


float meteo::getValue_V(const TMeteoData*, const QString& strelem, bool* ok)  
{
  int val = strelem.toInt(ok);
  if(val < 51) return 100*val;
  if(val >= 56 && val < 81) return (val-50.)*1000;
  if(val >= 81 && val < 90) return ((val-80.)*5.+30.)*1000;
  switch(val) {
  case 90: return 10;
  case 91: return 50;
  case 92: return 200;
  case 93: return 500;
  case 94: return 1000.;
  case 95: return 2000.;
  case 96: return 4000.;   
  case 97: return 10000.;   
  case 98: return 20000.;   
  case 99: return 50000.;   
 }
  *ok = false;
  return BAD_VALUE;
}


float meteo::getValue_fiw(const TMeteoData* data, const QString& strelem, bool* ok)
{
  if (0 == data) {
    *ok = false;
    return BAD_VALUE;
  }

   float val = strelem.toFloat(ok);

  int iw;
  control::QualityControl qual = control::NO_OBSERVE;
  if (data->getValue(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("iw"), &iw, &qual) && 
      qual <= control::NO_CONTROL) {
    if (iw == 3 || iw == 4) {
      val *= 0.514;
    }
  }
  
  return val;
}

float meteo::getValue_P(const TMeteoData*, const QString& strelem, bool* ok) 
{
  return ("0" == strelem.left(1))? 1000.+0.1*strelem.toFloat(ok): 0.1*strelem.toFloat(ok);
}

bool meteo::getValue_P0fract(TMeteoData* data, const QString& strelem)
{
  if (0 == data) {
    return false;
  }
  
  bool ok;
  float val = strelem.toFloat(&ok) * 0.01;
  if (!ok) return false;

  TMeteoParam* P0 = data->paramPtr(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("P0_int"), false);

  if (0 != P0 && P0->quality() <= control::NO_CONTROL) {
    P0->setValue(P0->value() + val);
  }
  
  return true;
}

float meteo::getValue_R(const TMeteoData*, const QString& strelem, bool* ok)
{
  int val = strelem.toInt(ok);
  return (990 > val)? val : 0.1*(val-990.);
}

float meteo::getValue_R1(const TMeteoData*, const QString& strelem, bool* ok)
{
  int val = strelem.toInt(ok);
  if (val > 8899 && val < 9999) {
    *ok = false;
    return BAD_VALUE;
  }
  return (val == 9999)? 0.5 : val;
}

float meteo::getValue_ssTw(const TMeteoData*, const QString& strelem, bool* ok)
{
  return (0 == strelem.left(1).toInt()%2)? 0.1*strelem.right(3).toFloat(ok) : -0.1*strelem.right(3).toFloat(ok);
}

//snTTT с десятыми
float meteo::getValue_snT(const TMeteoData*, const QString& strelem, bool* ok) 
{
  return ("0" == strelem.left(1))? 0.1*strelem.right(3).toFloat(ok) : -0.1*strelem.right(3).toFloat(ok);
}

//snTT в целых
float meteo::getValue_snTT(const TMeteoData*, const QString& strelem, bool* ok)
{
  if (strelem.size() == 3 && ("1" == strelem.left(1) || "M" == strelem.left(1)))  {
    return -strelem.right(2).toFloat(ok);
  }

  return strelem.right(2).toFloat(ok);
}

float meteo::getValue_Tpms(const TMeteoData*, const QString& strelem, bool* ok)
{
  int sign = strelem.left(1) == "M" ? -1 : 1;
  float val = strelem.right(2).toInt(ok);

  return sign*val;
}

// float meteo::getValue_snTg(const TMeteoData*, const QString& strelem, bool* ok)
// {
//   if (strelem.size() == 3 && ("1" == strelem.left(1) || "M" == strelem.left(1)))  {
//     return -0.1*strelem.right(2).toFloat(ok);
//   }

//   return 0.1*strelem.right(2).toFloat(ok);
// }

float meteo::getValue_sndT(const TMeteoData*, const QString& strelem, bool* ok)
{
  int sign = 1;
  if (strelem.size() == 2 && ("1" == strelem.left(1) || "M" == strelem.left(1)))  {
    sign = -1;
  }

  float val = strelem.right(1).toFloat(ok);
  if (val <= 4) val += 10;

  return sign*val;
}

float meteo::getValue_ssTa(const TMeteoData*, const QString& strelem, bool* ok)
{
  int sign = 1;
  if (strelem.left(2) == "MS")  {
    sign = -1;
  }

  float val = strelem.right(3).toFloat(ok);

  return sign * val * 0.1;
}

float meteo::getValue_U(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = strelem.toFloat(ok);
  if (val == 0) {
    val = 100;
  }

  return val;
}

bool meteo::getValue_qhshshs(TMeteoData* data, const QString& strelem)
{
  if (nullptr == data) {
    return false;
  };

  bool ok;
  int val = strelem.toInt(&ok);
  
  if (!ok) {
    return false;
  }

  //  TMeteoParam* h = data->meteoParamPtr(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("h"));
  TMeteoParam* h = data->meteoParamPtr("h");
  if (nullptr == h) {
    //не было ещё h
    debug_log << "not value";
    data->add(TMeteoDescriptor::instance()->descriptor("h"), "", val, control::NO_CONTROL);
    return true;
  }

  //заменяем h
  debug_log << "change value" << val;
  h->setValue(val);
  h->setCode("");

  return true;
}


bool meteo::getValue_hshsf(TMeteoData* data, const meteo::Descriptor& cur, TMeteoData* curChild,  const QString& strelem)
{
  if (nullptr == data || nullptr == curChild) {
    return false;
  };

  if (strelem.isEmpty() || strelem.contains('/')) {
    curChild->add(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("h"), strelem, BAD_VALUE, control::NO_CONTROL);
    return true;
  }
						   
  
  bool ok;
  float hshs = meteo::getValue_hshs(data, strelem, &ok);
  if (ok == false) {
    return ok;
  }

  curChild->add(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("h"), strelem, hshs, control::NO_CONTROL);

  //заменяем визуально измеренную ВНГО (правило h и hshs для наноски)
  TMeteoParam* h = data->paramPtr(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("h"));
  if (nullptr == h) {
    return ok;
  }

  if (h->quality() <= control::NO_CONTROL && h->code() != "9" && h->value() < hshs) {
    bool fok;
    int code = h->code().toInt(&fok);
    if (fok) {
      if (cur.table.contains(QString::number(code + 1))) {
	float h_next = cur.table.value(QString::number(code + 1));
	if (h_next > hshs) {
	  h->setValue(hshs);
	}
      }
    }
  }

  return ok;
}

float meteo::getValue_hshs(const TMeteoData*, const QString& strelem, bool* ok) 
{
  int val = strelem.toInt(ok);
  if(val < 51) return 30.*val;
  if(val >= 56 && val < 81) return (val-50.)*300.;
  if(val >= 81 && val < 90) return (val-80.)*1500.+9000.;
  
  switch (val) {
  case 90: return 10.;
  case 91: return 50.;
  case 92: return 100.;
  case 93: return 200.;
  case 94: return 300.;
  case 95: return 600.;
  case 96: return 1000.;   
  case 97: return 1500.;   
  case 98: return 2000.;   
  case 99: return 2500.;   
  }
  *ok = false;
  return BAD_VALUE;
}


float meteo::getValue_hi(const TMeteoData*, const QString& strelem, bool* ok)
{
  int sign = strelem.left(1) == "A" ? -1 : 1;
  float val = strelem.right(3).toInt(ok);

  return sign * val * 30.48;
}

float meteo::getValue_tt(const TMeteoData*, const QString& strelem, bool* ok) 
{
  int val = strelem.toInt(ok);
  if(val < 61) return 0.1*val;
  if(val >= 61 && val < 67) return (val-60.)+6.5;
  if(val == 67) return 15.;
  if(val == 68) return 18.;
  if(val >= 70 && val < 100) return val; //возвращается цифра кода
  *ok = false;
  return BAD_VALUE;
}

float meteo::getValue_tt906(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = strelem.toInt(ok);
  if (val <= 60) {
    return val * 0.1;
  }
  
  val = (val - 60) + 6;
  
  return val;
}

float meteo::getValue_RR(const TMeteoData*, const QString& strelem, bool* ok) 
{
  int val = strelem.toInt(ok);
  if(val < 56) return val;
  if(val >= 56 && val < 91) return 10.0*(val-50.);
  if(val >= 91 && val < 97) return 0.1*(val-90.);
  if(val == 97) return 0.;
  if(val == 98) return 450.;
  *ok = false;

  return BAD_VALUE;
}

float meteo::getValue_P1(const TMeteoData*, const QString& strelem, bool* ok) 
{
  return ("0" == strelem.left(1))? 1000.+strelem.toFloat(ok): strelem.toFloat(ok);
}

float meteo::getValue_Pt(const TMeteoData*, const QString& strelem, bool* ok) 
{
  if("999" == strelem) {
    *ok = false;
    return BAD_VALUE;
  }
  return ("0" == strelem.left(1))? 1000.+strelem.toFloat(ok): strelem.toFloat(ok);
}

float meteo::getValue_PtC(const TMeteoData*, const QString& strelem, bool* ok)
{
  if ("999" == strelem) {
    *ok = false;
    return BAD_VALUE;
  }
  return 0.1*strelem.toFloat(ok);
}

float meteo::getValue_T(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val1 = strelem.left(2).toFloat(ok);
  int val2 = strelem.right(1).toInt(ok);
  return pow(-1,val2) * (val1+0.1*val2);
}

float meteo::getValue_D(const TMeteoData*, const QString& strelem, bool* ok) 
{
  int val = strelem.toInt(ok);
  if(val < 51) return 0.1*val;
  if(val >= 56 && val < 100) return val-50.;
  *ok = false;
  return BAD_VALUE;
}


float meteo::getValue_dt(const TMeteoData*, const QString& strelem,bool* ok)
{
  return (3 == strelem.count())? strelem.toFloat(ok): 10.*strelem.left(2).toFloat(ok);
}

float meteo::getValue_ft(const TMeteoData* data, const QString& strelem,bool* ok)
{
  float val = (500 > strelem.toInt())? strelem.toFloat(ok) : strelem.right(2).toFloat(ok);

  //вычисление
  if (0 == data) {
    *ok = false;
    return BAD_VALUE;
  }

  int day;
  control::QualityControl qual = control::NO_OBSERVE;
  if (data->getValue(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("YY"), &day, &qual) && 
      qual <= control::NO_CONTROL) {
    if (day > 50) {
      val *= 0.514;
    }
  }
  
  return val;
}


float meteo::getValue_P1hA(const TMeteoData*, const QString& strelem,bool* ok) 
{
  int val1 = strelem.left(2).toInt(ok);
  float val2 = strelem.right(3).toFloat(ok);
  switch(val1){
  case 0:  return (val2 < 500.)? val2 : -val2+500.;
  case 10: return val2*10.+10000.;
  case 15: return val2*10.+10000.;   
  case 20: return val2*10.+10000.;
  case 25: return (val2 < 500.)? val2*10.+10000.:val2*10.;   
  case 30: return val2*10.;
  case 40: return val2*10.;   
  case 50: return val2*10.;
  case 70: return (val2 < 500.)? val2+3000.:val2+2000.;
  case 85: return val2+1000.;   
  case 92: return (val2 < 400)? val2+1000. : val2;
  }
  
  *ok = false;
  return BAD_VALUE;
}


float meteo::getValue_P1hC(const TMeteoData*, const QString& strelem, bool* ok) 
{
  int val1 = strelem.left(2).toInt(ok);
  float val2 = strelem.right(3).toFloat(ok);
 
  //qDebug() << val1 <<val2 << ok;
  switch(val1){
  case 10: return (val2 < 500.)? val2*10.+30000.:val2*10.+20000.;
  case 20: return val2*10.+20000.;
  case 30: return val2*10.+20000.;
  case 50: return (val2 < 500.)? val2*10.+20000.:val2*10.+10000.;
  case 70: return val2*10.+10000.;   
  }
  
  *ok = false;
  return BAD_VALUE;
}


float meteo::getValue_n1(const TMeteoData*, const QString& strelem, bool* ok) 
{
  return strelem.toFloat(ok)/11.;
}

float meteo::getValue_eR(const TMeteoData*, const QString& strelem, bool* ok) 
{
  int val = strelem.toInt(ok);
  if (!ok) return BAD_VALUE;

  *ok = true;
  if (val < 90) {
    return val/10.;
  }
  
  switch (val) {
  case 92: return 10;
  case 93: return 15;
  case 94: return 20;
  case 95: return 25;
  case 96: return 30;
  case 97: return 35;
  case 98: return 40;
  }
  
  *ok = false;
  return BAD_VALUE;
}

float meteo::getValue_ppp(const TMeteoData* data, const QString& strelem, bool* ok)
{
  //вычисление
  if (0 == data) {
    *ok = false;
    return BAD_VALUE;
  }

  float val = strelem.toFloat(ok) * 0.1;

  int a;
  control::QualityControl qual = control::NO_OBSERVE;
  if (data->getValue(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("a"), &a, &qual) && 
      qual <= control::NO_CONTROL) {
    if (a >= 5 && a <= 8) {
      val = -val;
    }
  }
  
  return val;

}

float meteo::getValue_La(const TMeteoData*, const QString& strelem, bool* ok)
{
  float lat = strelem.left(2).toFloat(ok);
  if (*ok == false) return BAD_VALUE;
  float min = strelem.mid(2, 2).toFloat(ok);
  if (*ok == false) return BAD_VALUE;

  int mul = 1; //если есть направление
  if (strelem.length() == 5 && strelem[4] == "S") {
    mul = -1;
  }
  
  return mul*(lat + min / 60);
}

float meteo::getValue_Lo(const TMeteoData*, const QString& strelem, bool* ok)
{
  float lon = strelem.left(3).toFloat(ok);
  if (*ok == false) return BAD_VALUE;
  float min = strelem.mid(3, 2).toFloat(ok);
  if (*ok == false) return BAD_VALUE;
  
  int mul = 1; //если есть направление
  if (strelem.length() == 6 && strelem[5] == "W") {
    mul = -1;
  }
  
  return mul* (lon + min / 60);
}

float meteo::getValue_LaGr(const TMeteoData*, const QString& strelem, bool* ok)
{
  float lat;
  if (strelem.right(2) == "/") {
    lat = strelem.left(3).toFloat(ok) / 10;
  } else if (strelem.right(1) == "/") {
    lat = strelem.left(4).toFloat(ok) / 100;
  } else {
    lat = strelem.left(5).toFloat(ok) / 1000;
  }
  
  if (*ok == false) return BAD_VALUE;

  return lat;
}

float meteo::getValue_LoGr(const TMeteoData*, const QString& strelem, bool* ok)
{
  float lon;
  if (strelem.right(2) == "/") {
    lon = strelem.left(4).toFloat(ok) / 10;
  } else if (strelem.right(1) == "/") {
    lon = strelem.left(5).toFloat(ok) / 100;
  } else {
    lon = strelem.left(6).toFloat(ok) / 1000;
  }
  
  if (*ok == false) return BAD_VALUE;

  return lon;
}


float meteo::getValue_Lagms(const TMeteoData*, const QString& strelem, bool* ok)
{
  QRegExp rx("([\\w])([\\d]{,3})\\s+([\\d]{,3})'([\\d]{,3})''");
  *ok = false;

  int  pos = 0;
  pos = rx.indexIn(strelem, pos);
  if (pos == -1) {
    return BAD_VALUE;
  }

  QStringList caption = rx.capturedTexts();  
  if (5 != caption.size()) {
    return BAD_VALUE;
  }

  QString b = caption.at(1);
  float deg = caption.at(2).toFloat(ok);
  float min = caption.at(3).toFloat(ok);
  float sec = caption.at(4).toFloat(ok);

  float lat = deg + min/60 + sec/3600;
  if (b == "S") {
    lat = -lat;
  }

  return lat;
}

float meteo::getValue_Logms(const TMeteoData*, const QString& strelem, bool* ok)
{
  QRegExp rx("([\\w])([\\d]{,3})\\s+([\\d]{,3})'([\\d]{,3})''");
  *ok = false;

  int  pos = 0;
  pos = rx.indexIn(strelem, pos);
  if (pos == -1) return BAD_VALUE;

  QStringList caption = rx.capturedTexts();  
  if (5 != caption.size()) {
    return BAD_VALUE;
  }

  QString b = caption.at(1);
  float deg = caption.at(2).toFloat(ok);
  float min = caption.at(3).toFloat(ok);
  float sec = caption.at(4).toFloat(ok);

  float lon = deg + min/60 + sec/3600;
  if (b == "W") {
    lon = -lon;
  }

  return lon;
}

float meteo::getValue_LaLo_v(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = BAD_VALUE;
  *ok = false;

  if (strelem[0] == 'N' || strelem[0] == 'E') {
    val = strelem.mid(1, strelem.size()-3).toFloat(ok) + strelem.right(2).toFloat(ok) / 60;
  } else if (strelem[0] == 'S') {
    val = -strelem.mid(1, strelem.size()-3).toFloat(ok) + strelem.right(2).toFloat(ok) / 60;
  } else if (strelem[0] == 'W') {
    val = -strelem.mid(1, strelem.size()-3).toFloat(ok) + strelem.right(2).toFloat(ok) / 60;
  } 
  else if (strelem.right(1) == "N" || strelem.right(1) == "E") {
    val = strelem.left(strelem.size()-3).toFloat(ok) + strelem.mid(strelem.size()-3, 2).toFloat(ok) / 60;
  } else if (strelem.right(1) == "S") {
    val = strelem.left(strelem.size()-3).toFloat(ok) + strelem.mid(strelem.size()-3, 2).toFloat(ok) / 60;
  } else if (strelem.right(1) == "W") {
    val = strelem.left(strelem.size()-3).toFloat(ok) + strelem.mid(strelem.size()-3, 2).toFloat(ok) / 60;
  }

  return val;
}

//видимость, м и дюймы
float meteo::getValue_VV(const TMeteoData*, const QString& strelem, bool* ok)  
{
  float val = BAD_VALUE;
  *ok = false;

  if (-1 != strelem.indexOf("SM")) {
    QRegExp rx("(\\d{1,2})?\\s?(?:(\\d)/(\\d{1,2}))?SM");
    int pos = 0;
    pos = rx.indexIn(strelem, pos);
    if (pos == -1) return BAD_VALUE;
    val = rx.cap(1).toFloat();
    if (!rx.cap(2).isEmpty() && !rx.cap(3).isEmpty()) {
      val += rx.cap(2).toFloat() / rx.cap(3).toFloat();
    }

    *ok = true;
    val *= 1609.34; //мили в м
  } else {
    val = strelem.toFloat(ok);
  }

  return val;
}

//видимость, м и км
float meteo::getValue_Vgmc(const TMeteoData*, const QString& strelem, bool* ok)  
{
  float val = BAD_VALUE;
  *ok = false;
  val = strelem.toFloat(ok);
  if (val < 20) {
    val *= 1000;
  }

  return val;
}

float meteo::getValue_c0c0(const TMeteoData* data,const QString& strelem, bool* ok)
{
  if (0 == data) {
    *ok = false;
    return BAD_VALUE;
  }
  
  float val = strelem.toFloat(ok);

  int ic;
  control::QualityControl qual = control::NO_OBSERVE;
  if (data->getValue(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("ic"), &ic, &qual) && 
      qual <= control::NO_CONTROL) {
    if (ic == 1) {
      val *= 0.514;
    }
  }
  
  return val;
}

float meteo::getValue_hshshs(const TMeteoData* , const QString& strelem, bool* ok)
{
  return ("0" == strelem.left(1))? strelem.right(3).toFloat(ok) : -strelem.right(3).toFloat(ok);
}


float meteo::getValue_zn(const TMeteoData* data,const QString& strelem, bool* ok)
{ 
  if (0 == data) {
    *ok = false;
    return BAD_VALUE;
  }

  float val = strelem.toFloat(ok);

  int zz;
  control::QualityControl qual = control::NO_OBSERVE;
  if (data->getValue(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("zz"), &zz, &qual) && 
      qual <= control::NO_CONTROL) {
    val += zz;
  }

  return val;
}

float meteo::getValue_Tn10(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = strelem.toFloat(ok);
  
  if (val > 500) {
    val = -1 * (val - 500);
  }

  return val * 0.1;
}

float meteo::getValue_Tn100(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = strelem.toFloat(ok);
  
  if (val > 5000) {
    val = -1 * (val - 5000);
  }

  return val * 0.01;
}

float meteo::getValue_bbbb(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = strelem.toFloat(ok);
  
  if (val > 5000) {
    val = -1 * (val - 5000);
  }

  return val;
}

float meteo::getValue_tt0(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = strelem.toFloat(ok);
  
  if (val > 50) {
    val = -1 * (val - 50);
  }

  return val;
}

// float meteo::getValue_dtdh(const TMeteoData* data, const QString& strelem, bool* ok)
// {
//   float val = strelem.toFloat(ok);

//   float t = data->getValue(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("dtableT"), BAD_VALUE);
//   if (t != BAD_VALUE && t < )

// }

float meteo::getValue_dtableP(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = strelem.toFloat(ok);
  
  if (val > 500) {
    val = -1 * (val - 500);
  }

  return val;
}


float meteo::getValue_kq(const TMeteoData*, const QString& strelem, bool* ok)
{
  int k = strelem.left(1).toInt(ok);
  if (!*ok) return BAD_VALUE;

  float val = strelem.right(3).toFloat(ok);
  if (!*ok) return BAD_VALUE;

  return val * pow(10, k - 3);
}

float meteo::getValue_kq1(const TMeteoData*, const QString& strelem, bool* ok)
{
  int k = strelem.right(1).toInt(ok);
  if (!*ok) return BAD_VALUE;

  float val = strelem.left(3).toFloat(ok);
  if (!*ok) return BAD_VALUE;

  return val * pow(10, k-3); //в м^3
}


float meteo::getValue_hik(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = strelem.left(3).toFloat(ok);
  if (!*ok) return BAD_VALUE;

  int s = strelem.right(1).toInt(ok);
  if (!*ok) return BAD_VALUE;
  if (s == 2) {
    val *= -1;
  }

  return val;
}


float meteo::getValue_dd(const TMeteoData*, const QString& strelem, bool* ok)
{
  uint val = strelem.toUInt(ok);
  if (val == 99) {
    val = 0;
  }

  return val * 10;
}


bool meteo::getValue_ddtr(TMeteoData* data, const QString& strelem)
{
  if (nullptr == data) {
    return false;
  }

  bool ok = false;
  //штиль
  if (strelem.trimmed() == QChar(',') || strelem.toLower().contains("tih")) {
    data->add(TMeteoDescriptor::instance()->descriptor("dd"), "0", 0, control::NO_CONTROL);
    data->add(TMeteoDescriptor::instance()->descriptor("ff"), "0", 0, control::NO_CONTROL);
    ok = true;
  }

  //переменный
  if (strelem.toLower().contains("per")) {
    data->add(TMeteoDescriptor::instance()->descriptor("dd"), "0", 0, control::NO_CONTROL);
    ok = true;
  }

  return ok;
}



// float meteo::getValue_iff(const TMeteoData*, const QString& strelem, bool* ok)
// {
//   *ok = true;
//   if (strelem == "P") {
//     return 1;
//   }

//   return 0;
// }

float meteo::getValue_Ns(const TMeteoData*, const QString& strelem, bool* ok)
{
  *ok = true;
  //Nh (20011)
  if (strelem.toUpper() == "FEW") {
    return 3;
  } else if (strelem.toUpper() == "SCT") {
    return 5;
  } else if (strelem.toUpper() == "BKN") {
    return 9;
  } else if (strelem.toUpper() == "OVC") {
    return 10;
    //Nsw (20009)
  } else if (strelem.toUpper() == "CLR") {
    return 3;
  } else if (strelem.toUpper() == "NCD") {
    return 3;
  } else if (strelem.toUpper() == "NSC") {
    return 1;
  } else if (strelem.toUpper() == "SKC") {
    return 3;
  } else if (strelem.toUpper() == "CAVOK") {
    return 2;
  }

  *ok = false;
  return BAD_VALUE;
}


float meteo::getValue_w(const TMeteoData* data,const QString& strelem, bool* ok)
{ 
  if (0 == data) {
    *ok = false;
    return BAD_VALUE;
  }

  float val = strelem.toFloat(ok);
  QString ix = "";
  control::QualityControl qual = control::NO_OBSERVE;
  if (data->getCode(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("ix"), &ix, &qual) && 
      qual <= control::NO_CONTROL && ix == "7") {
    val += 100;
  }

  return val;
}

float meteo::getValue_W(const TMeteoData* data,const QString& strelem, bool* ok)
{ 
  if (0 == data) {
    *ok = false;
    return BAD_VALUE;
  }

  float val = strelem.toFloat(ok);
  QString ix = "";
  control::QualityControl qual = control::NO_OBSERVE;
  if (data->getCode(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("ix"), &ix, &qual) && 
      qual <= control::NO_CONTROL && ix == "7") {
    val += 10;
  }

  return val;
}

float meteo::getValue_h0(const TMeteoData* data,const QString& strelem, bool* ok)
{ 
  if (0 == data) {
    *ok = false;
    return BAD_VALUE;
  }

  float val = strelem.toFloat(ok);
  float im = 0;
  control::QualityControl qual = control::NO_OBSERVE;
  if (!data->getValue(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("im"), &im, &qual) || 
      qual > control::NO_CONTROL) {
    *ok = false;
    return BAD_VALUE;
  }

  //футы в метры
  if (im >= 4.5) {
    val *= 0.3048;
  }

  return val;
}

float meteo::getValue_gk_p(const TMeteoData* , const QString& strelem, bool* ok)
{
  QString sval = strelem;
  return 0.1 * sval.remove(" ").toFloat(ok);
}

float meteo::getValue_gFf(const TMeteoData* data, const QString& strelem, bool* ok)
{
  if (0 == data) {
    *ok = false;
    return BAD_VALUE;
  }

  float val = strelem.toFloat(ok);
  float freq = 0;
  control::QualityControl qual = control::NO_OBSERVE;
  if (!data->getValue(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("gfreq"), &freq, &qual) || 
      qual > control::NO_CONTROL) {
    *ok = false;
    return BAD_VALUE;
  }

  if (freq >= 20000) {
    val *= 10;
  }

  return val;
}

float meteo::getValue_gpi(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = strelem.left(3).toFloat(ok);
  if (!ok) {
    return BAD_VALUE;
  }

  float deg = strelem.right(1).toFloat(ok);
  if (!ok) {
    return BAD_VALUE;
  }

  return val * pow(10, deg - 2);
}

float meteo::getValue_gdst(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = strelem.left(2).toFloat(ok);
  if (!ok) {
    return BAD_VALUE;
  }

  float deg = strelem.right(2).toFloat(ok);
  if (!ok) {
    return BAD_VALUE;
  }

  return val * pow(10, deg - 1);
}

float meteo::getValue_gic(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = strelem.toFloat(ok);
  if (!ok) {
    return BAD_VALUE;
  }

  if (val < 500) {
    val += 1000;
  }
  
  return val;
}


float meteo::getValue_bbih(const TMeteoData* data, const QString& strelem, bool* ok)
{
  if (0 == data) {
    *ok = false;
    return BAD_VALUE;
  }

  float val = strelem.toFloat(ok);
  float country = 0;
  control::QualityControl qual = control::NO_OBSERVE;
  if (!data->getValue(meteo::anc::MetaData::instance()->dataElements()->getDescriptor("A1Ci"), &country, &qual) || 
      qual > control::NO_CONTROL) {
    *ok = false;
    return BAD_VALUE;
  }
  
  val += country*100000;

  return val;
}

bool meteo::getValue_region(TMeteoData* data, const QString& strelem)
{
  if (0 == data) {
    return false;
  }  

  descr_t descr = TMeteoDescriptor::instance()->descriptor("region");

  if (strelem.contains("ENTIRE")) {
    data->add(descr, "ENTIRE", 0, control::RIGHT);
    return true;
  }

  if (strelem.contains("WI") ||strelem.contains("VA CLD")) {
    return getValue_regionWI(data, strelem);
  }

  if (strelem.contains("OF LINE")) {
    return getValue_regionLine(data, strelem);
  }

  if (strelem.contains("OF")) {
    return getValue_regionPart(data, strelem);
  }
  

  //TODO остальные варианты
  
  data->add(descr, strelem, BAD_VALUE, control::MISTAKEN);
  return true;
}

bool meteo::getValue_regionPart(TMeteoData* data, const QString& elem)
{
  if (0 == data) {
    return false;
  }  

  descr_t descr = TMeteoDescriptor::instance()->descriptor("region");

  QStringList strelem = elem.split("AND", QString::SkipEmptyParts);

  for (int idx = 0; idx < strelem.size(); idx++) {
    GeoVector reg(4);
    reg[0] = GeoPoint::fromDegree( 90, -180);
    reg[1] = GeoPoint::fromDegree( 90,  180);
    reg[2] = GeoPoint::fromDegree(-90,  180);
    reg[3] = GeoPoint::fromDegree(-90, -180);

    QRegExp rx("([SNEW])\\s*OF\\s*(?:([SN])(\\d{2})(\\d{2})?|([EW])(\\d{3})(\\d{2})?)");
    int pos = rx.indexIn(strelem.at(idx));

    while (-1 != pos) {
      QStringList sl = rx.capturedTexts();
      if (sl.size() != 8) {
	pos = rx.indexIn(strelem.at(idx), pos + 5);
	continue;
      }

      int sign = 1;
      if (sl.at(2) == "S" || sl.at(5) == "W") {
	sign = -1;
      }

      if (sl.at(1) == "N") {
	float pnt = sign * (sl.at(3).toFloat() + sl.at(4).toFloat() / 60);
	reg[2].setLatDeg(pnt);
	reg[3].setLatDeg(pnt);
      } else if (sl.at(1) == "S") {
	float pnt = sign * (sl.at(3).toFloat() + sl.at(4).toFloat() / 60);
	reg[0].setLatDeg(pnt);
	reg[1].setLatDeg(pnt);
      } else if (sl.at(1) == "W") {
	float pnt = sign * (sl.at(6).toFloat() + sl.at(7).toFloat() / 60);
	reg[1].setLonDeg(pnt);
	reg[2].setLonDeg(pnt);
      } else if (sl.at(1) == "E") {
	float pnt = sign * (sl.at(6).toFloat() + sl.at(7).toFloat() / 60);
	reg[0].setLonDeg(pnt);
	reg[3].setLonDeg(pnt);
      }

      pos = rx.indexIn(strelem.at(idx), pos + 5);
    }

    QString region = "POLYGON((";
    for (int ri = 0 ; ri < reg.size(); ri++) {
      region += QString::number(reg[ri].lonDeg(), 'f', 2) + " " + QString::number(reg[ri].latDeg(), 'f', 2) + "\\, ";
    }
    region += QString::number(reg[0].lonDeg(), 'f', 2) + " " + QString::number(reg[0].latDeg(), 'f', 2) + "\\, ";
    region.remove(-3, 3);
    region += "))";

    data->add(descr, region, 0, control::RIGHT);
  }

  return true;
}

bool meteo::getValue_regionLine(TMeteoData* data, const QString& strelem)
{ 
  if (0 == data) {
    return false;
  }  

  descr_t descr = TMeteoDescriptor::instance()->descriptor("region");
  data->add(descr, strelem, BAD_VALUE, control::MISTAKEN);
  return true;
}


bool meteo::getValue_regionWI(TMeteoData* data, const QString& strelem)
{
  if (0 == data) {
    return false;
  }  

  descr_t descr = TMeteoDescriptor::instance()->descriptor("region");

  QString region;
  QRegExp rx("([NS])(\\d\\d)(\\d{2})?\\s*([WE])([\\d]{3})(\\d{2})?");
  int pos = rx.indexIn(strelem);

  while (-1 != pos) {
    QStringList strList = rx.capturedTexts();
    if (strList.size() != 7) {
      pos = rx.indexIn(strelem, pos + 10);
      continue;
    }
   
    float lon = strList.at(5).toFloat() + strList.at(5).toFloat() / 60;
    if (strList.at(4) == "W") {
      lon = -lon;
    }
    lon = MnMath::M180To180(lon);
    float lat = strList.at(2).toFloat() + strList.at(3).toFloat() / 60;
    if (strList.at(3) == "S") {
      lat = -lat;
    }

    region += QString::number(lon, 'f', 2) + " " + QString::number(lat, 'f', 2) + "\\, ";
    pos = rx.indexIn(strelem, pos + 10);
  }

  region.remove(-3, 3);
  
  region = "POLYGON((" + region + "))";
  data->add(descr, region, 0, control::RIGHT);
  
  return true;
}

bool meteo::getValue_ccum(TMeteoData* data, const QString& strelem)
{
  bool ok = false;

  if (0 == data) {
    return ok;
  }

  if (strelem.contains("TCU")) {
    ok = true;
    data->add(TMeteoDescriptor::instance()->descriptor("C"), strelem, 32, control::NO_CONTROL);
  }

  if (strelem.contains("CB")) {
    ok = true;
    data->add(TMeteoDescriptor::instance()->descriptor("C"), strelem, 9, control::NO_CONTROL);
  }
  
  if (strelem.contains("ST")) {
    ok = true;
    data->add(TMeteoDescriptor::instance()->descriptor("C"), strelem, 7, control::NO_CONTROL);
  }
  
  if (strelem.contains("/")) {
    ok = true;
  }

  return ok;
}

float meteo::getValue_trash(const TMeteoData*, const QString& , bool* ok)
{
  *ok = true;
  return BAD_VALUE;
}

float meteo::getValue_status(const TMeteoData*, const QString& strelem, bool* ok)
{
  float val = 15;
  *ok = true;

  if (strelem == "COR") {
    val = 1;
  } else if (strelem == "AMD") {
    val = 2;
  } else if (strelem == "COR AMD") {
    val = 3;
  } else if (strelem == "CNL") {
    val = 4;
  } else if (strelem == "NIL") {
    val = 5;
  } else if (strelem == "SPECI") {
    val = 6;
  } else if (strelem == "SPECI COR") {
    val = 7;
  } else if (strelem.isEmpty()) {
    val = 0;    
  } else {
    *ok = false;
  }

  return val;
}

//n1P1A задает количество последовательных поверхностей
bool meteo::parse_P1seq(const QString& name, const QString& strelem, TMeteoData* data)
{
  if (0 == data) return false;

  QStringList values = strelem.trimmed().split(' ');
  if (values.size() < 2 || values.at(0).length() < 3) return false;
  
  bool ok;
  int cnt = values.at(0).left(1).toUInt(&ok);
  if (!ok || values.size() < cnt + 1 || cnt > 3) {
    return false;
  }

  int P = values.at(0).right(2).toInt(&ok);
  if (!ok) return false;


  QString n_name;
  QString descrName = "P1";
  if (name == "P1_seqA") {
    if (P == 0) P = 1000;
    else if (P == 92) P = 925;
    else P *= 10;
  }

  //  var(P);

  QList<int> Pst;
  Pst << 1000 << 925 << 850 << 700 << 500 << 400 << 300 << 250 
      << 200 << 150 << 100 << 70 << 50 << 30 << 20 << 10;
  
  int pidx = 50;
  for (int idx = 0; idx < Pst.count(); idx++) {
    if (P >= Pst.at(idx)) {
      pidx = idx;
      break;
    }
  }

  for (int idx = 0; idx < cnt; idx++) {
    if (pidx + idx < Pst.count()) {
      TMeteoData& child = data->addChild();
      child.add(TMeteoDescriptor::instance()->descriptor("level_type"), QString::number(meteodescr::kIsobarLevel), meteodescr::kIsobarLevel, control::RIGHT);
      child.add(TMeteoDescriptor::instance()->descriptor("P1"), QString::number(Pst.at(pidx + idx)), 
		Pst.at(pidx + idx), control::NO_CONTROL);
      float ff = meteo::getValue_ft(data, values.at(idx + 1).mid(2, 3), &ok);
      if (!ok) { ff = BAD_VALUE; }
      float dd;
      if (values.at(idx + 1).mid(2, 3).toInt() > 500) {
	dd = values.at(idx + 1).mid(0, 3).toInt(&ok);
      } else {
	dd = values.at(idx + 1).mid(0, 2).toInt(&ok)*10;
      }
      if (!ok) dd = BAD_VALUE;

      child.add(TMeteoDescriptor::instance()->descriptor("dd"), values.at(idx + 1).left(2),
		dd, control::NO_CONTROL);
      child.add(TMeteoDescriptor::instance()->descriptor("ff"), values.at(idx + 1).right(3),
		ff, control::NO_CONTROL);
    }
  }
  
  return true;
}


bool meteo::parse_wind_pilot(const QString& strelem, TMeteoData* parent)
{
  //TODO 
  //добавлять в локальную TMeteoData и при отсутствии ошибок объединить.
  QStringList values = strelem.trimmed().split(' ');

  int p_idx = 0;
  int w_idx = 1;
  float h = 0;
  float d = 0;
  float f = 0;

  int start = 0;
  int factor = 1;
  bool ok = true;

  while (p_idx < values.count() && w_idx < values.count()) {
    if (values.at(p_idx).length() != 5 ||
	values.at(w_idx).length() != 5 ) {
      return false;
    }

    switch (values.at(p_idx)[0].cell()) {
    case '9':
      start = 0;
      factor = 300;
      break;
    case '1':
      start = 30000;
      factor = 300;
      break;
    case '8':
      start = 0;
      factor = 500;
      break;
    default: {
      return false;
    }
    }
  
    for (int num = 0; num < 3; num++) {
      if (p_idx == 0 && w_idx == 1 && values.at(p_idx)[2] == '/') {
	if (w_idx >= values.count()) return false;
	//уровень станции
	h = values.at(p_idx).mid(1,1).toInt(&ok) * factor * 10 + start;
	if (!ok) { h = BAD_VALUE; }
	f = meteo::getValue_ft(parent, values.at(w_idx).mid(2, 3), &ok);
	if (!ok) { f = BAD_VALUE; }

	if (values.at(w_idx).mid(2, 3).toInt() > 500) {
	  d = values.at(w_idx).mid(0, 3).toInt(&ok);
	} else {
	  d = values.at(w_idx).mid(0, 2).toInt(&ok)*10;
	}
	if (!ok) { d = BAD_VALUE; }
	TMeteoData* data = &parent->addChild();
	data->add(TMeteoDescriptor::instance()->descriptor("level_type"), QString::number(meteodescr::kSurfaceLevel), meteodescr::kSurfaceLevel, control::RIGHT);
	data->add(TMeteoDescriptor::instance()->descriptor("h0_station"), values.at(p_idx), h, control::NO_CONTROL);
	data->add(TMeteoDescriptor::instance()->descriptor("dd"), values.at(w_idx).mid(0, 2), d, control::NO_CONTROL);
	data->add(TMeteoDescriptor::instance()->descriptor("ff"), values.at(w_idx).mid(2, 3), f, control::NO_CONTROL);
	w_idx++;
      } else if (values.at(p_idx)[2 + num] != '/') {
	if (w_idx >= values.count()) return false;
	h = (values.at(p_idx).mid(1,1).toInt() * 10 + values.at(p_idx).mid(2 + num,1).toInt(&ok)) * factor + start;
	if (!ok) { h = BAD_VALUE; }
	f = meteo::getValue_ft(parent, values.at(w_idx).mid(2, 3), &ok);
	if (!ok) { f = BAD_VALUE; }
	if (values.at(w_idx).mid(2, 3).toInt() > 500) {
	  d = values.at(w_idx).mid(0, 3).toInt(&ok);
	} else {
	  d = values.at(w_idx).mid(0, 2).toInt(&ok)*10;
	}
	if (!ok) { d = BAD_VALUE; }
	TMeteoData* data = &parent->addChild();
	data->add(TMeteoDescriptor::instance()->descriptor("level_type"), "", meteodescr::kGeopotentialLevel, control::RIGHT);
	data->add(TMeteoDescriptor::instance()->descriptor("hgpm"), values.at(p_idx), h, control::NO_CONTROL);
	data->add(TMeteoDescriptor::instance()->descriptor("dd"), values.at(w_idx).mid(0, 2), d, control::NO_CONTROL);
	data->add(TMeteoDescriptor::instance()->descriptor("ff"), values.at(w_idx).mid(2, 3), f, control::NO_CONTROL);
	w_idx++;
      }
    }
  
    p_idx = w_idx;
    w_idx += 1;
  }

  return true;
}
