aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCFILES)
add_library(chrgfx SHARED ${SRCFILES})

include(GNUInstallDirs)

set_target_properties(chrgfx PROPERTIES SOVERSION 1)

set(HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/global.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/chrgfx.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/builtin_defs.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/chrbank.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/conv_chr.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/conv_col.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/conv_pal.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/gfxdef.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/png_chunk.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/png_render.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/types.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/utils.hpp
)

set_target_properties(chrgfx PROPERTIES PUBLIC_HEADER "${HEADERS}")

find_library(PNG_LIB png)
if(NOT PNG_LIB)
  message(FATAL_ERROR "libpng not found")
endif()

check_include_files("png++/png.hpp" PNGPP_H)
if(NOT PNGPP_H)
  message(FATAL_ERROR "png++ not found")
endif()

target_compile_features(chrgfx PUBLIC cxx_std_11)
target_link_libraries(chrgfx png)

install(TARGETS chrgfx
    LIBRARY DESTINATION /usr/${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION /usr/${CMAKE_INSTALL_INCLUDEDIR}/chrgfx)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../gfxdef/gfxdefs
    DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/chrgfx)
