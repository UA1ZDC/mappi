#include "gridfilelist.h"

#include "gridfs.h"

#include <cross-commons/debug/tlog.h>

namespace meteo {

GridFileList::GridFileList()
{
}

GridFileList::GridFileList( const GridFileList& o )
{
  refcnt_ = o.refcnt_;
  if ( nullptr != refcnt_ ) {
    *refcnt_ += 1;
  }
  filelist_ = o.filelist_;
}

GridFileList::GridFileList( GridFileListPrivate* filelist )
  : filelist_(filelist),
  refcnt_( new int32_t )
{
  *refcnt_ = 1;
}

GridFileList::~GridFileList()
{
  release();
}

void GridFileList::release()
{
  if ( nullptr != refcnt_ ) {
    *refcnt_ -= 1;
    if ( 0 == *refcnt_ ) {
      delete refcnt_;
      delete filelist_;
    }
    refcnt_ = nullptr;
    filelist_ = nullptr;
  }
}

GridFileList& GridFileList::operator=( const GridFileList& o )
{
  release();
  refcnt_ = o.refcnt_;
  if ( nullptr == refcnt_ ) {
    *refcnt_ += 1;
  }
  filelist_ = o.filelist_;
  return *this;
}

bool GridFileList::isValid() const
{
  if ( nullptr == filelist_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return filelist_->isValid();
}

bool GridFileList::hasList() const
{
  if ( nullptr == filelist_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return filelist_->hasList();
}

bool GridFileList::next()
{
  if ( nullptr == filelist_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return filelist_->next();
}

GridFile GridFileList::file() const
{
  if ( nullptr == filelist_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return GridFile();
  }
  return filelist_->file();
}

GridFileList::GridFileListPrivate::GridFileListPrivate()
{
}

GridFileList::GridFileListPrivate::~GridFileListPrivate()
{
}

}
