cmake_minimum_required ( VERSION 3.20 )

project (MPI_KMP_TEST)

set ( PRJ_SRC_LIST )
set ( PRJ_LIBRARIES )
set ( PRJ_INCLUDE_DIRS )

find_package ( MPI )

message ( STATUS  "MPI_FOUND=${MPI_FOUND}" )
message ( STATUS  "MPI_CXX_INCLUDE_DIRS=${MPI_CXX_INCLUDE_DIRS}" )
message ( STATUS  "MPI_LIBRARIES=${MPI_LIBRARIES}" )
if ( MPI_FOUND )
	list ( APPEND PRJ_INCLUDE_DIRS ${MPI_CXX_INCLUDE_DIRS} )
	list ( APPEND PRJ_LIBRARIES ${MPI_LIBRARIES} )
endif ()


set ( PRJ_SRC_LIST_1 kmp.c )
set ( PRJ_SRC_LIST_2 gen_ped.c )

add_executable (kmp ${PRJ_SRC_LIST_1} )

add_executable (gen_ped ${PRJ_SRC_LIST_2} )

target_include_directories ( kmp
	PRIVATE
		${PRJ_INCLUDE_DIRS}
)

target_compile_features ( kmp
	PUBLIC 
		cxx_std_20
)

target_link_libraries ( kmp
    PRIVATE 
        ${PRJ_LIBRARIES}
)

target_include_directories ( gen_ped
	PRIVATE
		${PRJ_INCLUDE_DIRS}
)

target_compile_features ( gen_ped
	PUBLIC 
		cxx_std_20
)

target_link_libraries ( gen_ped
    PRIVATE 
        ${PRJ_LIBRARIES}
)

