#ifndef METEO_COMMONS_UI_MAP_FORMALDOC_H
#define METEO_COMMONS_UI_MAP_FORMALDOC_H

#include <meteo/commons/ui/map/document.h>

namespace meteo {
namespace map {

class FormalDoc : public Document
{
  public:
    FormalDoc(const proto::Document& doc);
    FormalDoc();
    ~FormalDoc();

    enum {
      Type = kFormal
    };

    void setFile(const QByteArray bar);
    void setFilename(const QString name);

    int type() const { return Type; }

    const QByteArray& getFile() const;
    const QString& getFilename() const;

  private:
    QByteArray file_;
    QString filename_;
};

}
}

#endif
