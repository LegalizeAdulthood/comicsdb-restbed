include(FindPackageHandleStandardArgs)

find_library(RESTBED_LIB NAMES restbed restbed-shared)
find_file(RESTBED_INCLUDE restbed)

find_package_handle_standard_args(restbed DEFAULT_MSG RESTBED_LIB RESTBED_INCLUDE)

if(restbed_FOUND)
    get_filename_component(RESTBED_LIB_DIR ${RESTBED_LIB} DIRECTORY)
    get_filename_component(RESTBED_INCLUDE_DIR ${RESTBED_INCLUDE} DIRECTORY)
    add_library(restbed::restbed SHARED IMPORTED)
    set_property(TARGET restbed::restbed PROPERTY IMPORTED_LOCATION ${RESTBED_LIB})
    if(WIN32)
        set_property(TARGET restbed::restbed PROPERTY IMPORTED_IMPLIB ${RESTBED_LIB})
    endif()
    target_include_directories(restbed::restbed INTERFACE ${RESTBED_INCLUDE_DIR})
endif()
