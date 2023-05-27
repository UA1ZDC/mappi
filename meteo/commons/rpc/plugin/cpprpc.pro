TEMPLATE = app
TARGET   = meteo.rpc

LOG_MODULE_NAME = rpc

QT += network

PUB_HEADERS =

PRIVATE_HEADERS =       cpp_enum.h                      \
                        cpp_enum_field.h                \
                        cpp_extension.h                 \
                        cpp_field.h                     \
                        cpp_file.h                      \
                        cpp_generator.h                 \
                        cpp_helpers.h                   \
                        cpp_map_field.h                 \
                        cpp_message.h                   \
                        cpp_message_field.h             \
                        cpp_primitive_field.h           \
                        cpp_service.h                   \
                        cpp_string_field.h              \
                        mathlimits.h                    \
                        strutil.h                       \
                        substitute.h

HEADERS =     $$PUB_HEADERS  $$PRIVATE_HEADERS

SOURCES =               cpp_enum.cc                     \
                        cpp_enum_field.cc               \
                        cpp_extension.cc                \
                        cpp_field.cc                    \
                        cpp_file.cc                     \
                        cpp_generator.cc                \
                        cpp_helpers.cc                  \
                        cpp_map_field.cc                \
                        cpp_message.cc                  \
                        cpp_message_field.cc            \
                        cpp_primitive_field.cc          \
                        cpp_service.cc                  \
                        cpp_string_field.cc             \
                        mathlimits.cc                   \
                        strutil.cc                      \
                        substitute.cc                   \
                        plugin.cpp


LIBS =                  -lprotobuf                      \
                        -lprotoc

include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-sign-compare
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-function
