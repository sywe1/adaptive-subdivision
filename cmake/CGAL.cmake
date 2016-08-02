find_package(CGAL REQUIRED)
include_directories(${CGAL_INCLUDE_DIRS})
link_directories(${CGAL_LIBRARY_DIRS})
add_definitions(${CGAL_DEFINITIONS})
list(APPEND LIBRARIES ${CGAL_LIBRARIES})

if(NOT CGAL_FOUND)
	message(WARNING "CGAL not found")
endif()
