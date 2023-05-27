#include "appstarteditor.h"
#include "ui_appstarteditor.h"

#include "appstartargsview.h"
#include "argedit.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/ui/custom/filedialogrus.h>

#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTreeWidgetItem>

namespace meteo {

AppStartEditorWidget::AppStartEditorWidget(const QStringList& usedIds, QWidget* parent) :
  QDialog(parent),
  ui_(new Ui::AppStartEditor()),
  usedIds_(usedIds)
{
  ui_->setupUi(this);

  ui_->errorLabel->hide();

  connect(ui_->applyButton, SIGNAL(clicked()), SLOT(accept()));
  connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(ui_->selectFileButton, SIGNAL(clicked()), SLOT(slotOpenDialog()));
  connect(ui_->viewArgsButton, SIGNAL(clicked()), SLOT(slotViewArgs()));
  connect(ui_->pathLineEdit, SIGNAL(editingFinished()), SLOT(slotValidateConfig()));
  connect(ui_->idLineEdit, SIGNAL(editingFinished()), SLOT(slotValidateConfig()));

  connect(ui_->addButton, SIGNAL(clicked()), SLOT(slotAddArg()));
  connect(ui_->removeButton, SIGNAL(clicked()), SLOT(slotRemoveArg()));
  connect(ui_->editButton, SIGNAL(clicked()), SLOT(slotEditArg()));

  connect(ui_->argsTreeWidget, SIGNAL(itemSelectionChanged()), SLOT(slotEnableButtons()));

  ui_->argsTreeWidget->setStyleSheet("QTreeView::item { height: 36px;}");

  ui_->argsTreeWidget->installEventFilter(this);
  ui_->argsTreeWidget->viewport()->installEventFilter(this);
}

AppStartEditorWidget::~AppStartEditorWidget()
{
  delete ui_;
  ui_ = nullptr;
}

void AppStartEditorWidget::init(const app::Application* const config)
{
  if (nullptr != config) {
    ui_->appGroupBox->setTitle(QString::fromStdString(config->title()));
    ui_->pathLineEdit->setText(QString::fromStdString(config->path()));
    ui_->idLineEdit->setText(QString::fromStdString(config->id()));

    slotValidateConfig();

    QStringList args;
    for (int i = 0, sz = config->arg_size(); i < sz; ++i) {
      args.append(QString::fromStdString(config->arg(i)));
    }
    addArguments(args);
    ui_->argsTreeWidget->resizeColumnToContents(KEY);
    ui_->argsTreeWidget->resizeColumnToContents(VALUE);
  }
  else {
    ui_->appGroupBox->setTitle(QString::fromUtf8("Новое приложение"));
    ui_->pathLineEdit->clear();
    ui_->idLineEdit->clear();
    ui_->argsTreeWidget->clear();
    ui_->errorLabel->clear();
    ui_->errorLabel->hide();
  }
}

void AppStartEditorWidget::addArguments(const QStringList& args)
{
  for (int i = 0, sz = args.size(); i < sz; ++i) {
    if (args[i].isEmpty() == false) {
      QTreeWidgetItem* item = new QTreeWidgetItem(ui_->argsTreeWidget);
      QChar sep;
      if (args[i].startsWith("--") == true) {
        sep = '=';
      }
      else if (args[i].startsWith('-') == true) {
        sep = ' ';
      }
      else {
        item->setText(VALUE, args[i]);
        continue;
      }

      if (args[i].contains(sep) == true) {
        int index = args[i].indexOf(sep);
        item->setText(KEY, args[i].left(index));
        item->setText(VALUE, args[i].mid(index+1));
      }
      else {
        item->setText(KEY, args[i]);
      }
    }
  }
}

app::Application AppStartEditorWidget::config() const
{
  app::Application appconf;
  appconf.set_title(ui_->appGroupBox->title().toStdString());
  appconf.set_path(ui_->pathLineEdit->text().toStdString());
  appconf.set_id(ui_->idLineEdit->text().toStdString());
  foreach (const QString& arg, arguments()) {
    appconf.add_arg(arg.toStdString());
  }
  return appconf;
}

QStringList AppStartEditorWidget::arguments() const
{
  QStringList result;
  for (int i = 0, sz = ui_->argsTreeWidget->topLevelItemCount(); i < sz; ++i) {
    QTreeWidgetItem* item = ui_->argsTreeWidget->topLevelItem(i);
    if (nullptr != item) {
      QString key = item->text(KEY).trimmed();
      QString value = item->text(VALUE).trimmed();
      if (key.isEmpty() == false &&
          value.isEmpty() == false) {
        if (key.startsWith("--") == true) {
          key += "=";
        }
        else if (key.startsWith('-') == true) {
          key += " ";
        }
        else {
          key = "--" + key + "=";
        }
      }
      if (value.isEmpty() == false) {
        if (value.contains(' ') == true) {
          value = "\"" + value + "\"";
        }
      }
      if (key.isEmpty() == false ||
          value.isEmpty() == false) {
        result.append(key + value);
      }
    }
  }
  return result;
}

void AppStartEditorWidget::slotOpenDialog()
{
  meteo::FileDialog dialog(this, QString::fromUtf8("Выбрать файл"), qApp->applicationDirPath());
  dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
  if ( QDialog::Accepted == dialog.exec() ) {
    auto fileNames = dialog.selectedFiles();
    if ( 1 == fileNames.size() ) {
      ui_->pathLineEdit->setText(fileNames.first());
      slotValidateConfig();
    }
  }
}

void AppStartEditorWidget::slotValidateConfig()
{
  if (validatePath() == true) {
    validateId();
  }
}

bool AppStartEditorWidget::validatePath()
{
  QString filepath = ui_->pathLineEdit->text();
  if (filepath.isEmpty() == true) {
    ui_->errorLabel->setText(QString("<font color=red>%1</font>")
                             .arg(QString::fromUtf8("Не указан путь к исполняемому файлу приложения")));
    ui_->errorLabel->show();
  }
  else if (QFile::exists(filepath) == false) {
    ui_->errorLabel->setText(QString("<font color=red>%1</font>")
                             .arg(QString::fromUtf8("Не найден исполняемый файл приложения")));
    ui_->errorLabel->show();
  }
  else {
    ui_->errorLabel->clear();
    ui_->errorLabel->hide();
    return true;
  }
  return false;
}

bool AppStartEditorWidget::validateId()
{
  bool uniqId = !(usedIds_.count(ui_->idLineEdit->text()) > 1);
  if (uniqId == false) {
    ui_->errorLabel->setText(QString("<font color=red>%1</font>")
                             .arg(QString::fromUtf8("Идентификатор приложения уже используется")));
    ui_->errorLabel->show();
  }
  else {
    ui_->errorLabel->clear();
    ui_->errorLabel->hide();
  }
  return uniqId;
}

void AppStartEditorWidget::slotAddArg()
{
  ArgEditDialog dlg;
  if (dlg.exec() == QDialog::Accepted) {
    QString key = dlg.argkey().trimmed();
    QString value = dlg.argvalue().trimmed();

    if (key.isEmpty() == true && value.isEmpty() == true) {
      return;
    }

    if(key.isEmpty() == false) {
      if (key.startsWith('-') == false &&
          key.startsWith("--") == false) {
        key = (key.length() > 1) ? "--"+key : "-"+key;
      }
    }

    QTreeWidgetItem* item = new QTreeWidgetItem(ui_->argsTreeWidget);
    item->setText(KEY, key);
    item->setText(VALUE, value);
    ui_->argsTreeWidget->resizeColumnToContents(KEY);
    ui_->argsTreeWidget->resizeColumnToContents(VALUE);
  }
}

void AppStartEditorWidget::slotRemoveArg()
{
  if (ui_->argsTreeWidget->selectionModel()->hasSelection() == true) {
    foreach (QTreeWidgetItem* item, ui_->argsTreeWidget->selectedItems()) {
      slotRemoveArg(item);
    }
  }
}

void AppStartEditorWidget::slotRemoveArg(QTreeWidgetItem* item)
{
  if ( nullptr != item &&
      QMessageBox::information(this, QString::fromUtf8("Подтверждение удаления"),
                                 QString::fromUtf8("Удалить параметр '%1'?").arg(item->text(KEY)),
                                 QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
    delete item;
  }
}

void AppStartEditorWidget::slotEditArg()
{
  if (ui_->argsTreeWidget->selectionModel()->hasSelection() == false) {
    return;
  }

  if (ui_->argsTreeWidget->selectedItems().size() > 1) {
    return;
  }

  QTreeWidgetItem* item = ui_->argsTreeWidget->selectedItems().first();
  if ( nullptr != item ) {
    slotEditArg(item);
  }
}

void AppStartEditorWidget::slotEditArg(QTreeWidgetItem* item)
{
  if (nullptr != item) {
    ArgEditDialog dlg;
    dlg.init(item->text(KEY), item->text(VALUE));
    if (dlg.exec() == QDialog::Accepted) {
      QString key = dlg.argkey().trimmed();
      QString value = dlg.argvalue().trimmed();

      if (key.isEmpty() == true && value.isEmpty() == true) {
        return;
      }

      if(key.isEmpty() == false) {
        if (key.startsWith('-') == false &&
            key.startsWith("--") == false) {
          key = (key.length() > 1) ? "--"+key : "-"+key;
        }
      }

      item->setText(KEY, key);
      item->setText(VALUE, value);
    }
  }
}

void AppStartEditorWidget::slotViewArgs()
{
  QString errorString;

  QString app = ui_->pathLineEdit->text();
  QProcess pr;
  pr.setProcessChannelMode(QProcess::MergedChannels);
  pr.start(app, QStringList() << "--help");
  if (pr.waitForStarted(500) == false) {
    errorString = QString::fromUtf8("Не удалось запустить процесс %1").arg(app);
  }
  else {
    if (pr.waitForFinished(500) == false) {
      errorString = QString::fromUtf8("Не удалось получить список параметров запуска от процесса %1").arg(app);
    }
    else {
      if (pr.state() == QProcess::Running) {
        pr.terminate();
      }
      QString output(pr.readAll());
      AppStartArgsViewWidget* aview = new AppStartArgsViewWidget(output, this);
      connect(aview, SIGNAL(finished(int)), aview, SLOT(deleteLater()));
      aview->show();
    }
  }

  if (errorString.isEmpty() == false) {
    QMessageBox::warning(this, QString::fromUtf8("Ошибка получения аргументов запуска"),
                         QString("%1\n%2").arg(errorString).arg(QString(pr.readAll())), QMessageBox::Ok);
  }
}

void AppStartEditorWidget::slotEnableButtons()
{
  bool enable = ui_->argsTreeWidget->selectionModel()->hasSelection();
  ui_->editButton->setEnabled(enable);
  ui_->removeButton->setEnabled(enable);
}

bool AppStartEditorWidget::eventFilter(QObject* object, QEvent* event)
{
  if (object == ui_->argsTreeWidget) {
    switch (event->type()) {
      case QEvent::KeyPress: {
          QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
          switch (keyEvent->key()) {
            case Qt::Key_Delete:
                slotRemoveArg();
              break;
            case Qt::Key_Return:
            case Qt::Key_Enter: {
                QTreeWidgetItem* item = ui_->argsTreeWidget->currentItem();
                if (nullptr != item) {
                  slotEditArg(item);
                }
              }
              break;
            default:
              break;
          }
        }
        break;
      case QEvent::ContextMenu: {
          QMenu menu;
          QAction add(QString::fromUtf8("Добавить"), &menu);
          QAction edit(QString::fromUtf8("Редактировать"), &menu);
          QAction remove(QString::fromUtf8("Удалить"), &menu);

          QList<QAction*> actions;
          actions.append(&add);
          const QPoint& pos = static_cast<QContextMenuEvent*>(event)->pos();
          QTreeWidgetItem* current = ui_->argsTreeWidget->itemAt(ui_->argsTreeWidget->viewport()->mapFromGlobal(ui_->argsTreeWidget->mapToGlobal(pos)));
          if (nullptr != current) {
            actions.append(&edit);
            actions.append(&remove);
          }

          QAction* answer = menu.exec(actions, ui_->argsTreeWidget->mapToGlobal(pos));
          if (answer == &add) {
            slotAddArg();
          }
          else if (answer == &edit) {
            slotEditArg(current);
          }
          else if (answer == &remove) {
            slotRemoveArg(current);
          }
        }
        break;
      default:
        break;
    }
  }
  else if (object == ui_->argsTreeWidget->viewport()) {
    switch (event->type()) {
      case QEvent::MouseButtonDblClick: {
          const QPoint& pos = static_cast<QMouseEvent*>(event)->pos();
          QTreeWidgetItem* current = ui_->argsTreeWidget->itemAt(pos);
          if (nullptr != current) {
            slotEditArg(current);
          }
          else {
            slotAddArg();
          }
        }
        break;
      default:
        break;
    }
  }
  return QDialog::eventFilter(object, event);
}

} // meteo
