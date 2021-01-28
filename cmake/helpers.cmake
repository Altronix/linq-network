function(import_library_shared ARG_LIB ARG_ID ARG_PATH)
  set(VAR_INC "${CMAKE_INSTALL_PREFIX}/include")
  if(ARG_PATH)
    get_filename_component(LIB_DIR ${ARG_PATH} DIRECTORY)
    list(APPEND VAR_INC "${LIB_DIR}/../include")
    set(VAR_LOC "${ARG_PATH}")
  else()
    set(VAR_LIB "${CMAKE_SHARED_LIBRARY_PREFIX}${ARG_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    if(NOT MSVC)
      set(VAR_LOC "${CMAKE_INSTALL_PREFIX}/lib/${VAR_LIB}")
    else()
      set(VAR_LOC "${CMAKE_INSTALL_PREFIX}/bin/${VAR_LIB}")
    endif()
  endif()
  add_library(${ARG_ID} SHARED IMPORTED)
  set_target_properties(${ARG_ID} PROPERTIES
	  IMPORTED_LOCATION ${VAR_LOC}
	  INTERFACE_INCLUDE_DIRECTORIES "${VAR_INC}")
endfunction()

function(import_library_static ARG_LIB ARG_ID ARG_PATH)
  set(VAR_INC "${CMAKE_INSTALL_PREFIX}/include")
  if(ARG_PATH)
    get_filename_component(LIB_DIR ${ARG_PATH} DIRECTORY)
    list(APPEND VAR_INC "${LIB_DIR}/../include")
    set(VAR_LOC "${ARG_PATH}")
  else()
    set(VAR_LIB "${CMAKE_STATIC_LIBRARY_PREFIX}${ARG_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(VAR_LOC "${CMAKE_INSTALL_PREFIX}/lib/${VAR_LIB}")
  endif()
  add_library(${ARG_ID} STATIC IMPORTED)
  set_target_properties(${ARG_ID} PROPERTIES
	  IMPORTED_LOCATION ${VAR_LOC}
	  INTERFACE_INCLUDE_DIRECTORIES "${VAR_INC}")
endfunction()

function (make_absolute path result)
  if(NOT IS_ABSOLUTE "${path}")
    get_filename_component(TMP "${path}" ABSOLUTE)
    set("${result}" "${TMP}" PARENT_SCOPE)
  endif()
endfunction()

function (resolve_install_dir loc result)
  set(TMP "${CMAKE_INSTALL_PREFIX}")
  if(NOT IS_ABSOLUTE "${TMP}")
    make_absolute("${TMP}" TMP)
  endif()
  set(TMP "${TMP}/${loc}")
  message(STATUS "RESOLVE ${loc} => ${TMP}")
  set("${result}" "${TMP}" PARENT_SCOPE)
endfunction()

function (copy_command src dst)
  add_custom_command(
    OUTPUT "${dst}"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${src}"
    "${dst}"
  )
endfunction()

function (check_submodule root test_file)
  get_filename_component(REPO "${test_file}" DIRECTORY)
  if(NOT EXISTS "${test_file}")
    message(STATUS "${test_file} not found! cloneing submodule ${REPO}")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init "${REPO}"
      WORKING_DIRECTORY ${root}
      RESULT_VARIABLE R
    )
    message(STATUS "GIT SUBMODULE UPDATE RESULT: ${R}")
  else()
    message(STATUS "${test_file} found!")
  endif()
endfunction()

function (check_extract dl test_file)
  get_filename_component(REPO "${test_file}" DIRECTORY)
  if(NOT EXISTS "${test_file}")
    message(STATUS "${test_file} not found! extracting ${REPO}")
    execute_process(
    	COMMAND tar -xzvf "${dl}" -C "${EXTERNAL_DIR}"
    	WORKING_DIRECTORY ${ROOT_DIR}
    	RESULT_VARIABLE EXTRACT_CMOCKA_RESULT)
    message(STATUS "EXTRACT_CMOCKA_RESULT: ${EXTRACT_CMOCKA_RESULT}")
    message(STATUS "GIT SUBMODULE UPDATE RESULT: ${R}")
  else()
    message(STATUS "${test_file} found!")
  endif()
endfunction()
