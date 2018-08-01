cmake_minimum_required (VERSION 2.8.5)
project (netlist-analyzer)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules CACHE STRING "CMake module path")


FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)
FIND_PACKAGE(TCL REQUIRED)
FIND_PACKAGE(Readline REQUIRED)

#INCLUDE(CheckIncludeFile)


INCLUDE_DIRECTORIES(${Readline_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${TCL_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/input/verilog)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/input/liberty)


ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_BINARY_DIR}/.bisondep/input/verilog
  OUTPUT ${CMAKE_BINARY_DIR}/.bisondep/input/liberty
  COMMAND mkdir -p ${CMAKE_BINARY_DIR}/.bisondep/input/verilog ${CMAKE_BINARY_DIR}/.bisondep/input/liberty
  COMMENT "Creating dirs"
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_BINARY_DIR}/.bisondep/input/verilog/verilog_parser.tab.cc
         ${CMAKE_BINARY_DIR}/.bisondep/input/verilog/verilog_parser.tab.h
  DEPENDS ${CMAKE_BINARY_DIR}/.bisondep/input/verilog
          ${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.y
  COMMAND ${BISON_EXECUTABLE} -o ${CMAKE_BINARY_DIR}/.bisondep/input/verilog/verilog_parser.tab.cc
                              --defines=${CMAKE_BINARY_DIR}/.bisondep/input/verilog/verilog_parser.tab.h
       ${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.y
  COMMENT "Running bison to make verilog parser"
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_BINARY_DIR}/.bisondep/input/verilog/lex.verilog_parser_yy.cc
  DEPENDS ${CMAKE_BINARY_DIR}/.bisondep/input/verilog/verilog_parser.tab.h
          ${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.l
  COMMAND ${FLEX_EXECUTABLE} --outfile=${CMAKE_BINARY_DIR}/.bisondep/input/verilog/lex.verilog_parser_yy.cc
       ${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.l
  COMMENT "Running flex to make verilog lexer"
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_BINARY_DIR}/.bisondep/input/liberty/liberty_parser.tab.cc
         ${CMAKE_BINARY_DIR}/.bisondep/input/liberty/liberty_parser.tab.h
  DEPENDS ${CMAKE_BINARY_DIR}/.bisondep/input/liberty
          ${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.y
  COMMAND ${BISON_EXECUTABLE} -o ${CMAKE_BINARY_DIR}/.bisondep/input/liberty/liberty_parser.tab.cc
                              --defines=${CMAKE_BINARY_DIR}/.bisondep/input/liberty/liberty_parser.tab.h
       ${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.y
  COMMENT "Running bison to make liberty parser"
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_BINARY_DIR}/.bisondep/input/liberty/lex.liberty_parser_yy.cc
  DEPENDS ${CMAKE_BINARY_DIR}/.bisondep/input/liberty/liberty_parser.tab.h
          ${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.l
  COMMAND ${FLEX_EXECUTABLE} --outfile=${CMAKE_BINARY_DIR}/.bisondep/input/liberty/lex.liberty_parser_yy.cc
       ${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.l
  COMMENT "Running flex to make liberty lexer"
)

add_library(main_dependencies STATIC timing/timing_path.cc timing/timing_node.cc input/verilog/verilog_connection.cc input/verilog/verilog_net.cc input/verilog/verilog_instance.cc input/verilog/verilog_design.cc input/verilog/verilog_assignment.cc input/verilog/verilog_module.cc input/verilog/verilog_preprocess.cc input/verilog/connection_builder.cc input/liberty/liberty_cell.cc input/liberty/lutable.cc input/liberty/liberty.cc input/liberty/liberty_pin.cc input/liberty/lutable_numbers.cc constraints/constraint_clock.cc constraints/constraints.cc program_state.cc ${CMAKE_BINARY_DIR}/.bisondep/input/verilog/verilog_parser.tab.cc ${CMAKE_BINARY_DIR}/.bisondep/input/liberty/liberty_parser.tab.cc ${CMAKE_BINARY_DIR}/.bisondep/input/liberty/lex.liberty_parser_yy.cc ${CMAKE_BINARY_DIR}/.bisondep/input/verilog/lex.verilog_parser_yy.cc)

add_executable(netan tclembed/interpreter.cc main.cc)

target_link_libraries(netan ${TCL_LIBRARY})
target_link_libraries(netan ${Readline_LIBRARY})
target_link_libraries(netan main_dependencies)


add_subdirectory(tests)
