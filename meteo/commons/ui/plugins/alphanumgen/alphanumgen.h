#ifndef METEO_COMMONS_UI_PLUGINS_ALPHANUMGEN_H
#define METEO_COMMONS_UI_PLUGINS_ALPHANUMGEN_H

#include <qhash.h>

#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/ui/plugins/alphanumgen/ancgen.pb.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/rpc.h>

class QString;
class QTreeWidget;
class QTreeWidgetItem;
class TMeteoData;

namespace rpc {
  class TController;
}


namespace meteo {
  namespace anc {
    class AncSimpleTemplate;
    class Param;

    typedef QString (*FuncGetCode)(float val);
    typedef QString (*FuncGetCodeCustom)(const QMap<QString, float>& val);

    QString func_VVvis(float val);
    QString func_VVinstr(float val);
    QString func_hshsVis(float val);
    QString func_hshsInstr(float val);
    QString func_RR(float val);
    QString func_ss(float val);
    void func_ddfff(const QString& ddval, const QString& ffval, QString* ddcode, QString* ffcode);
  }
}

namespace meteo {
  class AlphanumGen {
  public:
    AlphanumGen(const QString& file);
    ~AlphanumGen();

    bool readTemplate(const QString& confFile);    
    QString generate();
    void decode(const QByteArray& ba, QTreeWidget* view);

    void setRequire(const QString& setName, bool require);
    bool setCode(const QString& name, const QString& val);
    bool setValue(const QString& name, float val);
    bool setValue(const QString& name, const QString& val);
    void clearValue(const QString& name);
    bool setValue(anc::Param* param, float val);
    bool setValue(anc::Param* param, const QString& val);

    void clearGroupList(const QString& name);
    int addListValue(const QString& groupName, const QString& name, float value);
    template<class T> void setListValue(const QString& group, int idx, const QString& name, T value);

    void clearGroupSet(const QString& name);
    int addGroupSetValue(const QString& setName, const QString& name, float value);
    template<class T> void setGroupSetValue(const QString& setName, int idx, const QString& name, T value);
    void setGroupSetRequire(const QString& setName, int sidx, const QString& name, bool require);

    void saveDefaultDir(const QString& conf, const QString& dir) const;
    QString readDefaultDir(const QString& conf);
    QString chooseFile(const QString& conf, const QString& name, QWidget* parent);

  private:
    anc::Param* findParam(const QString& name);
    QString tableConvert( anc::Param* param, float value);
    QString functionConvert( anc::Param* param, float val);

    anc::Group* findGroup(const QString& name);
    anc::Section* findGroupSet(const QString& name);

    void registerFunc(const QStringList& dl, anc::FuncGetCode func);
    void registerFunc();

    QString paramCode(const ::google::protobuf::RepeatedPtrField< ::meteo::anc::Param >& param, 
		      bool required, int* length, bool* empty);
    QString groupsCode(const ::google::protobuf::RepeatedPtrField< ::meteo::anc::Group>& mgroup,
		       int* length, bool* sectEmpty);

    void fillCoord(TMeteoData* data);
    void fillDecodeView(const TMeteoData* md, QTreeWidget* view, QTreeWidgetItem* parent = 0);

  private:
    anc::AncTemplate* _templ; //! Правило формирования 
    meteo::rpc::Channel* _ctrl;
    QHash<QString, anc::FuncGetCode> _funcs; //!< Функции для преобразования числa в кодовое значение
    QHash<QString, anc::FuncGetCodeCustom> _funcsCustom; //!< Функции для преобразования числa в кодовое значение

    meteo::sprinf::Station _station;

    QString defaultDir_;
  };

  //-----------------

  //! Установка значения параметра из набора
  template<class T> void AlphanumGen::setListValue(const QString& groupName, int idx, const QString& name, T value)
  {
    anc::Group* group = findGroup(groupName);
    if (0 == group || idx < 0 ||  group->plist_size() <= idx) return;
    
    anc::ParamList* plist = group->mutable_plist(idx);
    
    if (plist->param_size() == 0) {
      plist->mutable_param()->CopyFrom(group->param());
    }
    
    for (int pidx = 0; pidx < plist->param_size(); pidx++) {
      if (QString::fromStdString(plist->param(pidx).name()) == name) {
	setValue(plist->mutable_param(pidx), value);
	break;
      }
    }
  }
  
  //! Установка значения параметра из набора групп
  template<class T> void AlphanumGen::setGroupSetValue(const QString& setName, int sidx, const QString& name, T value)
  {
    anc::Section* sect = findGroupSet(setName);
    if (0 == sect || sidx < 0 || sidx > sect->mgroupset_size()) return;

    anc::GroupList* gset = sect->mutable_mgroupset(sidx);
    if (gset == 0) return;
    for (int gidx = 0; gidx < gset->mgroup_size(); gidx++) {
      for (int pidx = 0; pidx < gset->mgroup(gidx).param_size(); pidx++) {
	if (QString::fromStdString(gset->mgroup(gidx).param(pidx).name()) == name) {
	  setValue(gset->mutable_mgroup(gidx)->mutable_param(pidx), value);
	  return;
	}
      }
    }
  }


}

#endif
