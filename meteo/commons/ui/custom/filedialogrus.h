#ifndef METEO_COMMONS_UI_CUSTOM_FILEDIALOGRUS_H
#define METEO_COMMONS_UI_CUSTOM_FILEDIALOGRUS_H

#include <qfiledialog.h>

namespace meteo {
  class FileDialog : public QFileDialog {
  public:
    FileDialog(QWidget * parent, Qt::WindowFlags flags);
    FileDialog(QWidget * parent = 0, const QString & caption = QString(), 
	       const QString & directory = QString(), const QString & filter = QString());

    virtual ~FileDialog();
    
    static QString getSaveFileName(QWidget * parent = 0, const QString & caption = QString(), 
				   const QString & dir = QString(), const QString & filter = QString(), 
				   QString * selectedFilter = 0, Options options = 0);


    static QString getOpenFileName(QWidget * parent = 0, const QString & caption = QString(), 
				   const QString & dir = QString(), const QString & filter = QString(), 
				   QString * selectedFilter = 0, Options options = 0);

    void setButtons();

  };

}

#endif

