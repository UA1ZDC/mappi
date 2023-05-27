#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_PRINTDOC_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_PRINTDOC_H

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/ui/map/view/widgets/layersselect.h>
#include <meteo/commons/ui/map/view/actions/selectaction.h>
#include <meteo/commons/ui/map/view/mapwindow.h>

#include <qprinter.h>

namespace Ui {
  class PrintDoc;
}

namespace meteo {
namespace map {

  class PrintPreview;

  class Document;

  class PrintDoc : public QWidget, public SelectActionIface
  {
    Q_OBJECT

    public:
    PrintDoc( MapWindow* parent );
    ~PrintDoc();

    void setOptions(const QString& options);

    bool isRubberPressed() const;
    void rubberFinished();
    void resetRubber(MapScene* scene); 
    void removeRubber();
    void fillCoordsFromDocument(map::Document* doc = 0);

    QPrinter* printer() { return printer_; }

  public slots:
    void finished(int result);
    void redrawPreview();

  protected:
    void setPoints(const GeoPoint& nw, const GeoPoint& ne, const GeoPoint& sw, const GeoPoint& se, const QRect& rect);		
		      
  private slots:
    //void slotPrint();
    //    void slotCancel();
    void slotSceneChanged(MapScene* scene);
    //void drawPreview(QPrinter* printer);
    void bandChanged();
    void previewStateChaged(int state);

    void slotRubberClicked(bool on);
    void changeFitMode(int index);
    void changedFixedBandMode(int state);
    void previewClosed();

  private:
    MapView* mapview() const { if (0 == mapwindow_) return 0;  return mapwindow_->mapview(); }
    Document* currentDocument() const;

    void close();
    bool eventFilter(QObject* watched, QEvent* event);
    void showEvent(QShowEvent *);

    void resetWatchedDocument();
    bool installWatchedDocument();

    void setPrinterSettings();
    QRect calcFixedBand();
    void calcRubberBand(int* colCnt, int* rowCnt);
    void calcOffset(int colCnt, int rowCnt, int* woffset, int* hoffset);

    void redrawPreview(int colCnt, int rowCnt, int woffset, int hoffset);
    void redrawPreview( int woffset, int hoffset);

  private:
    MapWindow* mapwindow_;
    Ui::PrintDoc* ui_;
    LayersSelect* layers_;
    QRect mapBand_; //!< рамка на карте
    QRect targetBand_; //!< рамка на листах
    QPrinter* printer_;

    QObject* watchedEventHandler_;

    PrintPreview* previewDialog_;

    QList<QPrinter::PageSize> paperSizes_;
    bool initOk_;
  };
  
}

}


#endif
