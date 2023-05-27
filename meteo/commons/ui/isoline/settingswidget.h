#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QtWidgets>
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/global/gradientparams.h>

namespace Ui{
 class Settings;
 class AddIsolineWidget;
}

namespace meteo {
  namespace settings {
    namespace internal {
      class TMeteoSettings;
    }
  }
  
  namespace sprinf {
    class MeteoParameters;
  }

namespace map {


class TreeWidget : public QTreeWidget{
  Q_OBJECT
public:
  TreeWidget(QWidget* parent = 0);
protected:
  void mouseDoubleClickEvent(QMouseEvent *event);
signals:
  void iconDoubleClicked(QTreeWidgetItem* item, int col, bool isicon);
};

class LineLabel : public QLabel{
  Q_OBJECT
public:
  LineLabel(QWidget* parent = 0);
  LineLabel(QColor color = Qt::black, Qt::PenStyle style = Qt::SolidLine, int width = 2, QWidget* parent = 0);
  void setColor(QColor color);
  void setStyle(Qt::PenStyle style);
  void setWidth(int width);
  const QColor color() const;
  Qt::PenStyle style() const;
  int width() const;
  QPixmap pixmap() const;
private:
  QColor color_;
  Qt::PenStyle style_;
  int width_;
  QPixmap pix_;
  void paint();
};

class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    LineEdit(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent *);

private slots:
    void updateCloseButton(const QString &text);

private:
    QToolButton *clearButton;
};

class ComboBoxItem : public QComboBox
{
  Q_OBJECT
  public:
  ComboBoxItem(QTreeWidgetItem*, int, int);
  void fill(const QStringList& values, const QString& current);
		
public slots:
  void changeItem(int);

private:
  void wheelEvent(QWheelEvent *e) { QWidget::wheelEvent(e); }

private:
  QTreeWidgetItem *item_;
  int column_;
};

class AddIsolineWidget : public QDialog {
  Q_OBJECT
public:
  AddIsolineWidget(QWidget *parent = 0);
  ~AddIsolineWidget();

  void setOptions(const QString& options);
  int index() const;
  QString name() const;
		      
public slots:
  virtual void acceptIsoline();

private slots:
  void slotIndexChanged();
  void slotFindIsoline(const QString& text);

private:
  Ui::AddIsolineWidget* ui_;
  QMap<int32_t, QTreeWidgetItem*> items_;
  meteo::sprinf::MeteoParameters* response_;
};

class SettingsWidget : public QWidget
{
  Q_OBJECT
public:
  SettingsWidget( app::MainWindow* parent);
  ~SettingsWidget();

  void setCurrentIsoline( int descr );

protected:
  void keyReleaseEvent( QKeyEvent * e);
  void showEvent(QCloseEvent *);
  void closeEvent(QCloseEvent *);
  
private:
  Ui::Settings* ui_;
  meteo::map::GradientParams isoparams_; //!< Параметры изолиний
  QString edit_;
  TreeWidget* leveltree_; //!< gui для отображения параметров изолинии по уровням
  AddIsolineWidget* addwgt_; //!< gui для добавления изолиний
  bool isChanged_; //!< true - есть несохраненные настройки
  QMap<int, QString> levelTypes_; //!< типы изолиний и их названия 

private:
  void setupLineStyle();
  void setupLineWidth();
  void setupLineLevels();
  void showAllItems();
  QColor colorMin() const ;
  QColor colorMax() const ;
  double minIsoValue() const ;
  double maxIsoValue() const ;
  double isoStep() const ;
  void fillLevelsTree(int isoline_id);
  void isoSettingsFromItem(const QTreeWidgetItem* lineItem, meteo::map::proto::FieldColor* iso);
  void setupGradientParamsFromItems(meteo::map::proto::FieldColor* isofill);
  void isoSettingsFromItem(QTreeWidgetItem* item);
  void setMarked(QTreeWidgetItem* item);
  void proto2ui();
  void setupTree();
    
  void saveBannedTypes();
  void fillLevelTypes();
private slots:
  void slotColorMinDialog();
  void slotColorMaxDialog();
  void slotColorFillDialog();
  void slotAddIsoline();
  void slotSaveIsoline();
  void slotLineStyleSelected(int);
  void slotLineWidthSelected(int);
  void slotFontSelected(int);
  void slotRemoveIsoline();
  void slotSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
  void uiFromCurrentItem();
  void slotItemDoubleClicked(QTreeWidgetItem* item, int col, bool isicon);
  void slotItemDoubleClicked(QTreeWidgetItem*,int);
  void slotItemChanged(QTreeWidgetItem*, int);
  void slotAddWidget();
  void slotSetChanged();
  void slotClose();
  void addIsoLevel();
  void removeIsoLevel();
signals:
  void updateSettings();
};

}
}

#endif // SETTINGSWIDGET_H
