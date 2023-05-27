#ifndef METEO_COMMONS_ALPHANUM_XMLLINEDECODE_H
#define METEO_COMMONS_ALPHANUM_XMLLINEDECODE_H


#include <QMap>

class QDomNode;
class QDomElement;
class TMeteoData;

namespace meteo {
  namespace anc {

    class XmlLineDecoder {
    public:
      XmlLineDecoder();
      ~XmlLineDecoder();

      //void clear();
      void startBlock();
            
      bool setMeteoDataValue(const QString& code, const QDomNode& node, const QString& astr, TMeteoData* data);

    private:
      
      TMeteoData* currentMeteoData(const QString& code, QDomElement elem, TMeteoData* data);
      
    private:
      
      QMap<int, TMeteoData*> _childs; //!< Для связи 
      QMap<int, TMeteoData*> _inchilds; //!< Для связи внутри последнего звена (тег Value)
      
    };
    
  }
}

#endif
