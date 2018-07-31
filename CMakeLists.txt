cmake_minimum_required (VERSION 2.8.5)
project (netlist-analyzer)


INCLUDE(CheckIncludeFiles)


INCLUDE_DIRECTORIES(/usr/include/tcl)

set(BISON_EXECUTABLE "bison")
set(FLEX_EXECUTABLE "flex")

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.tab.cc
         ${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.tab.h
  COMMAND ${BISON_EXECUTABLE} -o ${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.tab.cc
                              --defines=${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.tab.h
       ${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.y
  COMMENT "Running bison to make verilog parser"
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_SOURCE_DIR}/input/verilog/lex.verilog_parser_yy.cc
  DEPENDS ${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.tab.h
  COMMAND ${FLEX_EXECUTABLE} --outfile=${CMAKE_SOURCE_DIR}/input/verilog/lex.verilog_parser_yy.cc
       ${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.l
  COMMENT "Running flex to make verilog lexer"
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.tab.cc
         ${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.tab.h
  COMMAND ${BISON_EXECUTABLE} -o ${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.tab.cc
                              --defines=${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.tab.h
       ${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.y
  COMMENT "Running bison to make liberty parser"
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_SOURCE_DIR}/input/liberty/lex.liberty_parser_yy.cc
  DEPENDS ${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.tab.h
  COMMAND ${FLEX_EXECUTABLE} --outfile=${CMAKE_SOURCE_DIR}/input/liberty/lex.liberty_parser_yy.cc
       ${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.l
  COMMENT "Running flex to make liberty lexer"
)


add_executable(netan timing/timing_path.cc timing/timing_node.cc input/verilog/verilog_connection.cc input/verilog/verilog_net.cc input/verilog/verilog_instance.cc input/verilog/verilog_design.cc input/verilog/verilog_assignment.cc input/verilog/verilog_module.cc input/verilog/verilog_preprocess.cc input/verilog/connection_builder.cc input/liberty/liberty_cell.cc input/liberty/lutable.cc input/liberty/liberty.cc input/liberty/liberty_pin.cc input/liberty/lutable_numbers.cc tclembed/interpreter.cc constraints/constraint_clock.cc constraints/constraints.cc program_state.cc main.cc ${CMAKE_SOURCE_DIR}/input/verilog/verilog_parser.tab.cc ${CMAKE_SOURCE_DIR}/input/liberty/liberty_parser.tab.cc ${CMAKE_SOURCE_DIR}/input/liberty/lex.liberty_parser_yy.cc ${CMAKE_SOURCE_DIR}/input/verilog/lex.verilog_parser_yy.cc)

target_link_libraries(netan tcl)
target_link_libraries(netan readline)
