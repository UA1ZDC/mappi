#include <stdlib.h>

#include <google/protobuf/compiler/plugin.h>

#include <cpp_generator.h>

int main( int argc, char* argv[] )
{
  google::protobuf::compiler::cpp::CppGenerator generator;
  return google::protobuf::compiler::PluginMain( argc, argv, &generator );
}
