#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <iostream>

int main( int, char** )
{
  TAPPLICATION_NAME("taran");
  info_log << MnCommon::etcPath();
  return 0;
}
