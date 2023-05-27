#include "labellayout.h"

#include <cmath>

#include <qrect.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {

enum Direction { kLeft = -1, kRight = 1, kNone = 0 };

class IBlock
{
public:
  IBlock()
  {
    right = 0;
    left = 0;
    size = 0;
  }

  QString name() const { return name_; }
  void setNameIndex(int i) { name_ += QString::number(i); }

  virtual int weight() const { return 0; }
  virtual int weightInc(Direction /*dir*/) const { return 0; }
  virtual int calcLeftWeightSumm(int w = 0) const = 0;
  virtual int calcRightWeightSumm(int w = 0) const = 0;

  int calcLeftIncWeight(Direction dir, int w = 0) const
  {
    w += weightInc(dir);

    if ( 0 != left ) { w = left->calcLeftIncWeight(dir, w); }

    return w;
  }
  int calcRightIncWeight(Direction dir, int w = 0) const
  {
    w += weightInc(dir);

    if ( 0 != right ) { w = right->calcRightIncWeight(dir, w); }

    return w;
  }

  virtual int calcNextBlocks(int n = 0) const
  {
    if ( 0 == right ) { return n; }
    return right->calcNextBlocks(n);
  }
  virtual int calcPrevBlocks(int n = 0) const
  {
    if ( 0 == right ) { return n; }
    return right->calcPrevBlocks(n);
  }

  virtual bool needMove() const { return weight() != 0; }
  virtual bool canMove(Direction dir, const IBlock* block) = 0;
  virtual bool move(Direction dir, const IBlock* block) = 0;

  virtual int pos() const { return 0; }
  Direction direction() const
  {
    if ( weight() > 0 ) { return kRight; }
    else if ( weight() < 0 ) { return kLeft;}
    return kNone;
  }

  IBlock* right;
  IBlock* left;

  int size;

protected:
  QString name_;
};

class FreeSpace : public IBlock
{
public:
  FreeSpace() : IBlock()
  {
    name_ = "sp";
  }

  virtual int calcLeftWeightSumm(int w = 0) const
  {
    if ( size > 0 ) { return w; }
    if ( 0 == left ) { return w; }

    return left->calcLeftWeightSumm(w);
  }
  virtual int calcRightWeightSumm(int w = 0) const
  {
    if ( size > 0 ) { return w; }
    if ( 0 == right ) { return w; }

    return right->calcRightWeightSumm(w);
  }

  virtual bool move(Direction dir, const IBlock* block)
  {
    assert_log( 0 != block );
//    assert_log( right == block || left == block ) << var(block) << var(this);

    if ( size > 0 ) { return true; }

    bool res = true;

    switch ( dir ) {
      case kLeft:
        if ( 0 != left ) {
          res = left->move(dir, this);
        }
        break;
      case kRight:
        if ( 0 != right ) {
          res = right->move(dir, this);
        }
        break;
      case kNone:
        break;
    }

    return res;
  }

  virtual bool canMove(Direction dir, const IBlock* block)
  {
    if ( size > 0 ) { return true; }

    bool f = false;
    if ( kRight == dir && 0 != right && right != block ) {
      f = right->canMove(dir, this);
    }
    else if ( kLeft == dir && 0 != left && left != block ) {
      f = left->canMove(dir, this);
    }
    return f;
  }
};

class Block : public IBlock
{
public:
  Block() : targetPos(0) { name_ = "bl."; }
  Block(double tPos) : targetPos(tPos) { name_ = "bl."; }

  double targetPos;
  double pos_;

  virtual int pos() const { return pos_; }
  virtual int weight() const { return std::floor(targetPos - pos_); }
  virtual int weightInc(Direction dir) const
  {
    int w = weight();
    return w - (w + dir);
  }

  virtual int calcLeftWeightSumm(int w = 0) const
  {
    w += weight();
    if ( 0 == left ) { return w; }

    return left->calcLeftWeightSumm(w);
  }

  virtual int calcRightWeightSumm(int w = 0) const
  {
    w += weight();
    if ( 0 == right ) { return w; }

    return right->calcRightWeightSumm(w);
  }

  virtual bool needMove() const { return weight() != 0; }
  virtual bool move(Direction dir, const IBlock* /*block*/)
  {
//    none_log << name() << "MOVE from" << pos << "dir" << dir;

    if ( !canMove(dir,0) ) { return false; }

    bool res = true;

    switch ( dir ) {
      case kLeft:
        if ( 0 != left ) {
          res = left->move(dir, this);
        }
        break;
      case kRight:
        if ( 0 != right ) {
          res = right->move(dir, this);
        }
        break;
      case kNone:
        break;
    }

    if ( true == res ) {
      pos_ += dir;
      if ( 0 != left )  { left->size += dir; }
      if ( 0 != right ) { right->size -= dir; }
    }

    return res;
  }

  virtual bool canMove(Direction dir, const IBlock* /*block*/)
  {
    int lW  = 0;
    int rW = 0;
    int lInc  = 0;
    int rInc = 0;

//    none_log << name() << "CAN MOVE?" << var(pos()) << var(targetPos) << var(weight()) << var(dir);

    switch ( dir ) {
      case kLeft: {
        rW = calcRightWeightSumm();
        lW  = ( 0 != left  ) ? left->calcLeftWeightSumm() : 0;
        rInc = calcRightIncWeight(dir);
        lInc  = ( 0 != left ) ? left->calcLeftIncWeight(dir) : 0;
      } break;
      case kRight: {
        lW = calcLeftWeightSumm();
        rW = ( 0 != right ) ? right->calcRightWeightSumm() : 0;
        lInc  = calcLeftIncWeight(dir);
        rInc = ( 0 != right ) ? right->calcRightIncWeight(dir) : 0;
      } break;
      case kNone: {
//        none_log << "--> can: true";
        return true;
      }
    }
//    var(lW) << var(lInc) << var(rW) << var(rInc);
    lW += lInc;
    rW += rInc;
//    var(lW) << var(rW);

    if ( ( lW == -rW )
         || ( dir == kRight && lW + rW > 0 )
         || ( dir == kLeft && lW + rW < 0 )
         )
    {
//      none_log << "--> can: true";
      return true;
    }

//    none_log << "--> can: false";
    return false;
  }
};

class Axis
{
  double beg_;
  double size_;

  QList<IBlock*> blocks_;

public:
  Axis(double beg, double end)
  {
    beg_ = beg;
    size_ = qAbs(beg - end);

    last = new FreeSpace;
    last->size = size_;
  }

  inline double size() const { return size_; }

  IBlock* last;

  bool append(Block* block)
  {
    if ( block->size > last->size ) { return false; }

    block->pos_ = beg_ + (size_ - last->size) + block->size;

    IBlock* sp = new FreeSpace;

    sp->right = block;
    sp->left = last->left;
    block->right = last;
    block->left = sp;

    if ( 0 != last->left ) {
      last->left->right = sp;
    }
    last->left = block;

    last->size -= block->size;

    return false;
  }

  QVector<double> optimPositions() const
  {
    QVector<double> list;

    for ( IBlock* b = last; 0 != b; b = b->left ) {
      if ( b->name().startsWith("sp") ) {
        continue;
      }
      list.prepend(b->weight());
    }
    return list;
  }

  void debug()
  {
    QStringList list;
    for ( IBlock* b = last; 0 != b; b = b->left ) {
      QString s;
      if ( b->name() == "sp" ) {
        s = b->name() + "[" + QString::number(b->size) + "]";
      }
      else {
        s = b->name() + " [p:" +
            QString::number(b->pos()) + " s:" +
//            QString::number(b->size) + " w:" +
            QString::number(b->weight()) + "]";
      }
      list.prepend(s);
    }
    none_log << list.join("  ");
  }
};



LabelLayout::LabelLayout()
{
  top_ = 0;
  bot_ = 0;
}

void LabelLayout::setRamkaPos(double top, double bottom)
{
  top_ = top;
  bot_ = bottom;
}

void LabelLayout::addLabel(double pos, double size)
{
  labels_ << qMakePair(pos, size);
}

QVector<double> LabelLayout::optimPosition() const
{
  double summ = 0;
  for ( int i=0,isz=labels_.size(); i<isz; ++i ) {
    summ += labels_[i].second;
  }

  double ramkaSz = qAbs(top_ - bot_);
  if ( summ > ramkaSz ) {
    double delta = -qAbs(summ - ramkaSz) / 2;

    double prevSizeSumm = 0;
    QVector<double> list(labels_.size(), 0);
    for ( int i=0,isz=list.size(); i<isz; ++i ) {
      list[i] = delta + labels_[i].second + prevSizeSumm - labels_[i].first;
      prevSizeSumm += labels_[i].second;
    }
    return list;
  }

  Axis axis(top_, bot_);
//  axis.debug();
  for ( int i=0,isz=labels_.size(); i<isz; ++i ) {
    Block* block = new Block(labels_[i].first);
    block->setNameIndex(i);
    block->size = labels_[i].second;
    axis.append(block);
  }

//  axis.debug();
  while ( true ) {
    int nMove = 0;
    for ( IBlock* block = axis.last; 0 != block; block = block->left ) {
      if ( !block->needMove() ) {
        continue;
      }

//      none_log << "------ try MOVE" << block->name();
      if ( block->move(block->direction(), 0) ) {
        ++nMove;
//        none_log << "---> MOVED";
      }
//      axis.debug();
    }


    if ( 0 == nMove ) { break; }
  }

//  var(axis.optimPositions());
  return axis.optimPositions();
}

bool LabelLayout::hasIntersects(const QVector<double>& offsets) const
{
  assert_log( labels_.size() == offsets.size() );

  QList<QRect> rects;
  for ( int i=0,isz=labels_.size(); i<isz; ++i ) {
    QPoint p(labels_[i].first + offsets[i], 0);
    QSize sz(labels_[i].second, 1);
    rects << QRect(p,sz);
  }

  for ( int i=0,isz=rects.size(); i<isz; ++i ) {
    for ( int j=0,jsz=rects.size(); j<jsz; ++j ) {
      if ( i == j ) { continue; }

      if ( rects[i].intersects(rects[j]) ) {
        return true;
      }
    }
  }

  return false;
}

} // meteo
