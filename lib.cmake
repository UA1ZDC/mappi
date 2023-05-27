if( WIN32 )
  set(APP_EXT ".exe")
else()
  set(APP_EXT "")
endif()

# --- find make app
find_program(
  make_app
  NAMES make mingw32-make.exe
)

if( WIN32 )
  set(NPROC $ENV{NUMBER_OF_PROCESSORS})
else()
  execute_process (
      COMMAND nproc
      OUTPUT_VARIABLE NPROC
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )  
endif()

# --- get autotests projects
FUNCTION( f_autotest_list pro_dir projects result )
  set( res_arr )
  foreach( name ${projects} )
    set( path "${pro_dir}/${name}/autotest" )
    FILE(TO_NATIVE_PATH ${path} native_path)
  
    if ( EXISTS "${native_path}" AND IS_DIRECTORY "${native_path}" )
        execute_process(
          COMMAND qmake
          WORKING_DIRECTORY ${native_path}
          RESULT_VARIABLE res
          OUTPUT_VARIABLE out
          ERROR_VARIABLE  err
        )
        if( ${res} EQUAL 0 )
            set( res_arr ${res_arr} ${name}/autotest )
        endif()
    endif()
  endforeach()
  set( ${result} ${res_arr} PARENT_SCOPE )
ENDFUNCTION()

# -- f_run_make
FUNCTION( f_run_make target path res_code output )
  execute_process (
    COMMAND ${make_app} -j${NPROC} ${target}
    WORKING_DIRECTORY ${path}
    RESULT_VARIABLE res
    OUTPUT_VARIABLE out
    ERROR_VARIABLE  err
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
  )

  set( ${res_code} ${res} PARENT_SCOPE )
  set( ${output}  "${out}\n${err}" PARENT_SCOPE )
ENDFUNCTION()

# -- f_qmake_make_install
FUNCTION( f_qmake_make_install pro_dir pro_names )
  foreach( name ${pro_names} )
    set( path ${pro_dir}/${name} )
    FILE(TO_NATIVE_PATH ${path} native_path)

    message( "[Q] ${name}")
    execute_process(
      COMMAND qmake
      WORKING_DIRECTORY ${native_path}
      RESULT_VARIABLE res
      OUTPUT_VARIABLE out
      ERROR_VARIABLE  err
    )
    if( NOT ${res} EQUAL 0 )
      f_error_out( "qmake" "${out}\n${err}" )
      return()
    endif()

    message( "[B] ${name}")
    set( out )
    set( err )
    f_run_make("" ${native_path} res output )
    if( NOT ${res} EQUAL 0 )
      f_error_out( "make" "${output}" )
      return()
    endif()

    message( "[I] ${name}")
    f_run_make("install" ${native_path} res o)

  endforeach()
ENDFUNCTION()

# -- f_run_utest
FUNCTION( f_run_utest pro_dir pro_names )
  foreach( name ${pro_names} )
    # path to project
    set( path "${pro_dir}/${name}" )
    FILE(TO_NATIVE_PATH ${path} native_path)
    # path to exec file
    set( app "${pro_dir}/${name}/test${APP_EXT}" )
    FILE(TO_NATIVE_PATH ${app} app_native_path)
    
    if( EXISTS "${app_native_path}" )
      message( "[run] ${name} -----------------------------")
      execute_process(
        COMMAND ${app_native_path}
        WORKING_DIRECTORY ${native_path}
        RESULT_VARIABLE res
        ERROR_VARIABLE  err
      )

      if( NOT ${res} EQUAL 0 )
        message( SEND_ERROR ${err} )
      endif()
    else()
      message( "[not found] ${name} -----------------------")
    endif()

  endforeach()
ENDFUNCTION()

# -- f_qmake_make
FUNCTION( f_qmake_make pro_dir pro_names )
  foreach( name ${pro_names} )
    set( path ${pro_dir}/${name} )
    FILE( TO_NATIVE_PATH ${path} native_path )

    message( "[Q] ${name}" )
    execute_process(
      COMMAND qmake
      WORKING_DIRECTORY ${native_path}
      OUTPUT_VARIABLE out
      RESULT_VARIABLE res
      RESULT_VARIABLE err
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
    )
    if( NOT ${res} EQUAL 0 )
      f_error_out( "qmake" "${out}${err}" )
      return()
    endif()

    message( "[B] ${name}" )
    f_run_make( "" ${native_path} res_code output )
    if( NOT ${res_code} EQUAL 0 )
      f_error_out( "make" "${output}" )
      return()
    endif()

  endforeach()
ENDFUNCTION()

# -- f_make_install
FUNCTION( f_make_install pro_dir pro_names )
  foreach( name ${pro_names} )
    set( path ${pro_dir}/${name} )
    FILE(TO_NATIVE_PATH ${path} native_path)

    message( "[B] ${name}")
    f_run_make("" ${native_path} res output )
    if( NOT ${res} EQUAL 0 )
      f_error_out( "make" "${output}" )
      return()
    endif()

    message( "[I] ${name}")
    f_run_make("install" ${native_path} res output )

  endforeach()
ENDFUNCTION()

# --- f_clean
FUNCTION( f_clean pro_dir pro_names)
  foreach( name ${pro_names} )
    set( path ${pro_dir}/${name} )
    FILE(TO_NATIVE_PATH ${path} native_path)

    message( "[CLEAN] ${name}" )
    f_run_make( "clean" ${native_path} res output )
  endforeach ()
ENDFUNCTION()

# --- f_uninstall
FUNCTION( f_uninstall pro_dir pro_names)
  foreach( name ${pro_names} )
    set( path ${pro_dir}/${name} )
    FILE( TO_NATIVE_PATH ${path} native_path )

    message( "[UNINSTALL] ${name}" )
    f_run_make( "uninstall" ${native_path} res output )
  endforeach ()
ENDFUNCTION()

# --- f_distclean
FUNCTION( f_distclean pro_dir pro_names)
  foreach( name ${pro_names} )
    set( path ${pro_dir}/${name} )
    FILE( TO_NATIVE_PATH ${path} native_path )

    message( "[DISTCLEAN] ${name}")
    f_run_make( "distclean" ${native_path} res output )
  endforeach ()
ENDFUNCTION()

# --- f_process_deps
FUNCTION( f_process_deps dep_dirs res_code )
  foreach( dep ${dep_dirs} )
    execute_process(
      COMMAND cmake -P build.cmake
      WORKING_DIRECTORY "$ENV{SRC}/${dep}"
      RESULT_VARIABLE res
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
    )
    
    if( NOT ${res} EQUAL 0 )
      set( ${res_code} "1" PARENT_SCOPE )
      f_error_out("process_deps" "build error")
      return()
    endif()
  endforeach()
ENDFUNCTION()

# --- f_error_out
FUNCTION( f_error_out title txt )
  message( STATUS "ERROR - ${title} -----" )
  message( "${txt}" )
  message( "---------------------------------------------------------------\n" )
  message( FATAL_ERROR "" )
ENDFUNCTION()

