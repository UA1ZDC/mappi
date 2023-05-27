#
# Qt qmake integration with Google Protocol Buffers compiler protoc
#
# To compile protocol buffers with qt qmake, specify PROTOS variable and
# include this file
#
# Example:
# LIBS += /usr/lib/libprotobuf.so
# PROTOS = a.proto b.proto
# include(protobuf.pri)
#
# By default protoc looks for .proto files (including the imported ones) in
# the current directory where protoc is run. If you need to include additional
# paths specify the PROTOPATH variable
#
message("Using sql standalone protobuf.pri")
# необходимо для работы с protobuf v3
QMAKE_CXXFLAGS += -Wno-unused-parameter

PROTO_IPATH = $$replace(_PRO_FILE_PWD_, $$PWD, '')

for(p, PROTOPATH):INCLUDEPATH += $${p}

PROTOPATH += /usr/include
PROTOPATH += ${QMAKE_FILE_IN_PATH}
for(p, PROTOPATH):PROTOPATHS += --proto_path=$${p}

protobuf_decl.name = protobuf header
protobuf_decl.input = PROTOS
protobuf_decl.output = ${QMAKE_FILE_BASE}.pb.h
protobuf_decl.commands = protoc --cpp_out="${QMAKE_FILE_IN_PATH}" $${PROTOPATHS} ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_NAME} \
                          && mkdir -p $$(BUILD_DIR)/include/$${PROTO_IPATH} \
                          && cp ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.h  $$(BUILD_DIR)/include \
                          && cp ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.h  $$(BUILD_DIR)/include/$${PROTO_IPATH}

protobuf_decl.variable_out = GENERATED_FILES
protobuf_decl.CONFIG += target_predeps
QMAKE_EXTRA_COMPILERS += protobuf_decl

protobuf_impl.name = protobuf implementation
protobuf_impl.input = PROTOS
protobuf_impl.output = ${QMAKE_FILE_BASE}.pb.cc
protobuf_impl.depends = ${QMAKE_FILE_BASE}.pb.h
protobuf_impl.commands = $$escape_expand(\\n)
protobuf_impl.variable_out = GENERATED_SOURCES
QMAKE_EXTRA_COMPILERS += protobuf_impl

!isEmpty( PROTOS ){
  for(p, PROTOS):PROTO_HEADERS += $$replace(p, '.proto', '.pb.h')
  proto_headers.path  = $$(BUILD_DIR)/include/$$PROTO_IPATH
  proto_headers.files = $$PROTO_HEADERS
  !win32 {proto_headers.extra = chmod a+r $$PROTO_HEADERS}
  INSTALLS += proto_headers

  proto_headers2.path  = $$(BUILD_DIR)/include
  proto_headers2.files = $$PROTO_HEADERS
  INSTALLS += proto_headers2
}

protobuf_py_decl.name = protobuf header
protobuf_py_decl.input = PROTOS
protobuf_py_decl.output = ${QMAKE_FILE_BASE}_pb2.py
protobuf_py_decl.commands = protoc --python_out="${QMAKE_FILE_IN_PATH}" $${PROTOPATHS} ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_NAME}

protobuf_py_decl.variable_out = GENERATED_FILES
protobuf_py_decl.CONFIG += target_predeps
QMAKE_EXTRA_COMPILERS += protobuf_py_decl

VER = $$(PROTOC_VER)
contains( VER, 3 ){
  protobuf_js_decl.name = protobuf header
  protobuf_js_decl.input = PROTOS
  protobuf_js_decl.output = ${QMAKE_FILE_BASE}.pb.js
  protobuf_js_decl.commands = protoc --js_out=library="${QMAKE_FILE_BASE}.pb",binary:"${QMAKE_FILE_IN_PATH}" $${PROTOPATHS} ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_NAME}

  protobuf_js_decl.variable_out = GENERATED_FILES
  protobuf_js_decl.CONFIG += target_predeps
  QMAKE_EXTRA_COMPILERS += protobuf_js_decl
}

contains( ENV_QTV, QT4 ):!isEmpty( PROTOS ){
  for( p, PROTOS): PROTOPY_FILES += $$replace(p, ".proto", "_pb2.py")
  protopy2.path  = $$(BUILD_DIR)/include.py/
  protopy2.files = $${PROTOPY_FILES}
  protopy2.CONFIG += no_check_exist
  INSTALLS += protopy2
}

contains( ENV_QTV, QT5 ) {
  !isEmpty( PROTOS ) {
    for( p, PROTOS): PROTOPY_FILES += $$replace(p, ".proto", "_pb2.py")
    protopy2.path  = $$(BUILD_DIR)/include.py/
    protopy2.files = $${PROTOPY_FILES}
    protopy2.CONFIG += no_check_exist
    INSTALLS += protopy2
  }
}

contains( ENV_QTV, QT5 ) {
  !isEmpty( PROTOS ) {
    for( p, PROTOS): PROTOJS_FILES += $$replace(p, ".proto", ".pb.js")
    protojs.path  = $$(BUILD_DIR)/include.js/
    protojs.files = $${PROTOJS_FILES}
    protojs.CONFIG += no_check_exist
    INSTALLS += protojs
  }
}
