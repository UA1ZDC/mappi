#include "thematiccalc.h"

#include <functional>

#include <qfile.h>
#include <qstack.h>
#include <cross-commons/debug/tlog.h>


/* stack frame for function x86
  func(int a, int b, int c) {
    int d, f, g;
  }
  c         <-    ebp + 16
  b         <-    ebp + 12
  a         <-    ebp + 8
  ret addr  <-    ebp + 4
  saved ebp <-    ebp
  d         <-    ebp - 4
  f         <-    ebp - 8
  g         <-    ebp - 12    <- esp

  func(int a, int b, int c) { }
  c         <-    ebp + 16
  b         <-    ebp + 12
  a         <-    ebp + 8
  ret addr  <-    ebp         <- esp
*/
/* stack frame x86_64 for *nix
  func(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    int k, l, m;
    func_(a, b, c);
  }
  a         <-    rdi
  b         <-    rsi
  c         <-    rdx
  d         <-    rcx
  e         <-    r8
  f         <-    r9
  g         <-    rbp + 40
  h         <-    rbp + 32
  i         <-    rbp + 24
  j         <-    rbp + 16
  ret addr  <-    rbp + 8
  saved rbp <-    rbp
  k         <-    rbp - 8
  l         <-    rbp - 16
  m         <-    rbp - 24    <- rsp
  { 128 byte red zone }

  func(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    int k, l, m;
  }
  a         <-    rdi
  b         <-    rsi
  c         <-    rdx
  d         <-    rcx
  e         <-    r8
  f         <-    r9
  g         <-    rbp + 40
  h         <-    rbp + 32
  i         <-    rbp + 24
  j         <-    rbp + 16
  ret addr  <-    rbp + 8
  saved rbp <-    rbp         <- rsp
  {
    k         <-    rbp - 8
    l         <-    rbp - 16
    m         <-    rbp - 24
  } <- 128 byte red zone }

  func(int a, int b, int c) {
    int k, l, m;
  }
  a         <-    rdi
  b         <-    rsi
  c         <-    rdx
  ret addr  <-    rbp + 8
  saved rbp <-    rbp         <- rsp
  {
    k         <-    rbp - 8
    l         <-    rbp - 16
    m         <-    rbp - 24
  } <- 128 byte red zone }

  func(int a, int b, int c) {
    int k, l, m;
  }
  a         <-    rdi
  b         <-    rsi
  c         <-    rdx
  ret addr  <-    rbp   <- rsp
  { 128 byte red zone }
*/

static const float kErrConst = -9999.0;

namespace mappi {
  namespace thematic {

// static
    ThematicCalc::MappingIndex ThematicCalc::NodeExpr::mapIndex_;

    ThematicCalc::ThematicCalc() : root_(nullptr) {}

    ThematicCalc::~ThematicCalc() { delete root_; }

    void ThematicCalc::bindArray(const QString &name, to::Channel &vec) { map_.insert(name, vec); }

    bool ThematicCalc::parsingFile(const QString &filename) {
      RPN rpn;
      stack_ = rpn.parsingFile(filename);
      return !stack_.isEmpty();
    }

    bool ThematicCalc::parsingExpression(const QString &data) {
      RPN rpn;
      stack_ = rpn.parsingExpression(data);
      expression_ = data;
      return !stack_.isEmpty();
    }

    ThematicCalc::NodeExpr *ThematicCalc::createTree(QList <Token> stack) {
      QStack < NodeExpr * > nodeStack;
      while (!stack.isEmpty()) {
        NodeExpr *node = unaryNode(stack);
        if (nullptr != node) {
          nodeStack.push(node);
        }
        if (false == stack.isEmpty()) {
          if (stack.first().isBinary()) {
            if (nodeStack.size() > 1) {
              NodeExpr *right = nodeStack.pop(),
                  *left = nodeStack.pop();
              Token op = stack.takeFirst();
              NodeExpr *tmp = new NodeExpr(left, right, Token::getBinary(op.value));
              nodeStack.push(tmp);
            }
          } else if (stack.first().isUnary()) {
            NodeExpr *tmp = nodeStack.pop();
            tmp = applyUnary(tmp, stack);
            nodeStack.push(tmp);
          }
        }
      }
      if (nodeStack.isEmpty()) {
        debug_log << "S";
        return nullptr;
      }
      NodeExpr *root = nodeStack.pop();
      return root;
    }

    ThematicCalc::NodeExpr *ThematicCalc::unaryNode(QList <Token> &stack) const {
      NodeExpr *node = nullptr;
      if (stack.size() > 1 && stack.at(1).isUnary()) {
        Token token = stack.takeFirst();
        Token op = stack.takeFirst();
        node = new NodeExpr(token, Token::getUnary(op.value));
      } else if (stack.first().isNumber() || stack.first().isVariable()) {
        node = new NodeExpr(stack.takeFirst());
      }
      return applyUnary(node, stack);
    }

    ThematicCalc::NodeExpr *ThematicCalc::applyUnary(NodeExpr *node, QList <Token> &stack) const {
      while (nullptr != node && !stack.isEmpty() && stack.first().isUnary()) {
        Token op = stack.takeFirst();
        node = new NodeExpr(node, Token::getUnary(op.value));
      }
      return node;
    }

    int ThematicCalc::mappingFromArrayIndex(const QString &name) {
      int index = 1;
      for (auto it = map_.begin(); it != map_.end(); ++it, ++index)
        if (it.key() == name) break;
      return index;
    }

    void ThematicCalc::processing(float* res, float** ch) const {
      switch (map_.size()) {
        case 0:  fn_(res, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr); break;
        case 1:  fn_(res, ch[0], nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr); break;
        case 2:  fn_(res, ch[0], ch[1], nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr); break;
        case 3:  fn_(res, ch[0], ch[1], ch[2], nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr); break;
        case 4:  fn_(res, ch[0], ch[1], ch[2], ch[3], nullptr, nullptr, nullptr, nullptr, nullptr, nullptr); break;
        case 5:  fn_(res, ch[0], ch[1], ch[2], ch[3], ch[4], nullptr, nullptr, nullptr, nullptr, nullptr); break;
        case 6:  fn_(res, ch[0], ch[1], ch[2], ch[3], ch[4], ch[5], nullptr, nullptr, nullptr, nullptr); break;
        case 7:  fn_(res, ch[0], ch[1], ch[2], ch[3], ch[4], ch[5], ch[6], nullptr, nullptr, nullptr); break;
        case 8:  fn_(res, ch[0], ch[1], ch[2], ch[3], ch[4], ch[5], ch[6], ch[7], nullptr, nullptr); break;
        case 9:  fn_(res, ch[0], ch[1], ch[2], ch[3], ch[4], ch[5], ch[6], ch[7], ch[8], nullptr); break;
        case 10: fn_(res, ch[0], ch[1], ch[2], ch[3], ch[4], ch[5], ch[6], ch[7], ch[8], ch[9]); break;
        default: break;
      }
    }

    QVector <uchar> ThematicCalc::dataProcessing() {
      for(const auto channel : map_.values()){
        if(!channel.hasData()){
          min_value_ = kErrConst;
          max_value_ = kErrConst;
          result_ = QVector<float>(map_.value(expression_).size(), kErrConst);
          size_ = result_.size();
          debug_log << QObject::tr("Нет данных в канале %1 размером %2")
                                .arg(QString::fromStdString(channel.name())).arg(size_);
          return QVector<uchar>(size_, 0);
        }
      }

      //Если математика не нужна
      if(map_.contains(expression_)){
        min_value_ = map_.value(expression_).min();
        max_value_ = map_.value(expression_).max();
        result_ = QVector<float>(map_.value(expression_));
        size_ = result_.size();
        debug_log << QObject::tr("Возвращаем массив размером %1 для %2").arg(size_).arg(expression_);
        return normalized();
      }

      delete root_;
      root_ = nullptr;

      root_ = createTree(stack_);
      if (nullptr == root_) {
        return QVector<uchar>();
      }
      root_->mapIndex_ = std::bind(&ThematicCalc::mappingFromArrayIndex, this, std::placeholders::_1);

      asmjit::JitRuntime rt;
      asmjit::CodeHolder code;
      // asmjit::FileLogger logger(stdout);
      code.init(rt.getCodeInfo());
      // code.setLogger(&logger);
      asmjit::X86Assembler a(&code);

      // ebp - function frame
      /* Раскомментировать при использовании локальных переменных, везде заменить esp на ebp и rsp на rbp
      {
    #ifndef __LP64__
        a.push(asmjit::x86::ebp);
        a.mov(asmjit::x86::ebp, asmjit::x86::esp);
    #else
        a.push(asmjit::x86::rbp);
        a.mov(asmjit::x86::rbp, asmjit::x86::rsp);
    #endif
      }
      */
      // dst - указатель на массив с результатом
      asmjit::X86Gp dst;
      {
#ifndef __LP64__
        //Кладем в eax адрес массива с результатом
        dst = asmjit::x86::eax;
        a.mov(dst, asmjit::x86::ptr_128(asmjit::x86::esp, sizeof(intptr_t)));
#else
        dst = asmjit::x86::rdi;
#endif
      }

      /*
      asmjit::X86Gp src;
    #ifndef __LP64__
      src = asmjit::x86::ebx;
      //Кладем в ebx адрес массива с первой переменной и загружаем переменные в xmm0
      a.movq(src, asmjit::x86::ptr_128(asmjit::x86::esp, 2 * sizeof(intptr_t)));
    #else
      src = asmjit::x86::rsi;
    #endif
      a.movups(asmjit::x86::xmm0, asmjit::x86::ptr_128(src));

    #ifndef __LP64__
      //Кладем в ebx адрес массива с первой переменной и загружаем переменны в xmm1
      a.mov(src, asmjit::x86::ptr_128(asmjit::x86::esp, 3 * sizeof(intptr_t)));
    #else
      src = asmjit::x86::rdx;
    #endif
      a.movups(asmjit::x86::xmm1, asmjit::x86::ptr_128(src));

      //Складываем xmm0 и xmm1
      a.addps(asmjit::x86::xmm0, asmjit::x86::xmm1);

      //создаем переменную
      float const1 = 128;
      //Записываем её в ebx
      a.mov(src, *((uint32_t*)&const1));
      //Векторизуем переменную из ebx в xmm1
      a.movd(asmjit::x86::xmm1, asmjit::x86::ebx);
      a.shufps(asmjit::x86::xmm1, asmjit::x86::xmm1, 0);

      //Складываем xmm0 и xmm1
      a.addps(asmjit::x86::xmm0, asmjit::x86::xmm1);

      //Загружаем ответ из xmm0
      a.movups(asmjit::x86::ptr_128(dst), asmjit::x86::xmm0);
    */

      int stack = 0;
      root_->generate(a, stack);
      a.movups(asmjit::x86::ptr_128(dst), asmjit::x86::xmm(root_->kStackSize - stack + 1));
      //a.leave();  // a.mov(rsp, rbp); a.pop(ebp); //Раскоментировать при использовании локальных переменных, везде заменить esp на ebp и rsp на rbp
      a.ret();    // return

      asmjit::Error err = rt.add(&fn_, &code);
      if (err) {
        error_log << err;
        result_.append(kErrConst);
        return QVector<uchar>(1, 0);
      }else{
        debug_log << QObject::tr("ASM функция успешно подготовлена: %1").arg(expression_);
      }

      size_ = map_.first().size();
      if (size_ == 0) {
        result_.push_back(0);
        return QVector<uchar>(1, 0);
      }
      result_.resize(size_);
      debug_log << QObject::tr("Размер выходных векторов: %1").arg(size_);

      populate();

      for (int i = 0; i < size_; i++){
        float val = result_.at(i);
        if (val == kErrConst) continue;
        if (val > max_value_) max_value_ = val;
        if (val < min_value_) min_value_ = val;
      }
      return normalized();
    }

    QVector<uchar> ThematicCalc::normalized(){
      debug_log << QObject::tr("Копируем данные в массив color. Мин-Макс: %1,%2").arg(min_value_, 0, 'f', 2).arg(max_value_, 0, 'f', 2);
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

    void ThematicCalc::populate() {
      int const map_size = map_.size();
      float **ch = new float *[map_size];
      for (int i = 0; i < map_size; ++i) ch[i] = new float[4];

      //process values by groups of 4
      for (int i = 0; i < size_; i++) {
        int n = 0;
        for (const auto channel: map_.values()) {
          ch[n++][i % 4] = channel.at(i);
        }
        //group of 4 populated, processing
        if (i % 4 == 3) { // TODO: maybe (i & 3) == 3 for speed
          float *result_pointer = result_.data() + i - 3;
          processing(result_pointer, ch);
        }
      }
      //process values in the last 0-4 places
      int size_remainder = size_ % 4;
      if (size_remainder != 0) {
        result_.reserve(size_ + (4 - size_remainder)); // чтобы не писать в память за вектором (SEGFAULT)
        float *result_pointer = result_.data() + size_ - size_remainder;
        processing(result_pointer, ch);
      }
      for (int i = 0; i < map_size; i++) delete[] ch[i];
      delete[] ch;

      //check again for invalid values in channels
      for (int i = 0; i < size_; i++) {
        for (const auto channel: map_.values()) {
          if (channel.at(i) == kErrConst) {
            result_[i] = kErrConst;
            break;
          }
        }
      }
    }
  } //commons
} //mappi
