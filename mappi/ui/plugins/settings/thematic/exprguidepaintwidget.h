#ifndef MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_EXPRGUIDEPAINTWIDGET_H
#define MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_EXPRGUIDEPAINTWIDGET_H

#include <qwidget.h>

#include <mappi/ui/exprguid/exprmake/texpr_builder.h>

class ExprGuidePaintWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ExprGuidePaintWidget(QWidget *parent = 0);
    ~ExprGuidePaintWidget();
  QString expr();

signals:
  void finishBuildExpr(const TExpr_Error& _error);

public slots:
  void showAxisX(bool value);
  void showAxisY(bool value);
  void setFontFamily(const QString& family);
  void setFontSize(int size);
  void setExpr(const QString &_expr);

protected:
  virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

private:
  void paintAxiss();
  void paintExpr();

private:
  bool showAxisX_;
  bool showAxisY_;
  ExprDraw::TExprHorAlign hAligment_;
  ExprDraw::TExprVertAlign vAligment_;
  QString fontFamily_;
  int fontSize_;
  QString expr_;
  TExpr_Builder exprBuilder_;
  TExpr_Class* exprClass_;
};

#endif // MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_EXPRGUIDEPAINTWIDGET_H
