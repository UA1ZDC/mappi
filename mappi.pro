TEMPLATE = subdirs
SUBDIRS = mappi

isEmpty(SRC):SRC = $$(SRC)
isEmpty(SRC){
  error("SRC is empty.")
}else{
  message("SRC --> $$SRC")
}

isEmpty(BUILD_DIR):BUILD_DIR = $$(BUILD_DIR)
isEmpty(BUILD_DIR){
  error("BUILD_DIR is empty.")
}else{
  message("BUILD_DIR --> $$BUILD_DIR")
}

isEmpty(PACKAGE_DIR):PACKAGE_DIR = $$(PACKAGE_DIR)
isEmpty(PACKAGE_DIR){
  error("PACKAGE_DIR is empty.")
}else{
  message("PACKAGE_DIR --> $$PACKAGE_DIR")
}

isEmpty(PRJ_DIR):PRJ_DIR = $$(PRJ_DIR)
isEmpty(PRJ_DIR){
  error("PRJ_DIR is empty.")
}else{
  message("PRJ_DIR --> $$PRJ_DIR")
}

isEmpty(QTV):QTV = $$(QT_VERSION)
isEmpty(QTV){
  error("QT_VERSION is empty.")
}else{
  message("QT_VERSION --> $$QTV")
}
message("QT_FULL -->:    $$[QT_VERSION]")

isEmpty(PROTOC_VER):PROTOC_VER = $$(PROTOC_VER)
isEmpty(PROTOC_VER){
  error("PROTOC_VER is empty.")
}else{
  message("PROTOC_VER --> $$PROTOC_VER")
}