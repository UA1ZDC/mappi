#include "formaldoc.h"

namespace meteo {
namespace map {

FormalDoc::FormalDoc(const proto::Document &doc)
  : Document( doc )
{
}

FormalDoc::FormalDoc()
 : Document()
{
}

FormalDoc::~FormalDoc()
{
}

void FormalDoc::setFile(const QByteArray bar)
{
  file_ = bar;
}

void FormalDoc::setFilename(const QString name)
{
  filename_ = name;
}

const QByteArray& FormalDoc::getFile() const
{
  return file_;
}

const QString& FormalDoc::getFilename() const
{
  return filename_;
}

}
}
