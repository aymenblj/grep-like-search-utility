function(add_doxygen_target)
  find_package(Doxygen REQUIRED)

  if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs")

    set(DOXYGEN_INPUT_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/src
      ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    )

    set(DOXYGEN_STRIP_FROM_PATH
      ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Create Doxyfile using configure_file (optional)
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    if(EXISTS ${DOXYFILE_IN})
      # Let user provide a template Doxyfile.in
      list(JOIN DOXYGEN_INPUT_DIRS " " DOXYGEN_INPUT_PATHS)
      configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
    else()
      # Or fallback to a very basic in-place file
      file(WRITE ${DOXYFILE_OUT}
"PROJECT_NAME = Grep-like Search Utility
OUTPUT_DIRECTORY = ${DOXYGEN_OUTPUT_DIR}
INPUT = ${DOXYGEN_INPUT_DIRS}
FILE_PATTERNS = *.h *.hpp *.cpp *.c *.md
RECURSIVE = YES
EXTENSION_MAPPING = md=markdown
GENERATE_HTML = YES
GENERATE_LATEX = NO
STRIP_FROM_PATH = ${DOXYGEN_STRIP_FROM_PATH}
USE_MDFILE_AS_MAINPAGE = README.md
")
    endif()

    add_custom_command(
      OUTPUT ${DOXYGEN_OUTPUT_DIR}/html/index.html
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM
    )

    add_custom_target(doc ALL DEPENDS ${DOXYGEN_OUTPUT_DIR}/html/index.html)
  endif()
endfunction()
