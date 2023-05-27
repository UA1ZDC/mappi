TEMPLATE = lib
TARGET   = meteo.appclient

QT      += widgets

LOG_MODULE_NAME = procview

SOURCES =               notfoundwidget.cpp              \
                        homepagewidget.cpp              \
                        tabwidgetappclient.cpp          \
                        remoteconnectiondlg.cpp         \
                        controllerappclient.cpp         \
                        monitorpagewidget.cpp           \
                        procmonitorwidget.cpp           \
                        windowappclient.cpp             \

FORMS =                 notfound.ui                     \
                        homepage.ui                     \
                        remoteconnection.ui             \
                        procmonitorwidget.ui            \
                        monitorpagewidget.ui            \

HEADERS =               notfoundwidget.h                \
                        homepagewidget.h                \
                        tabwidgetappclient.h            \
                        remoteconnectiondlg.h           \
                        controllerappclient.h           \
                        monitorpagewidget.h             \
                        procmonitorwidget.h             \
                        windowappclient.h               \

RESOURCES =             appclient.icons.qrc             \

LIBS +=                 -lprotobuf                      \

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -ltrpc                          \

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.proto                   \
                        -lmeteo.global                  \
                        -lmeteo.textproto               \
                        -lmeteo.viewheader   \

include( $$(SRC)/include.pro )

