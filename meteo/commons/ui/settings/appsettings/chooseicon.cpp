#include "chooseicon.h"
#include "ui_chooseicon.h"

#include <QDir>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QStringList>

#include <meteo/commons/ui/custom/filedialogrus.h>

namespace{

QStringList recursiveFilesList(const QString& dirpath)
{
  QStringList result;
  foreach (const QFileInfo& fi, QDir(dirpath).entryInfoList()) {
    if (fi.isDir() == true) {
      result += recursiveFilesList(fi.absoluteFilePath());
    }
    else if (fi.isFile() == true){
      result.append(fi.absoluteFilePath());
    }
  }
  return result;
}

QString iconsPath() { return ":/meteo/icons/apps"; }

}

namespace meteo {

ChooseIconDialog::ChooseIconDialog(QWidget* parent) :
  QDialog(parent),
  ui_(new Ui::ChooseIcon()),
  currentIco_(defaultIcon())
{
  ui_->setupUi(this);

  loadDefaultIcons();
  previewDefaultIcons();
  previewCurrent();

  connect(ui_->defaultRadioButton, SIGNAL(clicked()), SLOT(slotSelectPage()));
  connect(ui_->manualRadioButton, SIGNAL(clicked()), SLOT(slotSelectPage()));
  connect(ui_->openButton, SIGNAL(clicked()), SLOT(slotOpenFileDialog()));
  connect(ui_->pathLineEdit, SIGNAL(editingFinished()), SLOT(slotLoadIcon()));
  connect(ui_->applyButton, SIGNAL(clicked()), SLOT(accept()));
  connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(reject()));
}

ChooseIconDialog::~ChooseIconDialog()
{
  delete ui_;
  ui_ = 0;
}

void ChooseIconDialog::init(const QString& icopath)
{
  currentIco_ = icopath;
  previewCurrent();
}

QString ChooseIconDialog::iconPath() const
{
  return currentIco_;
}

QString ChooseIconDialog::defaultIcon()
{
  return ":/meteo/icons/process_none.png";
}

void ChooseIconDialog::previewDefaultIcons()
{
  int row = 0,
      col = 0;
  const int colcount = ui_->pageDefault->width() / ui_->previewLabel->width();

  typedef QHash<QLabel*, QString>::const_iterator Iter;
  for (Iter it = defaultIcons_.constBegin(), end = defaultIcons_.constEnd(); it != end; ++it) {
    if (col >= colcount) {
      col = 0;
      ++row;
    }
    static_cast<QGridLayout*>(ui_->pageDefault->layout())->addWidget(it.key(), row, col++);
  }
}

void ChooseIconDialog::loadDefaultIcons()
{
  foreach (const QString& icopath, ::recursiveFilesList(::iconsPath())) {
    QPixmap px = QPixmap(icopath).scaledToWidth(ui_->previewLabel->width());
    QLabel* lb = new QLabel(this);
    lb->installEventFilter(this);
    lb->setPixmap(px);
    lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    defaultIcons_.insert(lb, icopath);
  }
}

void ChooseIconDialog::slotSelectPage()
{
  if (sender() == ui_->defaultRadioButton) {
    ui_->stackedWidget->setCurrentWidget(ui_->pageDefault);
  }
  else if (sender() == ui_->manualRadioButton) {
    ui_->stackedWidget->setCurrentWidget(ui_->pageManual);
  }
}

void ChooseIconDialog::slotOpenFileDialog()
{
  ui_->pathLineEdit->setText(meteo::FileDialog::getOpenFileName(this, QString::fromUtf8("Выбрать файл..."),
                                                          QDir::homePath(), QString::fromUtf8("Файлы изображений (*.png *.bmp *.jpg *.jpeg)")));
  slotLoadIcon();
}

void ChooseIconDialog::previewCurrent()
{
  if (currentIco_.isEmpty() == true ||
      QFile::exists(currentIco_) == false) {
    currentIco_ = defaultIcon();
  }

  QPixmap px(currentIco_);
  if (px.isNull() == true) {
    QMessageBox::warning(this, QString::fromUtf8("Ошибка"),
                         QString::fromUtf8("Не удалось загрузить изображение из файла: %1").arg(currentIco_),
                         QMessageBox::Ok);
    currentIco_ = defaultIcon();
    px.load(currentIco_);
  }
  ui_->previewLabel->setPixmap(px.scaledToWidth(ui_->previewLabel->width()));
}

void ChooseIconDialog::slotLoadIcon()
{
  currentIco_ = ui_->pathLineEdit->text();
  previewCurrent();
}

bool ChooseIconDialog::eventFilter(QObject* obj, QEvent* ev)
{
  QLabel* lb = qobject_cast<QLabel*>(obj);
  if (obj != 0 && ev->type() == QEvent::MouseButtonRelease) {
    QMouseEvent* me = static_cast<QMouseEvent*>(ev);
    if (me->button() == Qt::LeftButton) {
      if (defaultIcons_.contains(lb) == true) {
        currentIco_ = defaultIcons_[lb];
        previewCurrent();
      }
    }
  }
  return QDialog::eventFilter(obj, ev);
}

} // meteo
