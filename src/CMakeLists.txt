add_definitions(
    -D_FILE_OFFSET_BITS=64
    -D_LARGEFILE_SOURCE
    -DRARDLL
    -DQTRAR_LIBRARY
)

if (NOT WIN32)
    add_definitions(-DRAR_SMP)
endif ()

# set all include directories for in and out of source builds
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/unrar
	${CMAKE_CURRENT_BINARY_DIR}
)

set(CMAKE_AUTOMOC ON)

set(UNRAR_SRC
    "unrar/rar.cpp"
    "unrar/strlist.cpp"
    "unrar/strfn.cpp"
    "unrar/pathfn.cpp"
    "unrar/smallfn.cpp"
    "unrar/global.cpp"
    "unrar/file.cpp"
    "unrar/filefn.cpp"
    "unrar/filcreat.cpp"
    "unrar/archive.cpp"
    "unrar/arcread.cpp"
    "unrar/unicode.cpp"
    "unrar/system.cpp"
    "unrar/isnt.cpp"
    "unrar/crypt.cpp"
    "unrar/crc.cpp"
    "unrar/rawread.cpp"
    "unrar/encname.cpp"
    "unrar/resource.cpp"
    "unrar/match.cpp"
    "unrar/timefn.cpp"
    "unrar/rdwrfn.cpp"
    "unrar/consio.cpp"
    "unrar/options.cpp"
    "unrar/errhnd.cpp"
    "unrar/rarvm.cpp"
    "unrar/secpassword.cpp"
    "unrar/rijndael.cpp"
    "unrar/getbits.cpp"
    "unrar/sha1.cpp"
    "unrar/sha256.cpp"
    "unrar/blake2s.cpp"
    "unrar/hash.cpp"
    "unrar/extinfo.cpp"
    "unrar/extract.cpp"
    "unrar/volume.cpp"
    "unrar/list.cpp"
    "unrar/find.cpp"
    "unrar/unpack.cpp"
    "unrar/headers.cpp"
    "unrar/threadpool.cpp"
    "unrar/rs16.cpp"
    "unrar/cmddata.cpp"
    "unrar/ui.cpp"
    "unrar/filestr.cpp"
    "unrar/scantree.cpp"
    "unrar/dll.cpp"
    "unrar/qopen.cpp"
)

file(GLOB SRCS "*.cpp")
file(GLOB PUBLIC_HEADERS "*.h")

set(SRCS ${SRCS} ${UNRAR_SRC})

add_library(qtrarobjs STATIC ${SRCS})
target_link_libraries(qtrarobjs Qt::Core ${CMAKE_THREADS_LIBS_INIT})

add_library(qtrar ${SRCS})
set_target_properties(qtrar PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(qtrar Qt::Core ${CMAKE_THREADS_LIBS_INIT})

if(BUILD_SHARED_LIBS)
    install(FILES ${PUBLIC_HEADERS} DESTINATION include/qtrar)
    install(TARGETS qtrar LIBRARY DESTINATION ${LIB_DESTINATION} ARCHIVE DESTINATION ${LIB_DESTINATION} RUNTIME DESTINATION ${LIB_DESTINATION})
endif(BUILD_SHARED_LIBS)