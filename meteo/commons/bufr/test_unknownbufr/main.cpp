#include <qdebug.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qfile.h>
#include <qtextcodec.h>

#define SEQ_FILE MnCommon::projectPath() + "/var/meteo/bufr_unknown.txt"


bool consitsDescr(QList<QList<QString> >& dset, QList<QString>& cur, QVector<int>& repeats)
{
  for (int i = 0; i < dset.size(); ++i) {
    if (dset.at(i) == cur) {
      repeats[i]++;
      return true;
    }
  }

  return false;
}

int main(int argc, char** argv)
{
  if (argc < 3) {
    qDebug() << "Need point <src file name> and <dst file name>";
    return -1;
  }

  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale());
  
  QString  srcFile = argv[1];
  QString  resFile = argv[2];

  QFile file(srcFile);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() <<" no file" << srcFile;
  }

  QList<QList<QString> > dset;
  QList<QString> cur;
  QVector<int> repeats(75);
  int cnt = 0;
  QString fileCur;
  int categoryCur;
  QList<QString> files;
  QList<int> categ;
  QList<int> categSort;

  QTextStream in(&file);
  bool flag = false;

  while (!in.atEnd()) {
    QString line = in.readLine().trimmed();
    if (line.contains("home") || line.contains("source")) {
      fileCur = line;
    } else if (line.contains("category")) {
      categoryCur = line.remove("category").toInt();
    } 

    if (line == "end") {
      flag = false;
      ++cnt;
      if (!consitsDescr(dset, cur, repeats)) {
	dset.append(cur);
	files.append(fileCur);
	categ.append(categoryCur);
	if (!categSort.contains(categoryCur)) {
	  categSort.append(categoryCur);
	}
      }
      cur.clear();
    }

    if (flag) {
      cur.append(line.remove('"'));
    }

    if (line == "start") {
      flag = true;
    }
  }

  file.close();

  qDebug() << "BUFR sets count=" << dset.count();

  file.setFileName(resFile);
  file.open(QIODevice::WriteOnly);
  QTextStream out(&file);

  int max = 0;
  
  qSort(categSort);

  for (int cIdx = 0; cIdx < categSort.count(); cIdx++) {
    for (int i = 0; i < dset.size(); ++i) {
      if (categ.at(i) != categSort.at(cIdx)) continue;
      // out << files.at(i) << "\n";
      out<< "start num="<<i<<" repeats="<<(repeats.at(i)+1)<<"\n";
      out << QString("<!-- category %1 -->\n").arg(categ.at(i));
      out << "<sequence_todo number=\"\" station_type=\"500\">\n";
      if (max < (repeats.at(i)+1)) {
	max = (repeats.at(i)+1);
      }
      for (int j = 0; j < dset.at(i).size(); ++j) {
	out << "  <descr num=" << '"' << dset.at(i).at(j).rightJustified(6, '0') << '"' << "></descr>" << "\n";
      }
      out << "</sequence_todo>\n";
      //    out<< "end\n";
      out<< "\n";
    }
  }

  out<<"cnt ="<<cnt << " max="<<max;
  file.close();

  return 0;
}
