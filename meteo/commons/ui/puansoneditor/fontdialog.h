#ifndef METEO_COMMONS_UI_MAP_VIEW_PUANSON_FONTDIALOG_H
#define METEO_COMMONS_UI_MAP_VIEW_PUANSON_FONTDIALOG_H

#include <qdialog.h>
#include <qmap.h>

namespace Ui {
  class FontDialog;
}

namespace meteo {
namespace puanson {

class Editor;

class FontDialog : public QDialog
{
  Q_OBJECT
  public:
    FontDialog( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~FontDialog();

  private:
    Ui::FontDialog* ui_;
    const QMap< int, QString > weights_;

    void setFont();

  private slots:
    void slotFontChanged();
    void slotWeightChanged();
    void slotSizeChanged();

    void slotItalicToggled( bool fl );
    void slotUnderlineToggled( bool fl );

  friend class Editor;
};

}
}

#endif
