if(EIGEN3_INCLUDE_DIRS)
	set(EIGEN3_FOUND TRUE)
else(EIGEN3_INCLUDE_DIRS)
	find_path(EIGEN3_INCLUDE_DIR
		NAMES
			eigen3/signature_of_eigen3_matrix_library
		PATHS
			/usr/local/include
			/usr/include
		)

	set (EIGEN3_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR})

	if(EIGEN3_INCLUDE_DIRS)
		set(EIGEN3_FOUND TRUE)
	endif(EIGEN3_INCLUDE_DIRS)

	if(EIGEN3_FOUND)
		message(STATUS "Find Eigen3: ${EIGEN3_INCLUDE_DIRS}")
	endif(EIGEN3_FOUND)
endif(EIGEN3_INCLUDE_DIRS)
