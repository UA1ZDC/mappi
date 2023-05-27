#include "taction.h"
#include <cross-commons/debug/tlog.h>

TAction::TAction( const QString &text, QObject* parent ) :
  QAction(text, parent),
  uuid_(QString())
{
}

