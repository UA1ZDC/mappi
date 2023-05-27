#include <stdlib.h>

#include <qbytearray.h>
#include <qfile.h>

#include <commons/compresslib/mcompress.h>

int main( int argc, char* argv[] )
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);
  QFile inf("./test.tlg");
  inf.open(QIODevice::ReadOnly);
  QByteArray in = inf.readAll();
  compress::Compress cmprs;
  QByteArray out = cmprs.compress(in);
  QFile cmprsf("./test.tlg.cmprs");
  cmprsf.open(QIODevice::WriteOnly);
  cmprsf.write(out);
  cmprsf.flush();
  cmprsf.close();

  QByteArray res = cmprs.decompress(out);
  QFile outf("./test.res.tlg");
  outf.open(QIODevice::WriteOnly);
  outf.write(res);
  outf.flush();
  outf.close();
  return EXIT_SUCCESS;
}
