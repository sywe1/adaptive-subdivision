find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
list(APPEND LIBRARIES ${GLUT_LIBRARIES})

if(NOT GLUT_FOUND)
	message(WARNING "GLUT not found")
endif()
