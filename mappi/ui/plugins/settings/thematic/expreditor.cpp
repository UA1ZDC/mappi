#include "expreditor.h"
#include "ui_expreditor.h"
#include "variableeditor.h"
#include "exprguidepaintwidget.h"

#include <qsettings.h>
#include <qdir.h>
#include <qmenu.h>

static QString kSettings = QDir::homePath() + "/.meteo/mappi/settings.ini";

namespace mappi {

ExprEditor::ExprEditor(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::ExprEditor)
, variableEditor_(new VariableEditor(this))
{
  ui_->setupUi(this);

  QHBoxLayout *layout = new QHBoxLayout(ui_->exprViewWdg);
  exprVis_ = new ExprGuidePaintWidget(this);
  layout->addWidget(exprVis_);
  ui_->exprViewWdg->setLayout(layout);
  ui_->variableBtn->setMenu(new QMenu(ui_->variableBtn));
  //connect calc button
  {
    connect(ui_->zeroBtn,   &QPushButton::clicked, [=](){ ui_->exprEdt->insert("0"); });
    connect(ui_->oneBtn,    &QPushButton::clicked, [=](){ ui_->exprEdt->insert("1"); });
    connect(ui_->twoBtn,    &QPushButton::clicked, [=](){ ui_->exprEdt->insert("2"); });
    connect(ui_->threeBtn,  &QPushButton::clicked, [=](){ ui_->exprEdt->insert("3"); });
    connect(ui_->fourBtn,   &QPushButton::clicked, [=](){ ui_->exprEdt->insert("4"); });
    connect(ui_->fiveBtn,   &QPushButton::clicked, [=](){ ui_->exprEdt->insert("5"); });
    connect(ui_->sixBtn,    &QPushButton::clicked, [=](){ ui_->exprEdt->insert("6"); });
    connect(ui_->sevenBtn,  &QPushButton::clicked, [=](){ ui_->exprEdt->insert("7"); });
    connect(ui_->eightBtn,  &QPushButton::clicked, [=](){ ui_->exprEdt->insert("8"); });
    connect(ui_->nineBtn,   &QPushButton::clicked, [=](){ ui_->exprEdt->insert("9"); });

    connect(ui_->addBtn,    &QPushButton::clicked, [=](){ ui_->exprEdt->insert("+"); });
    connect(ui_->subBtn,    &QPushButton::clicked, [=](){ ui_->exprEdt->insert("-"); });
    connect(ui_->mulBtn,    &QPushButton::clicked, [=](){ ui_->exprEdt->insert("*"); });
    connect(ui_->divBtn,    &QPushButton::clicked, [=](){ ui_->exprEdt->insert("/"); });
    connect(ui_->modBtn,    &QPushButton::clicked, [=](){ ui_->exprEdt->insert("%"); });
    connect(ui_->powBtn,    &QPushButton::clicked, [=](){ ui_->exprEdt->insert("^"); });
    connect(ui_->factBtn,   &QPushButton::clicked, [=](){ ui_->exprEdt->insert("!"); });

    connect(ui_->rBrackBtn, &QPushButton::clicked, [=](){ ui_->exprEdt->insert(")"); });

    connect(ui_->backspaceBtn, &QPushButton::clicked, [=](){ ui_->exprEdt->backspace(); });
    connect(ui_->delBtn,       &QPushButton::clicked, [=](){ ui_->exprEdt->del();       });
    connect(ui_->clearBtn,     &QPushButton::clicked, [=](){ ui_->exprEdt->clear();     });
    connect(ui_->undoBtn,      &QPushButton::clicked, [=](){ ui_->exprEdt->undo();      });
    connect(ui_->redoBtn,      &QPushButton::clicked, [=](){ ui_->exprEdt->redo();      });

    connect(ui_->sinBtn, &QPushButton::clicked, this, &ExprEditor::slotSin);
    connect(ui_->cosBtn, &QPushButton::clicked, this, &ExprEditor::slotCos);
    connect(ui_->tanBtn, &QPushButton::clicked, this, &ExprEditor::slotTan);
    connect(ui_->ctgBtn, &QPushButton::clicked, this, &ExprEditor::slotCtg);

    connect(ui_->commaBtn,  &QPushButton::clicked, this, &ExprEditor::slotComma);
    connect(ui_->absBtn,    &QPushButton::clicked, this, &ExprEditor::slotAbs);
    connect(ui_->lBrackBtn, &QPushButton::clicked, this, &ExprEditor::slotLeftBracket);

    connect(ui_->leftBtn,  &QPushButton::clicked, [=](){ ui_->exprEdt->cursorBackward(false); ui_->exprEdt->setFocus(); });
    connect(ui_->rightBtn, &QPushButton::clicked, [=](){ ui_->exprEdt->cursorForward(false);  ui_->exprEdt->setFocus(); });
    connect(ui_->enterBtn, &QPushButton::clicked, this, &ExprEditor::complited);

    connect(ui_->exprEdt, &QLineEdit::textChanged, this, &ExprEditor::refreshExpr);
  }
  connect(ui_->exprEdt,      &QLineEdit::selectionChanged,   this, &ExprEditor::slotSelectionText);
  connect(ui_->variableBtn,  &QToolButton::clicked,          ui_->variableBtn, &QToolButton::showMenu);
  connect(variableEditor_,   &VariableEditor::compliteVars,   this, &ExprEditor::slotSetVars);
  connect(ui_->addVariable,  &QPushButton::clicked,          [=](){ variableEditor_->open(vars_); });

  loadSettings(kSettings);
}

ExprEditor::~ExprEditor()
{
  saveSettings(kSettings);
  delete variableEditor_;
  delete ui_;
}

void ExprEditor::loadSettings(const QString &filename)
{
  QFile file(filename);
  if(false == file.exists()) return;

  QSettings settings(filename, QSettings::IniFormat);
  variableEditor_->restoreGeometry(settings.value("expreditor/channeleditor").toByteArray());
}
void ExprEditor::saveSettings(const QString &filename)
{
  QSettings settings(filename, QSettings::IniFormat);
  settings.setValue("expreditor/channeleditor", variableEditor_->saveGeometry());
}

void ExprEditor::open(const QString& expr, const QVector<Variable>& vars)
{
  ui_->exprEdt->setText(expr);
  ui_->variableBtn->menu()->clear();
  for(auto it : vars) addNewVariable(it);
  refreshExpr();
  show();
}
void ExprEditor::complited()
{
  Q_EMIT(complite(ui_->exprEdt->text(), vars_));
  hide();
}

void ExprEditor::addNewVariable(const Variable& variable)
{
  if(variable.has_name()) {
    QString name = QString::fromStdString(variable.name());
    vars_.insert(name, variable);
    auto action = ui_->variableBtn->menu()->addAction(name);
    QObject::connect(action, &QAction::triggered, [=](){ slotVar(name); });
  }
}
void ExprEditor::removeVariable(const Variable& variable)
{
  if(variable.has_name()) {
    QString name = QString::fromStdString(variable.name());
    vars_.remove(name);
    auto actions = ui_->variableBtn->menu()->actions();
    for(auto action :ui_->variableBtn->menu()->actions()) {
      if(action->text() == name) {
        ui_->variableBtn->menu()->removeAction(action);
        break;
      }
    }
  }
}
void ExprEditor::slotSetVars(const QMap<QString, Variable>& vars)
{
  for(auto it = vars.begin(); vars.end() != it; ++it)
    if(vars_.end() == vars_.find(it.key()))
      addNewVariable(it.value());

  QList<conf::ThematicVariable> removeList;
  for(auto it = vars_.begin(); vars_.end() != it; ++it)
    if(vars.end() == vars.find(it.key()))
      removeList.append(it.value());
  for(auto it : removeList)
    removeVariable(it);

  vars_ = vars;
}
void ExprEditor::slotVar(const QString &variable)
{
  ui_->exprEdt->insert(variable);
  refreshExpr();
}

void ExprEditor::slotSin()
{
  QString text = ui_->exprEdt->text();
  if(selectedText_.isEmpty()) {
    ui_->exprEdt->insert("sin()");
    ui_->exprEdt->setCursorPosition(ui_->exprEdt->cursorPosition() - 1);
  }
  else {
    selectedText_.insert(0, "sin(").append(')');
    text.replace(startSelectedText_, selectedText_.size() - 5, selectedText_);
    ui_->exprEdt->setText(text);
    selectedText_ = "";
    startSelectedText_ = ui_->exprEdt->text().size();
  }
  refreshExpr();
}
void ExprEditor::slotCos()
{
  QString text = ui_->exprEdt->text();
  if(selectedText_.isEmpty()) {
    ui_->exprEdt->insert("cos()");
    ui_->exprEdt->setCursorPosition(ui_->exprEdt->cursorPosition() - 1);
  }
  else {
    selectedText_.insert(0, "cos(").append(')');
    text.replace(startSelectedText_, selectedText_.size() - 5, selectedText_);
    ui_->exprEdt->setText(text);
    selectedText_ = "";
    startSelectedText_ = ui_->exprEdt->text().size();
  }
  refreshExpr();
}
void ExprEditor::slotTan()
{
  QString text = ui_->exprEdt->text();
  if(selectedText_.isEmpty()) {
    ui_->exprEdt->insert("tan()");
    ui_->exprEdt->setCursorPosition(ui_->exprEdt->cursorPosition() - 1);
  }
  else {
    selectedText_.insert(0, "tan(").append(')');
    text.replace(startSelectedText_, selectedText_.size() - 5, selectedText_);
    ui_->exprEdt->setText(text);
    selectedText_ = "";
    startSelectedText_ = ui_->exprEdt->text().size();
  }
  refreshExpr();
}
void ExprEditor::slotCtg()
{
  QString text = ui_->exprEdt->text();
  if(selectedText_.isEmpty()) {
    ui_->exprEdt->insert("ctg()");
    ui_->exprEdt->setCursorPosition(ui_->exprEdt->cursorPosition() - 1);
  }
  else {
    selectedText_.insert(0, "ctg(").append(')');
    text.replace(startSelectedText_, selectedText_.size() - 5, selectedText_);
    ui_->exprEdt->setText(text);
    selectedText_ = "";
    startSelectedText_ = ui_->exprEdt->text().size();
  }
  refreshExpr();
}

void ExprEditor::slotComma()
{
  QString text = ui_->exprEdt->text();
  if(ui_->exprEdt->cursorPosition() > 0 &&
     text.at(ui_->exprEdt->cursorPosition() - 1).isDigit())
    ui_->exprEdt->insert(".");
  else
    ui_->exprEdt->insert("0.");
  refreshExpr();
}
void ExprEditor::slotAbs()
{
  QString text = ui_->exprEdt->text();
  if(selectedText_.isEmpty()) {
    int countBracket = std::count(text.begin(), text.end(), '|');
    if(countBracket % 2) {
      ui_->exprEdt->insert("|");
    }
    else {
      ui_->exprEdt->insert("||");
      ui_->exprEdt->setCursorPosition(ui_->exprEdt->cursorPosition() - 1);
    }
  }
  else {
    if('|' == selectedText_.at(0) && '|' == selectedText_.at(selectedText_.size() - 1)) {
      selectedText_ = selectedText_.mid(1, selectedText_.size() - 2);
      text.replace(startSelectedText_, selectedText_.size() + 2, selectedText_);
    }
    else {
      selectedText_.insert(0, '|').append('|');
      text.replace(startSelectedText_, selectedText_.size() - 2, selectedText_);
    }
    ui_->exprEdt->setText(text);
    selectedText_ = "";
    startSelectedText_ = ui_->exprEdt->text().size();
  }
  refreshExpr();
}
void ExprEditor::slotLeftBracket()
{
  QString text = ui_->exprEdt->text();
  if(selectedText_.isEmpty()) {
    int countLeftBracket = std::count(text.begin(), text.end(), '('),
        countRightBracket = std::count(text.begin(), text.end(), ')');
    if(countLeftBracket == countRightBracket) {
      ui_->exprEdt->insert("()");
      ui_->exprEdt->setCursorPosition(ui_->exprEdt->cursorPosition() - 1);
    }
    else {
      ui_->exprEdt->insert("(");
    }
  }
  else {
    if('(' == selectedText_.at(0) && ')' == selectedText_.at(selectedText_.size() - 1)) {
      selectedText_ = selectedText_.mid(1, selectedText_.size() - 2);
      text.replace(startSelectedText_, selectedText_.size() + 2, selectedText_);
    }
    else {
      selectedText_.insert(0, '(').append(')');
      text.replace(startSelectedText_, selectedText_.size() - 2, selectedText_);
    }
    ui_->exprEdt->setText(text);
    selectedText_ = "";
    startSelectedText_ = ui_->exprEdt->text().size();
  }
  refreshExpr();
}

void ExprEditor::slotSelectionText()
{
  if(this->focusWidget() != ui_->exprEdt) return;
  selectedText_ = ui_->exprEdt->selectedText();
  startSelectedText_ = ui_->exprEdt->selectionStart();
}

void ExprEditor::refreshExpr()
{
  ui_->exprEdt->setFocus();
  exprVis_->setExpr(ui_->exprEdt->text());
}

} //mappi
