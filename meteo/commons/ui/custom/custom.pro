TARGET   = tcustomui
TEMPLATE = lib

QT +=  widgets script

LOG_MODULE_NAME = vizdoc

SOURCES   =           tfilestreamoptions.cpp            \
                      customtreewidgetitem.cpp          \
                      tftpstreamoptions.cpp             \
                      geopointeditor.cpp                \
                      tunimasstreamoptions.cpp          \
                      docoptions.cpp                    \
                      clicklabel.cpp                    \
                      textposdlg.cpp                    \
                      fontwidget.cpp                    \
                      fontcombobox.cpp                  \
                      alignbutton.cpp                   \
                      symbdlg.cpp                       \
                      symbolbtn.cpp                     \
                      colordlg.cpp                      \
                      colorbtn.cpp                      \
                      iconsetbtn.cpp                    \
                      iconsetdlg.cpp                    \
                      multichoosebtn.cpp                \
                      multichoosedlg.cpp                \
                      timesheet.cpp                     \
                      stationwidget.cpp                 \
                      stationfind.cpp                   \
                      multichoosewgt.cpp                \
                      timesheetwgt.cpp                  \
                      stretchcombobox.cpp               \
                      choosefile.cpp                    \
                      choosedir.cpp                     \
                      tlgeditor.cpp                     \
                      coordedit.cpp                     \
                      ahdwidget.cpp                     \
                      preparewidget.cpp                 \
                      placewidget.cpp                   \
                      resizedcombo.cpp                  \
                      intervaledit.cpp                  \
                      filterheader.cpp                  \
                      filteredit.cpp                    \
                      placemodel.cpp                    \
                      selectlevel.cpp                   \
                      sloywidget.cpp                    \
                      zondwidget.cpp                    \
                      alphaedit.cpp                     \
                      doublespinaction.cpp              \
                      colorwidget.cpp                   \
                      gradienteditor.cpp                \
                      descrselect.cpp                   \
                      saveiface.cpp                     \
                      layerename.cpp                    \
                      filedialogrus.cpp                 \
                      doubleslider.cpp                  \
                      selectstation.cpp                 \
                      selectdatewidget.cpp              \
                      timeselectwidget.cpp              \
                      forecastwidgetinput.cpp           \
                      longintspinbox.cpp                \
                      forecastformwidget.cpp            \
                      forecastwidgetbutton.cpp          \
                      forecastwidgetitem.cpp            \
                      forecastwidgetresultbox.cpp       \
                      checkpanel.cpp                    \
                      calendardlg.cpp                   \
                      calendarbtn.cpp                   \
                      userselectform.cpp                



PUB_HEADERS     =     tfilestreamoptions.h              \
                      customtreewidgetitem.h            \
                      tftpstreamoptions.h               \
                      geopointeditor.h                  \
                      tunimasstreamoptions.h            \
                      docoptions.h                      \
                      clicklabel.h                      \
                      textposdlg.h                      \
                      fontwidget.h                      \
                      fontcombobox.h                    \
                      alignbutton.h                     \
                      symbdlg.h                         \
                      symbolbtn.h                       \
                      colordlg.h                        \
                      colorbtn.h                        \
                      iconsetbtn.h                      \
                      iconsetdlg.h                      \
                      multichoosebtn.h                  \
                      multichoosedlg.h                  \
                      timesheet.h                       \
                      stationwidget.h                   \
                      stationfind.h                     \
                      multichoosewgt.h                  \
                      timesheetwgt.h                    \
                      stretchcombobox.h                 \
                      choosefile.h                      \
                      choosedir.h                       \
                      tlgeditor.h                       \
                      coordedit.h                       \
                      ahdwidget.h                       \
                      preparewidget.h                   \
                      placewidget.h                     \
                      resizedcombo.h                    \
                      intervaledit.h                    \
                      filterheader.h                    \
                      filteredit.h                      \
                      placemodel.h                      \
                      selectlevel.h                     \
                      sloywidget.h                      \
                      zondwidget.h                      \
                      alphaedit.h                       \
                      doublespinaction.h                \
                      colorwidget.h                     \
                      gradienteditor.h                  \
                      descrselect.h                     \
                      saveiface.h                       \
                      layerename.h                      \
                      filedialogrus.h                   \
                      selectstation.h                   \
                      selectdatewidget.h                \
                      forecastwidgetinput.h             \
                      forecastformwidget.h              \
                      forecastwidgetbutton.h            \
                      forecastwidgetitem.h              \
                      forecastwidgetresultbox.h         \
                      timeselectwidget.h                \
                      checkpanel.h                      \
                      doubleslider.h                    \
                      longintspinbox.h                  \
                      calendardlg.h                     \
                      calendarbtn.h                     \
                      userselectform.h

LIBS +=               -lprotobuf                        \
                      -lmeteo.proto                     \
                      -lmeteo.textproto                 \
                      -lmeteo.geobasis                  \
                      -lmeteo.etc                       \
                      -lmeteo.font                      \
                      -lmeteo.global                    \
                      -lmeteodata                       \
                      -ltapp                            \
                      -ltdebug                          \
                      -lmnfuncs                         \
                      -lmeteo.sql                        \
                      -ltrpc                            \
                      -lmnmathtools

FORMS =               filestreamoptions.ui              \
                      ftpstreamoptions.ui               \
                      unimasstreamoptions.ui            \
                      geopointeditor.ui                 \
                      docoptions.ui                     \
                      textposdlg.ui                     \
                      fontwidget.ui                     \
                      stationwidget.ui                  \
                      stationfind.ui                    \
                      choosefile.ui                     \
                      choosedir.ui                      \
                      tlgeditor.ui                      \
                      ahdwidget.ui                      \
                      preparewidget.ui                  \
                      placewidget.ui                    \
                      selectlevel.ui                    \
                      intervaledit.ui                   \
                      sloywidget.ui                     \
                      alphaedit.ui                      \
                      gradienteditor.ui                 \
                      descrselect.ui                    \
                      saveiface.ui                      \
                      layerename.ui                     \
                      selectstation.ui                  \
                      selectdatewidget.ui               \
                      forecastformwidget.ui             \
                      timeselectwidget.ui               \
                      calendardlg.ui                    \
                      userselectform.ui                 
                     



            
                      
include( $$(SRC)/include.pro )

RESOURCES += \
  ../../rc/meteo.icons.qrc \
  ../appclient/appclient.icons.qrc
