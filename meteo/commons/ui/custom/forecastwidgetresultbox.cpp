#include "forecastwidgetresultbox.h"


ForecastWidgetResultBox::ForecastWidgetResultBox( QWidget* p )
  : QScrollArea(p)
{
  this->Init();
}

/**
 * функция инициализации
 */
void ForecastWidgetResultBox::Init()
{

  this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setWidgetResizable(true);

  this->setMinimumWidth(250);
  this->setMaximumWidth(250);

  resulttext_ = new QString();

  items_ = new QList<ForecastWidgetItem *>;

  // добавляем лайоут
  layout_ = new QVBoxLayout(this);
  layout_->addSpacerItem( new QSpacerItem(1,1, QSizePolicy::Minimum, QSizePolicy::Expanding) );
  layout_->setSpacing(1);

  QWidget *widget_ = new QWidget(this);
  widget_->setLayout(layout_);

  widget_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred );

  widget_->setMaximumWidth(2000);
  this->setWidget(widget_);  
  widget_->show();
}


ForecastWidgetResultBox::~ForecastWidgetResultBox()
{
  delete layout_; 
  delete items_;
  delete resulttext_;
}

/**
 * Функция
 * @param item [description]
 */
void ForecastWidgetResultBox::add( ForecastWidgetItem* item )
{
  layout_->insertWidget(layout_->count()-1,item);
  items_->append(item);
  // пересчитваем текст
  this->compileText();
  return;
}

/**
 * Функция
 * @param item [description]
 */
void ForecastWidgetResultBox::addWithParam(  QString name, QString type, QString cat, bool  bcomma, bool comma, bool catcomma, bool notfirst  )
{
  // error_log << id << name << " " << type << " " << cat << " " << bcomma << " " << comma << " " << notfirst << " " << catcomma;
  // error_log << this;
  ForecastWidgetItem *item = new ForecastWidgetItem(this);
  item->setName( name );
  item->setType( type );
  item->setCategory( cat );
  item->setBcomma( bcomma );
  item->setComma( comma );
  item->setCatcomma( catcomma );
  item->setNotfirst( notfirst );

  this->add(item);
  
  // подписываемся на изменения (создание и удаление) 
  connect( item, SIGNAL(updated()), this, SLOT(itemChange()) );
  // при удалении айтема - удаляем его из списка 
  connect( item, SIGNAL(deleteItem(ForecastWidgetItem*)), this, SLOT(itemDelete(ForecastWidgetItem*)) );
  return;
}

/**
 * Функция
 * @param item [description]
 */
int ForecastWidgetResultBox::count( )
{
  return items_->count();
}


/**
 * Функция
 * @param item [description]
 */
void ForecastWidgetResultBox::insert( ForecastWidgetItem* item, int index )
{
  items_->insert(index, item);
  layout_->addWidget(item);
  return;
}


/**
 * Функция
 * @param item [description]
 */
void ForecastWidgetResultBox::remove(int index)
{
  items_->removeAt(index);
  return;
}


/**
 * Функция
 * @param item [description]
 */
ForecastWidgetItem* ForecastWidgetResultBox::get(int index)
{
  return items_->at(index);
}


/**
 * устанавливаем имя для виджета, чтобы использовать в качестве первого текста
 * @param name [description]
 */
void ForecastWidgetResultBox::setName(QString name)
{
  name_=name;
  return;
}

/**
 * [ForecastWidgetResultBox::compileText description]
 */
void ForecastWidgetResultBox::compileText()
{
  // результирующая строка
  QString str;
  /**
   * проходимся по списку и компилируем
   * в текст
   *
   */
  // пердыдущий айтем
  QString beforeitemtext;
  // текст, который указывается в начале
  QString firsttext;
  // тип категории
  QString cattype;
  // есть ли первое слово
  bool notfirst;
  notfirst=false;

  // первая итерация цикла
  bool first;
  first = false;

  beforeitemtext="";

  // устаналиваем первое слово, если надо
  firsttext = name_;

  for ( auto item : *items_)
  {
    QString itemstr;

    if ( first==false ){
      notfirst = item->getNotfirst();
      first=true;
    }
    itemstr  = item->getName();

    //если надо ставить запятую
    if ( item->getComma()==true )
    {
      itemstr.replace(",","");
      itemstr.append(",");
    }

    if ( 
          ( item->getBcomma()==true && beforeitemtext!="" )
          ||
          ( item->getCatcomma()==true && cattype==item->getCategory() )
       )
    {
      if ( item->getBcomma()==true && beforeitemtext!="" && item->getCatcomma()==false && cattype==item->getCategory() )
      {

      } else{
        if ( beforeitemtext!="" )
        {
          beforeitemtext.replace(",","");
          beforeitemtext.append(",");
        }
      }
    }//endif

    str.append(" ");
    str.append( beforeitemtext.trimmed() );

    cattype  = item->getCategory(); //сохраняем тип предыдущего объекта
    beforeitemtext = itemstr; //сохраняем предыдущий объект    
  }

  str.append(" ");
  str.append( beforeitemtext.trimmed() );

  // error_log<< firsttext << " - " << notfirst;

  if ( notfirst==true )
  {
    firsttext="";
  }
  str.prepend(firsttext);
  *resulttext_ = str.trimmed(); 

  // испускаем сигнал о том, что мы изменили поле
  emit boxChanged();
  return;
}



/**
 * получаем скомпилированный текст
 */
QString ForecastWidgetResultBox::getText()
{
  return *resulttext_;
}



/**
 *
 *    S L O T S
 *
 * 
 */


/**
 * изменения в списке айтемов
 * надо испустить сигнал о пересчете всего текста
 текст=false;
 */
void ForecastWidgetResultBox::itemChange()
{


  emit itemsChanged();
}

/**
 * удаляем айтем из списков
 * и запускаем пересчет
 */
void ForecastWidgetResultBox::itemDelete(ForecastWidgetItem* item)
{
  int index = items_->indexOf(item);
  
  if ( index >=0 ){
    items_->removeAt(index);
 
    // испускаем сигнал о том, что мы изменили поле
    emit itemsChanged();
    // пересчитваем текст
    this->compileText();
  }
  
}

