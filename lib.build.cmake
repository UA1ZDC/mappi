include( $ENV{SRC}/lib.cmake )

FUNCTION( f_print_help )
 message( "Error: Unknown target '$ENV{target}'" )
  message( "Usage: build.sh [target ...]" )
  message( "--prefix PATH     - устанавливает PRJ_DIR" )
  message( "build             - полная сборка (используется по умолчанию если не указаны другие цели)" )
  message( "build-nodeps      - сборка без зависимостей" )
  message( "" )
  message( "autotests         - для каждого проекта, указанного в секции QMAKE_PROJECTS," )
  message( "                    проверяет наличие директории autotest и если она существует," )
  message( "                    собирает находящийся в ней проект." )
  message( "autotests-all     - выполняет autotest для проекта и его зависимостей" )
  message( "" )
  message( "run-autotests     - для каждого проекта, указанного в секции QMAKE_PROJECTS, ")
  message( "                    проверяет наличие директории autotest и если она существует, запускает файл test" )
  message( "                    если тест пройден успешно - ставится метка [pass]" )
  message( "                    если тест завершился неудачей - ставится метка [fail] и в разделе Test log выводится stdout и stderr" )
  message( "                    если проект не покрыт тестами - ставится метка [not-covered]" )
  message( "run-autotests-all - выполняет run-autotest для проекта и его зависимостей" )
  message( "" )
  message( "clean             - выполняет цель clean (из Makefile) для qmake и make проектов" )
  message( "clean-all         - clean для текущего проекта и его зависимостей" )
  message( "" )
  message( "uninstall         - удаляет установленные файлы" )
  message( "uninstall-all     - uninstall для текущего проекта и его зависимостей" )
  message( "" )
  message( "distclean         - выполняет цель distclean (из Makefile) для qmake и make проектов" )
  message( "distclean-all     - distclean для текущего проекта и его зависимостей" )
  message( "" )
  message( "remove-build      - удаляет директорию $BUILD_DIR" )
  message( "" )
  message( "fullclean         - полная очистка текущего проекта:" )
  message( "                    1) выполнение distclean              " )
  message( "                    2) удаление директорий *.o, *.moc, *.rcc, *.ui")
  message( "                    3) удаление файлов *.pb.cc, *.pb.h")
  message( "                    4) удаление пустых директорий autotest")
  message( "fullclean-all     - выполняется fullclean для текущего проекта и его зависимостей + remove-build" )
ENDFUNCTION()


if( $ENV{TARGET} STREQUAL "build" )
  set( res_code 0 )
  f_process_deps( "${DEPENDS_PROJECTS}" res_code )

  if( NOT ${res_code} EQUAL 1 )
    message( "<${PROJECT_NAME}>")
    f_make_install( ${PRO_DIR} "${MAKE_PROJECTS}" )
    f_qmake_make_install( ${PRO_DIR} "${QMAKE_PROJECTS}" )
  endif()
elseif( $ENV{TARGET} STREQUAL "build-nodeps" OR $ENV{TARGET} STREQUAL "nodeps" )
  message( "<${PROJECT_NAME}>")
  f_make_install( ${PRO_DIR} "${MAKE_PROJECTS}" )
  f_qmake_make_install( ${PRO_DIR} "${QMAKE_PROJECTS}" )

elseif( $ENV{TARGET} STREQUAL "autotests" )
  message( "<${PROJECT_NAME} -- autotests >")

  set( autotest_projects )
  f_autotest_list( ${PRO_DIR} "${QMAKE_PROJECTS}" autotest_projects )
  
  if( NOT autotest_projects )
    message( "--- autotest projects not found ---" )
    return()
  endif()

  f_qmake_make( ${PRO_DIR} "${autotest_projects}" )

elseif( $ENV{TARGET} STREQUAL "autotests-all" )
  set( res_code 0 )
  f_process_deps( "${DEPENDS_PROJECTS}" res_code )

  if( ${res_code} EQUAL 0 )
    message( "<${PROJECT_NAME} -- autotests >")

    set( autotest_projects )
    f_autotest_list( ${PRO_DIR} "${QMAKE_PROJECTS}" autotest_projects )

    if( NOT autotest_projects )
      message( "--- autotest projects not found ---" )
      return()
    endif()
  endif()

  f_qmake_make( ${PRO_DIR} "${autotest_projects}" )

elseif( $ENV{TARGET} STREQUAL "run-autotests" OR $ENV{TARGET} STREQUAL "run-autotests-all" )
  if( $ENV{TARGET} STREQUAL "run-autotests-all" )
    f_process_deps( "${DEPENDS_PROJECTS}" res_code )
  endif()

  set(autotest_projects)
  set(without_autotest)
  foreach( name ${QMAKE_PROJECTS} )
    set( path "${PRO_DIR}/${name}/autotest" )
    FILE(TO_NATIVE_PATH ${path} native_path)

    if(EXISTS "${native_path}" AND IS_DIRECTORY "${native_path}")
      set(autotest_projects ${autotest_projects} "${name}/autotest")
    else()
      set( without_autotest "${without_autotest}[not-covered] ${PROJECT_NAME}/${name}\n" )
    endif()
  endforeach()

  foreach( test ${autotest_projects} )
    set( path ${PRO_DIR}/${test} )
    FILE(TO_NATIVE_PATH ${path} native_path)
    
    if( EXISTS ${native_path}/test )
      execute_process(
        COMMAND ${native_path}/test
        WORKING_DIRECTORY ${native_path}
        RESULT_VARIABLE res
        OUTPUT_VARIABLE out
        ERROR_VARIABLE  err
      ) 
 
      set( res_text "pass" )

      if( ${res} EQUAL 1 )
        set( res_text "fail" )
        set( logs "${logs}\nOUT for ${native_path}" )
        set( logs "${logs}\n${out}" )
        set( logs "${logs}\nERR for ${native_path}" )
        set( logs "${logs}\n${err}" )
      endif()
    else()
      set( res_text "not-run" )
    endif()

    set( result_table "${result_table}[${res_text}] ${PROJECT_NAME}/${test}\n" )
  endforeach()

  message( "Test results:\n${result_table}${without_autotest}\n" )
  if( DEFINED logs )
    message( "Test logs:${logs}" )
  endif()

elseif( $ENV{TARGET} STREQUAL "clean" )
  f_clean(${PRO_DIR} "${QMAKE_PROJECTS}")
  f_clean(${PRO_DIR} "${MAKE_PROJECTS}")

elseif( $ENV{TARGET} STREQUAL "clean-all" )
  set( res_code 0 )
  f_process_deps( "${DEPENDS_PROJECTS}" res_code )

  message( "<${PROJECT_NAME}>" )
  f_clean(${PRO_DIR} "${QMAKE_PROJECTS}")
  f_clean(${PRO_DIR} "${MAKE_PROJECTS}")

  set(autotest_projects)
  f_autotest_list( ${PRO_DIR} "${QMAKE_PROJECTS}" autotest_projects )

  if( autotest_projects )
    message( "<${PROJECT_NAME}> -- autotests")
    f_clean(${PRO_DIR} "${autotest_projects}")
    # TODO удалять исполняемые файлы для тестов
  endif()

elseif( $ENV{TARGET} STREQUAL "uninstall" )
  f_uninstall(${PRO_DIR} "${QMAKE_PROJECTS}")
  f_uninstall(${PRO_DIR} "${MAKE_PROJECTS}")
elseif( $ENV{TARGET} STREQUAL "uninstall-all" )
  f_process_deps( "${DEPENDS_PROJECTS}" res_code )

  message( "<${PROJECT_NAME}>")
  f_uninstall(${PRO_DIR} "${QMAKE_PROJECTS}")
  f_uninstall(${PRO_DIR} "${MAKE_PROJECTS}")
elseif( $ENV{TARGET} STREQUAL "distclean" )
  f_distclean(${PRO_DIR} "${QMAKE_PROJECTS}")
  f_distclean(${PRO_DIR} "${MAKE_PROJECTS}")
  
  set(autotest_projects)
  f_autotest_list( ${PRO_DIR} "${QMAKE_PROJECTS}" autotest_projects )

  if( autotest_projects )
    message( "<${PROJECT_NAME}> -- autotests")
    f_distclean(${PRO_DIR} "${autotest_projects}")
  endif()
elseif( $ENV{TARGET} STREQUAL "distclean-all" )
  f_process_deps( "${DEPENDS_PROJECTS}" res_code )

  message( "<${PROJECT_NAME}>")
  f_distclean(${PRO_DIR} "${QMAKE_PROJECTS}")
  f_distclean(${PRO_DIR} "${MAKE_PROJECTS}")

  set(autotest_projects)
  f_autotest_list( ${PRO_DIR} "${QMAKE_PROJECTS}" autotest_projects )

  if( autotest_projects )
    message( "<${PROJECT_NAME}> -- autotests")
    f_distclean(${PRO_DIR} "${autotest_projects}")
  endif()
elseif( $ENV{TARGET} STREQUAL "remove-build" )
  message( "REMOVE-BUILD ${BUILD_DIR}")
  execute_process(
      COMMAND rm -rf $ENV{BUILD_DIR}
      #WORKING_DIRECTORY "$ENV{SRC}/${dep}"
      #RESULT_VARIABLE res
      #OUTPUT_VARIABLE out
      #ERROR_VARIABLE  err
    )

elseif( $ENV{TARGET} STREQUAL "fullclean" )
  message( "<${PROJECT_NAME}>")
  execute_process(
      COMMAND bash ${PRO_DIR}/build.sh distclean
      #WORKING_DIRECTORY "$ENV{SRC}/${dep}"
      #RESULT_VARIABLE res
      #OUTPUT_VARIABLE out
      #ERROR_VARIABLE  err
    )

  execute_process(
      COMMAND bash $ENV{SRC}/build_tools/fullclean.sh
      #WORKING_DIRECTORY "$ENV{SRC}/${dep}"
      #RESULT_VARIABLE res
      #OUTPUT_VARIABLE out
      #ERROR_VARIABLE  err
    )

elseif( $ENV{TARGET} STREQUAL "fullclean-all" )
  set( res_code 0 )
  f_process_deps( "${DEPENDS_PROJECTS}" res_code )

  execute_process(
      COMMAND bash ${PRO_DIR}/build.sh fullclean
      #WORKING_DIRECTORY "$ENV{SRC}/${dep}"
      #RESULT_VARIABLE res
      #OUTPUT_VARIABLE out
      #ERROR_VARIABLE  err
    )

  execute_process(
      COMMAND bash ${PRO_DIR}/build.sh remove-build
      #WORKING_DIRECTORY "$ENV{SRC}/${dep}"
      #RESULT_VARIABLE res
      #OUTPUT_VARIABLE out
      #ERROR_VARIABLE  err
    )

elseif( $ENV{TARGET} STREQUAL "--help" )
  f_print_help()
else()
  f_print_help()  
endif()
