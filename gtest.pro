win32 {
  INCLUDEPATH += $$(GTEST_INCLUDE)
  QMAKE_LIBDIR += $$(GTEST_LIB)
}
LIBS += -lgtest_main
