#include "thematiccalc.h"

#include <functional>

#include <qfile.h>
#include <qstack.h>
#include <cross-commons/debug/tlog.h>

static const float kErrConst = -9999.0;

namespace mappi {
  namespace thematic {

// static
    void ThematicCalc::bindArray(const QString &name, to::Channel &vec) {
      if(vec.size() > size_) size_ = vec.size();
      map_.insert(name, vec);
    }

    bool ThematicCalc::parsingExpression(const QString &data) {
      stringExpression_ = data;
      return true;
    }

    QVector <uchar> ThematicCalc::dataProcessing() {
      for(const auto channel : map_.values()){
        if(!channel.hasData()){
          min_value_ = kErrConst;
          max_value_ = kErrConst;
          result_ = QVector<float>(map_.value(stringExpression_).size(), kErrConst);
          size_ = result_.size();
          debug_log << QObject::tr("Нет данных в канале %1 размером %2")
                                .arg(QString::fromStdString(channel.name())).arg(size_);
          return QVector<uchar>(size_, 0);
        }
      }

      //Если математика не нужна
      if(map_.contains(stringExpression_)){
        min_value_ = map_.value(stringExpression_).min();
        max_value_ = map_.value(stringExpression_).max();
        result_ = QVector<float>(map_.value(stringExpression_));
        size_ = result_.size();
        debug_log << QObject::tr("Возвращаем массив размером %1 для %2").arg(size_).arg(stringExpression_);
        return fitTo8Bit();
      }

      if (size_ == 0) {
        result_.push_back(0);
        return QVector<uchar>(1, 0);
      }
      debug_log << QObject::tr("Размер выходных векторов: %1").arg(size_);

      processVector();
      if(result_.size() != size_){
        error_log << QObject::tr("Неверно посчитан вектор: %1 != %2").arg(result_.size()).arg(size_);
        return QVector<uchar>(1, 0);
      }

      //check for kErrConst values in channels
      for (const auto channel: map_.values()) {
        for (const int no_data_idx: channel.noData()) {
          result_[no_data_idx] = kErrConst;
        }
      }

      bool hasData = false;
      for (const float val: result_) {
        if (val == kErrConst) continue;
        hasData = true;
        max_value_ = std::max(val, max_value_);
        min_value_ = std::min(val, min_value_);
      }
      if(!hasData) max_value_ = min_value_;
      if(max_value_ < min_value_) std::swap(max_value_,min_value_);
      return fitTo8Bit();
    }

    void ThematicCalc::processVector() {
      exprtk::symbol_table<float> symbol_table;
      exprtk::expression<float> expression;
      exprtk::parser<float> parser;

      for(const auto key : map_.keys()){
        if(!symbol_table.add_vector(key.toStdString(), map_[key].data(), size_)){
          error_log << QObject::tr("Невозможно добавить вектор: %1").arg(key);
          result_ = QVector<float>(1, kErrConst);
          return;
        };
      }
      result_.resize(size_);
      symbol_table.add_vector("res", result_.data(), size_);
      expression.register_symbol_table(symbol_table);

      const std::string expression_string = "res := " + stringExpression_.toStdString();
      if (!parser.compile(expression_string, expression)) {
        error_log <<  QObject::tr("[load_expression] - Parser Error: %1\tExpression: %2\n")
                        .arg(QString::fromStdString(parser.error()))
                        .arg(QString::fromStdString(expression_string));
        result_ = QVector<float>(1, kErrConst);
        return;
      }

      debug_log << QObject::tr("Математическая функция успешно подготовлена: '%1'").arg(stringExpression_);
      expression.value();
    }

    QVector<uchar> ThematicCalc::fitTo8Bit(){
      debug_log << QObject::tr("Копируем данные в массив color. Мин-Макс: %1,%2").arg(min_value_, 0, 'f', 2).arg(max_value_, 0, 'f', 2);
      if(min_value_ == max_value_) return QVector<uchar>(size_, 0); //Длина диапазона 0, нечего возвращать
      QVector<uchar> color(size_, 0);
      for (int i = 0; i < size_; i++){
        if (result_.at(i) <= 0) continue;
        color[i] = static_cast<uint16_t>(result_.at(i)) >> 8;
      }
      return color;
    }

    QVector<uchar> ThematicCalc::stretchTo8Bit(){
      debug_log << QObject::tr("Растягиваем гистограмму данные в массив color. Мин-Макс: %1,%2").arg(min_value_, 0, 'f', 2).arg(max_value_, 0, 'f', 2);
      if(min_value_ == max_value_) return QVector<uchar>(size_, 0); //Длина диапазона 0, нечего возвращать
      QVector<uchar> color(size_, 0);
      for (int i = 0; i < size_; i++){
        if (result_.at(i) == kErrConst) continue;
        color[i] = normalize<uchar, float>(result_.at(i), min_value_, max_value_);
      }
      return color;
    }

    template<typename T, typename U>
    T ThematicCalc::normalize(U value){
      return normalize<T, U>(value, std::numeric_limits<U>::min(), std::numeric_limits<U>::max());
    }

    template<typename T, typename U>
    T ThematicCalc::normalize(U value, U old_min, U old_max){
      if(old_max == old_min) return static_cast<T>(0);
      constexpr T new_min = std::numeric_limits<T>::min();
      constexpr T new_max = std::numeric_limits<T>::max();
      double normFactor = static_cast<double>(new_max - new_min) / (old_max - old_min);
      return static_cast<T>(normFactor * (value - old_min) + new_min);
    }
  } //commons
} //mappi
