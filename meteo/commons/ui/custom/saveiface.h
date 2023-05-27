#ifndef METEO_COMMONS_UI_PLUGINS_ANIMATION_SAVE_H
#define METEO_COMMONS_UI_PLUGINS_ANIMATION_SAVE_H

#include <qdialog.h>
#include <meteo/commons/proto/exportmap.pb.h>

namespace Ui {
  class SaveIface;
}


namespace meteo {
  
  class SaveIface : public QDialog {
    Q_OBJECT
    
    public:
    SaveIface(const QString& confFile, QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~SaveIface();

    void setDefaultDoc(const QString& name);
    void setDefaultFile(const QString& name, const QString& filter);

    void setDefaultDir(const QString& name);
    QString defaultDir() const;
    
    bool isSaveInner(QString* name = 0);
    bool isSaveFile(QString* name = 0);
   
  signals:
      void saveClicked();
			
  private slots:
    void chooseFile();
    void save();
    
  private:
    
    void readConfig();
    void saveConfig(const QString& dir);
    
  private:
    Ui::SaveIface* _ui;

    QString _confFile;
    meteo::map::exportmap::ExportPath _conf;
    QString _filter;
  };
}

#endif
