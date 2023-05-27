#include "forecastformwidget.h"
#include "ui_forecastformwidget.h"



ForecastFormWidget::ForecastFormWidget(QWidget *parent, QString path)
  : QWidget(parent),
    ui_(new Ui::ffWidget), paramfilepath_(path)
{
  ui_->setupUi(this);

  // инициализируем виджеты
  this->InitForm();
}

ForecastFormWidget::~ForecastFormWidget()
{

  delete resulttext_;
  delete ui_;
}

/**
 * Инициализация формы, загрузка кнопок и табов из протофайла
 *
 * @kForecastWidgetFileName - путь к файлу с настройками
 * 
 */
void ForecastFormWidget::InitForm()
{
  meteo::ui::proto::ForecastWidget protmsg;
  if ( false == TProtoText::fromFile( this->paramfilepath_, &protmsg ) ) {
    error_log << QObject::tr("Не удалось прочитать из файла %1 набор свойств виджета для составления прогнозов.")
      .arg(meteo::global::kForecastWidgetFileName);
    return;
  }

  // инициализируем переменную с текстом
  resulttext_ = new QString();

  // навешиваем на кнопку копирования в буфер  сохранение в буфер
  connect( ui_->copyButton, SIGNAL(clicked()), this, SLOT(clipboardCopy()) );

  // навешиваем на кнопку копирования в буфер  сохранение в буфер
  connect( ui_->submitButton, SIGNAL(clicked()), this, SLOT(submitButtonClicked()) );

  // основной виджет с табами
  // задаем название объекта
  ui_->mainTabWidget->setObjectName("mainTabWidget");

  // считываем файл с настройками
  for ( int i = 0, tabcount = protmsg.tabs_size(); i < tabcount; ++i  ) {
    // 
    // считываем табы
    // 
    meteo::ui::proto::TabType tabone = protmsg.tabs(i);
 
    // Горизотальный лайоут для таба
    QHBoxLayout *currentTabLayout = new QHBoxLayout(this);

    // 
    // в табе считываем секции
    // 
    for ( int si = 0, sectioncount = tabone.section_size(); si < sectioncount; ++si ) {
      // считываем секцию
      meteo::ui::proto::SectionType sectionone = tabone.section(si);

      // гроупбокс с кнопками
      QGroupBox *currentGBoxSection = new QGroupBox( this);
      currentGBoxSection->setTitle(QString::fromStdString(sectionone.name()));
      // текущая секция с кнопками
      QVBoxLayout *currentSectionLayout = new QVBoxLayout( this);
      currentSectionLayout->setSpacing(0);

      // 
      // в секции считываем кнопки
      //       
      for ( int bi = 0, buttoncount = sectionone.button_size(); bi < buttoncount; ++bi ) {
        // считываем секцию
        meteo::ui::proto::ButtonType buttonone = sectionone.button(bi);

        // текущая кнопка 
        ForecastWidgetButton *currentButton = new ForecastWidgetButton( this);

        // задаем цвет кнопке, соответственно данным из таба
        if ( tabone.has_rgb() ){
          currentButton->setColor( tabone.rgb().r(),tabone.rgb().g(),tabone.rgb().b() );  
        }
        
        currentButton->setStyleSheet("text-align:left;padding:2px;");
        currentButton->setText( QString::fromStdString( buttonone.name()) );
        currentButton->setMinimumHeight(25);
        currentButton->setMaximumHeight(27);
        currentButton->setMinimumWidth(120);
        currentButton->setName( QString::fromStdString( buttonone.name() ) );
        currentButton->setType( QString::fromStdString( buttonone.type() ) );
        currentButton->setCategory( QString::fromStdString( buttonone.category() ) );
        currentButton->setBcomma( buttonone.bcomma() );
        currentButton->setComma( buttonone.comma() );
        currentButton->setCatcomma( buttonone.catcomma() );
        currentButton->setNotfirst( buttonone.notfirst() );
        // вешаем обработчик нажатия на кнопку 
        connect( currentButton, SIGNAL(clickedWithParam(QString, QString, QString, bool, bool, bool, bool)), SLOT(buttonForecastClicked( QString, QString, QString, bool, bool, bool, bool)) );

        // добавляем текущую кнопку
        currentSectionLayout->addWidget(currentButton);
        // если есть пробел после кнопки - то ставим его
        if ( buttonone.has_afterspace() && buttonone.afterspace()==true )
        {
          currentSectionLayout->addSpacing(10);
        }
      }//endfor

      // 
      // в секции считываем поля ввода
      //       
      for ( int bi = 0, inputcount = sectionone.input_size(); bi < inputcount; ++bi ) {
        // считываем секцию
        meteo::ui::proto::InputType inputone = sectionone.input(bi);

        // текущая кнопка 
        ForecastWidgetInput *currentInput = new ForecastWidgetInput( this);
        currentInput->setName( QString::fromStdString( inputone.name() ) );
        currentInput->setGradation( inputone.gradation() );

        connect( currentInput, SIGNAL(inputChange()), SLOT(childChanged()) );

        // 
        // добавляем ввод температуры в список
        // 
        temperatureInputs_.append( currentInput );

        // добавляем текущую кнопку
        currentSectionLayout->addWidget(currentInput);
      }

      // добавляем спэйсеры
      // QSpacerItem *verticalSpacer = new QSpacerItem(20,40);
      // currentSectionLayout->addWidget(verticalSpacer);
      currentSectionLayout->addStretch(10);


      currentGBoxSection->setLayout(currentSectionLayout);
      // добавляем секции к табам
      currentTabLayout->addWidget(currentGBoxSection);

    }//endfor


    /**
     *
     *  Добавляем последнее поле с набранными прогнозами и 
     *  текстовым полем с готовым текстом
     *  
     * 
     */
    // 
    // если в прото файле указано, что последнюю секцию показывать не надо - то не доабвляем ее
    // 
    if ( tabone.last_section()!=false  ){
      // добавляем последний результирующий бокс и кнопку 
      ForecastWidgetResultBox *forecastResultBox = new ForecastWidgetResultBox(this);
      // задаем имя для бокса с результатами
      forecastResultBox->setName(QString::fromStdString(tabone.name()));
      // навишиваем коннект на изменение текста в результирующем боксе
      connect( forecastResultBox, SIGNAL(boxChanged()), this, SLOT(childChanged()) );

      // текущая секция с кнопками
      QVBoxLayout *currentResultLayout = new QVBoxLayout( this);
      currentResultLayout->setSpacing(0);
      currentResultLayout->addWidget(forecastResultBox);
      // нажатие на кнопку
      QPushButton *lastSectionButton = new QPushButton(this);
      lastSectionButton->setMaximumWidth(150);
      // если это поледний таб
      if ( protmsg.tabs_size() == (i+1) ){
        lastSectionButton->setText(lastButtonTextFinish_);
      }else{
        lastSectionButton->setText(lastButtonText_);
      }
      if ( meteo::global::kForecastWidgetFileNameOrnitolog == paramfilepath_ ) {
        lastSectionButton->hide();
      }

      // добавляем линию разделения    

      currentResultLayout->addWidget(lastSectionButton);
      currentResultLayout->addSpacerItem( new QSpacerItem(20,40) );
      // обабатываем как 
      connect( lastSectionButton, SIGNAL(clicked(bool)), SLOT(buttonLastSectionClicked()) );


      // добавляем в основной лайоут результирующую секцию
      currentTabLayout->addLayout(currentResultLayout);

      /**
       * ========== заканчиваем добавлять последнюю секцию с кнопкой ========
       */
    }

    QWidget *tab = new QWidget( this);
    tab->setLayout(currentTabLayout);
    // добавляем созданную секцию в основной виджет
    ui_->mainTabWidget->addTab( tab, QString::fromStdString(tabone.name()) );   
  }//endfor

  
  
  // сохраняем список результирующих боксов
  // после инициализации
  for ( int i=0; i<ui_->mainTabWidget->count();i++ ){ 
    QList<ForecastWidgetResultBox *> wOnPage = ui_->mainTabWidget->widget(i)->findChildren<ForecastWidgetResultBox *>();
    if ( wOnPage.count()>0 ){
      resBoxes_.append( wOnPage[0] );    
    }    
  }
  
  return;
}


/**
 * компилируем текст в один
 */
void ForecastFormWidget::compileText()
{
  resulttext_->clear();
  for ( auto resBox : resBoxes_)
  {
    resulttext_->append( resBox->getText() );
    resulttext_->append(" ");
  }
  
  for ( auto tempInput : temperatureInputs_)
  {
    resulttext_->append( tempInput->getText() );
    resulttext_->append(" ");
  }

  return;
}


/**
 * получаем текст
 * @return [description]
 */
QString ForecastFormWidget::getText()
{
  return ui_->resultTextEdit->toPlainText();
}



/**
 *
 * ==========   S L O T S   ===========
 *
 * 
 */


/**
 * нажатие на кнопку сабмита текста из окна
 * по нажатию испускаем сигнал с текстом и 
 * убиваем форму
 */
void ForecastFormWidget::submitButtonClicked()
{
  // испускаем сигнал с получившимся текстом
  emit submitFormText( this->getText() );
}


/**
 * копирование текста в буфер обмена
 */
void ForecastFormWidget::clipboardCopy()
{
  QClipboard *clipboard = QGuiApplication::clipboard();
  clipboard->setText( this->getText() );
  return;
}

/**
 * слот с изменениями от детей
 */
void ForecastFormWidget::childChanged()
{
  this->compileText();
  ui_->resultTextEdit->setPlainText( *resulttext_ );
  return;
}

/**
 * обрабатываем нажатие на кнопку с прогнозами
 *  
 */
void ForecastFormWidget::buttonForecastClicked( QString name, QString type, QString cat, bool  bcomma, bool comma, bool catcomma, bool notfirst )
{
  // 
  // находим текущий таб и в нем находим результирующее окно
  // 
  
  if ( resBoxes_.size()>0 ){
    ForecastWidgetResultBox *resBox = resBoxes_.at(ui_->mainTabWidget->currentIndex());
    resBox->addWithParam( name, type, cat, bcomma, comma, catcomma, notfirst );
  }
  return;
}


/**
 * обрабатываем нажатие на последнюю кнопку
 * смотрим, если мы не в последнем табе, тогда текст кнопки "далее", если на последнем, то "сохранить"
 * и на последнем табе сохраняем прогноз
 * 
 */
void ForecastFormWidget::buttonLastSectionClicked()
{
  //
  // Переключаем таб, если есть следующие вкладки
  //
  int curindex = ui_->mainTabWidget->currentIndex();

  if ( ui_->mainTabWidget->count() == curindex+1 ){

  }else{
    ui_->mainTabWidget->setCurrentIndex(curindex+1);  
  }
  return;
}
