#include "drawtoolbarwidget.h"
#include "ui_drawtoolbarwidget.h"

#include <qlabel.h>

#include <cross-commons/app/paths.h>

#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>

#include "frontpropwidget.h"
#include "polygonpropwidget.h"

namespace meteo {
namespace map {

const QString kNormalStyle  = QString("QToolButton { color: #000000; }");
const QString kCurrentStyle = QString("QToolButton { color: #1010ff; }");

DrawToolbarWidget::DrawToolbarWidget(QWidget* parent)
  : QWidget(parent),
    currentTool_(kCursor),
    opt_(new QSettings(QDir::homePath() + "/." + MnCommon::applicationName() + "/drawtools.ini", QSettings::IniFormat))
{
  ui_ = new Ui::DrawToolbarWidget;
  ui_->setupUi(this);

  setWindowFlags(Qt::Tool);
  setWindowTitle(" ");

  lineBtn_ = new ActionButton();
  textBtn_ = new ActionButton();
  frontBtn_ = new ActionButton();
  polygonBtn_ = new ActionButton();
  pixmapBtn_ = new ActionButton();
  groupBtn_ = new ActionButton();
  cursorBtn_ = new ActionButton();
  ui_->lineLayout->addWidget(lineBtn_);
  ui_->textLayout->addWidget(textBtn_);
  ui_->frontLayout->addWidget(frontBtn_);
  ui_->polygonLayout->addWidget(polygonBtn_);
  ui_->pixmapLayout->addWidget(pixmapBtn_);
  ui_->groupLayout->addWidget(groupBtn_);
  ui_->cursorLayout->addWidget(cursorBtn_);
  lineBtn_->setIcon(QIcon(":/meteo/icons/drawtools/polyline.png"));
  textBtn_->setIcon(QIcon(":/meteo/icons/drawtools/text.png"));
  frontBtn_->setIcon(QIcon(":/meteo/icons/drawtools/fronthol.png"));
  polygonBtn_->setIcon(QIcon(":/meteo/icons/drawtools/polygon.png"));
  pixmapBtn_->setIcon(QIcon(":/meteo/icons/misc/windowico.png"));
  groupBtn_->setIcon(QIcon(":/meteo/icons/drawtools/group.png"));
  cursorBtn_->setIcon(QIcon(":/meteo/icons/drawtools/cursor_arrow_new.png"));
  cursorBtn_->setIconSize(QSize(18,18));
  lineBtn_->setToolTip(QObject::tr("Линия"));
  textBtn_->setToolTip(QObject::tr("Текст"));
  frontBtn_->setToolTip(QObject::tr("Фронт"));
  polygonBtn_->setToolTip(QObject::tr("Замкнутая линия"));
  pixmapBtn_->setToolTip(QObject::tr("Изображение"));
  groupBtn_->setToolTip(QObject::tr("Сгруппировать"));
  cursorBtn_->setToolTip(QObject::tr("Курсор"));
  toolButtons_.insert(kCursor, cursorBtn_);
  toolButtons_.insert(kLine, lineBtn_);
  toolButtons_.insert(kPolygon, polygonBtn_);
  toolButtons_.insert(kFront, frontBtn_);
  toolButtons_.insert(kText, textBtn_);
  toolButtons_.insert(kGroup, groupBtn_);
  toolButtons_.insert(kPixmap, pixmapBtn_);
  setAttribute( Qt::WA_TranslucentBackground, true );

  QMap<Tool, ActionButton*>::iterator cur = toolButtons_.begin();
  QMap<Tool, ActionButton*>::iterator end = toolButtons_.end();
  for ( ; cur!=end; ++cur ) {
    QObject::connect( cur.value(), SIGNAL(clicked(bool)), SLOT(slotSwitchTool()) );
  }
  QObject::connect(textBtn_, &ActionButton::hovered, [=](){ if ( true == textBtn_->isChecked() ) { emit textHovered(); }} );
  QObject::connect(textBtn_, &ActionButton::leave, [=](){ if ( true == textBtn_->isChecked() ) { emit textLeave(); }});
  setCurrentTool(kCursor);
  slotSetupGui();

  QByteArray ba = opt_->value("toolbar/geometry", QByteArray()).toByteArray();
  if ( !ba.isEmpty() ) {
    restoreGeometry(ba);
  }

  // TODO функциональность кнопок отменить/повторить необходимо доработать
  undoBtn_ = new ActionButton();
  ui_->undoLayout->addWidget(undoBtn_);
  undoBtn_->setIcon(QIcon(":/meteo/icons/tools/undo.png"));
  redoBtn_ = new ActionButton();
  ui_->redoLayout->addWidget(redoBtn_);
  redoBtn_->setIcon(QIcon(":/meteo/icons/tools/redo.png"));
  undoBtn_->hide();
  redoBtn_->hide();
//  connect( ui_->undoBtn, SIGNAL(clicked(bool)), SIGNAL(undo()) );
//  connect( ui_->redoBtn, SIGNAL(clicked(bool)), SIGNAL(redo()) );
  onShowSignals();
}

DrawToolbarWidget::~DrawToolbarWidget()
{
  delete ui_;
  ui_ = nullptr;

  opt_->deleteLater();
  opt_ = nullptr;
}

DrawToolbarWidget::Tool DrawToolbarWidget::currentTool() const
{
   return currentTool_;
}

void DrawToolbarWidget::setCurrentTool(DrawToolbarWidget::Tool tool)
{
  if ( currentTool() != kNoTool ) {
    toolButtons_[currentTool()]->setChecked(true);
  }

  if ( currentTool() == tool ) {
    if ( currentTool() != kNoTool ) {
      toolButtons_[currentTool()]->setChecked(false);
    }
    currentTool_ = Tool::kNoTool;
  }
  else {
    currentTool_ = tool;
  }
  slotSetupGui();

  emit toolChanged(currentTool_);
}

void DrawToolbarWidget::setPixmapIcon(QString& str)
{
  if (false == str.isEmpty()) {
    pixmapBtn_->setIcon(QIcon(str));
  }
}

void DrawToolbarWidget::slotSetupGui()
{
  QMap<Tool, ActionButton*>::iterator cur = toolButtons_.begin();
  QMap<Tool, ActionButton*>::iterator end = toolButtons_.end();
  for ( ; cur!=end; ++cur ) {
    cur.value()->setChecked(false);
  }

  if ( kNoTool == currentTool_ ) {
    onShowSignals();
    return;
  }
  offShowSignals();
  toolButtons_[currentTool_]->setChecked(true);
}

void DrawToolbarWidget::slotSwitchTool()
{
  ActionButton* btn = qobject_cast<ActionButton*>(sender());
  if ( nullptr == btn ) {
    return;
  }
  setCurrentTool(toolButtons_.key(btn));
}

void DrawToolbarWidget::slotShowTool()
{
  ActionButton* btn = qobject_cast<ActionButton*>(sender());
  if ( nullptr == btn ) {
    return;
  }
  emit toolShow(toolButtons_.key(btn));
}

void DrawToolbarWidget::slotHideTool()
{
  ActionButton* btn = qobject_cast<ActionButton*>(sender());
  if ( nullptr == btn ) {
    return;
  }
  emit toolHide(toolButtons_.key(btn));
}

void DrawToolbarWidget::resizeEvent(QResizeEvent* e)
{
  opt_->setValue("toolbar/geometry", saveGeometry());
  QWidget::resizeEvent(e);
}

void DrawToolbarWidget::moveEvent(QMoveEvent* e)
{
  opt_->setValue("toolbar/geometry", saveGeometry());

  QWidget::moveEvent(e);
}

void DrawToolbarWidget::closeEvent(QCloseEvent* e)
{
  e->accept();
  emit closed();
}

void DrawToolbarWidget::onShowSignals()
{
  QMap<Tool, ActionButton*>::iterator cur = toolButtons_.begin();
  QMap<Tool, ActionButton*>::iterator end = toolButtons_.end();
  for ( ; cur!=end; ++cur ) {
    QObject::connect( cur.value(), SIGNAL( hovered() ), SLOT( slotShowTool() ) );
    QObject::connect( cur.value(), SIGNAL( leave() ), SLOT( slotHideTool() ) );
  }
}

void DrawToolbarWidget::offShowSignals()
{
  QMap<Tool, ActionButton*>::iterator cur = toolButtons_.begin();
  QMap<Tool, ActionButton*>::iterator end = toolButtons_.end();
  for ( ; cur!=end; ++cur ) {
    QObject::disconnect( cur.value(), SIGNAL( hovered() ), this, SLOT( slotShowTool() ) );
    QObject::disconnect( cur.value(), SIGNAL( leave() ), this, SLOT( slotHideTool() ) );
  }
}

} // map
} // meteo
