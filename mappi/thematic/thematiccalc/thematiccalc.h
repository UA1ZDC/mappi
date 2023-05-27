#ifndef MAPPI_THEMATIC_THEMATICCALC_TEMATICCALC_H
#define MAPPI_THEMATIC_THEMATICCALC_TEMATICCALC_H

#include <cmath>
#include <functional>
#include <qmap.h>
#include <qvector.h>

#include <qstring.h>
#include <qtextstream.h>
#include <qdatastream.h>

#include <mappi/thematic/asmjit/asmjit.h>
#include <mappi/thematic/asmjit/x86/x86operand.h>
#include <mappi/thematic/asmjit/x86/x86inst.h>
#include <mappi/thematic/algs_calc/channel.h>

#include "rpn.h"

namespace mappi {

namespace thematic {

typedef void (*AsmFunc)(float*, float*, float*, float*, float*,
                                float*, float*, float*, float*,
                                float*, float*);

class ThematicCalc
{
  using MappingIndex = std::function<int(const QString&)>;
  struct NodeExpr {
#ifdef __LP64__
    static const int kStackSize = 15;
#else
    static const int kStackSize = 7;
#endif
    Token token_ = { Token::Type::kNull, "" };
    NodeExpr* left_ = nullptr;
    NodeExpr* right_ = nullptr;
    static MappingIndex mapIndex_;

    void generate(asmjit::X86Assembler &a, int& stack) {
      if(token_.isNumber()) {           // token - значение
        //создаем переменную
        float val = token_.value.toFloat();
        //Записываем её в ebx
        a.mov(asmjit::x86::ebx, *((uint32_t*)&val));
        //Векторизуем переменную из ebx в xmm(7-stack)
        a.movd(asmjit::x86::xmm(kStackSize - stack), asmjit::x86::ebx);
        a.shufps(asmjit::x86::xmm(kStackSize - stack), asmjit::x86::xmm(kStackSize - stack), 0);
      }
      else if(token_.isVariable()) {    //token - переменная
        //Кладем в ebx адрес массива с первой переменной и загружаем переменны в xmm(kStackSize - stack)
        int index = mapIndex_(token_.value);
        asmjit::X86Gp src;
#ifdef __LP64__
        switch (index) {
          case 1: src = asmjit::x86::rsi; break;
          case 2: src = asmjit::x86::rdx; break;
          case 3: src = asmjit::x86::rcx; break;
          case 4: src = asmjit::x86::r8;  break;
          case 5: src = asmjit::x86::r9;  break;
          default:
            src = asmjit::x86::rbx;
            a.mov(src, asmjit::x86::ptr_128(asmjit::x86::rsp, (index - 5) * sizeof(std::ptrdiff_t)));
        }
#else
        src = asmjit::x86::ebx;
        a.mov(src, asmjit::x86::ptr_128(asmjit::x86::esp, (index - 1) * sizeof(std::ptrdiff_t)));
#endif
        a.movups(asmjit::x86::xmm(kStackSize - stack), asmjit::x86::ptr_128(src));
      }
      else if(token_.isUnary()) {       //token - унарное выражение
        if(nullptr != left_) left_->generate(a, stack);
        if(stack > 0) {
          switch(token_.operation) {
          case Token::Operation::kNeg:
              //Записываем её в ebx
              a.mov(asmjit::x86::ebx, 2147483648);
              //Векторизуем переменную из ebx в xmm1
              a.movd(asmjit::x86::xmm(kStackSize - stack), asmjit::x86::ebx);
              a.shufps(asmjit::x86::xmm(kStackSize - stack), asmjit::x86::xmm(kStackSize - stack), 0);
              a.xorps(asmjit::x86::xmm(kStackSize - stack + 1), asmjit::x86::xmm(kStackSize - stack));
            break;
          case Token::Operation::kPos:
            break;
          case Token::Operation::kAbs:
            break;
          default:
            break;
          }
          stack--;
        }
      }
      else if(token_.isBinary()) {
        if(nullptr != left_)  left_->generate (a, stack);
        if(nullptr != right_) right_->generate(a, stack);
        if(stack > 1) {
          stack -= 2;
          switch (token_.operation) {
          case Token::Operation::kAdd:
            a.addps(asmjit::x86::xmm(kStackSize - stack), asmjit::x86::xmm(kStackSize - stack - 1));
            break;
          case Token::Operation::kSub:
            a.subps(asmjit::x86::xmm(kStackSize - stack), asmjit::x86::xmm(kStackSize - stack - 1));
            break;
          case Token::Operation::kMul:
            a.mulps(asmjit::x86::xmm(kStackSize - stack), asmjit::x86::xmm(kStackSize - stack - 1));
            break;
          case Token::Operation::kDiv:
            a.divps(asmjit::x86::xmm(kStackSize - stack), asmjit::x86::xmm(kStackSize - stack - 1));
            break;
          default:
            break;
          }
        }
      }
      stack++;
    }

    void setLeft(NodeExpr* left) { left_ = left; token_ = { Token::Type::kUnary, "" }; }
    void setRight(NodeExpr* right) { right_ = right; token_ = { Token::Type::kBinary, "" }; }

    NodeExpr() {}
    NodeExpr(Token left)
      : token_(left) {}
    NodeExpr(Token left, Token::Operation _op)
      : token_(Token::Type::kUnary, _op), left_(new NodeExpr(left)) {}
    NodeExpr(NodeExpr* left, Token::Operation _op)
      : token_(Token::Type::kUnary, _op), left_(left) {}
    NodeExpr(Token left, Token right, Token::Operation _op)
      : token_(Token::Type::kBinary, _op), left_(new NodeExpr(left)), right_(new NodeExpr(right)) {}
    NodeExpr(NodeExpr *left, Token right, Token::Operation _op)
      : token_(Token::Type::kBinary, _op), left_(left), right_(new NodeExpr(right)) {}
    NodeExpr(Token left, NodeExpr* right, Token::Operation _op)
      : token_(Token::Type::kBinary, _op), left_(new NodeExpr(left)), right_(right) {}
    NodeExpr(NodeExpr *left, NodeExpr* right, Token::Operation _op)
      : token_(Token::Type::kBinary, _op), left_(left), right_(right) {}
    virtual ~NodeExpr() { delete left_; delete right_; }
  };

public:
  explicit ThematicCalc();
  ~ThematicCalc();

  void  bindArray(const QString& name, to::Channel& vec);
  bool  parsingFile(const QString& filename);
  bool  parsingExpression(const QString& data_);
  bool prepareASMFunction();
  QVector<uchar> dataProcessing();
  void populate();
  QVector<uchar> normalized();
  template<typename T, typename U>
  T normalize(U value);
  template<typename T, typename U>
  T normalize(U value, U old_min, U old_max);

  QVector<float> getResult() { return result_; }
  AsmFunc  fn_;

private:
  NodeExpr* createTree(QList<Token> stack);

  NodeExpr* unaryNode(QList<Token>& stack) const;
  NodeExpr* applyUnary(NodeExpr* node, QList<Token>& stack) const;

  void processing(float* res, float** ch = nullptr) const;
private:
  QList<Token> stack_;
  NodeExpr* root_;
  void generate();

  QString expression_;
  QMap<QString, to::Channel> map_;
  int size_ = 0;
  float min_value_ = std::numeric_limits<float>::max();
  float max_value_ = std::numeric_limits<float>::min();
  QVector<float> result_;

private:
  int mappingFromArrayIndex(const QString& name);
};

} //thematic
} //mappi

#endif // MAPPI_THEMATIC_THEMATICCALC_TEMATICCALC_H
