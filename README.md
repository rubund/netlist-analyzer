netlist-analyzer
================


Import a verilog netlist to a model in memory and trace signals.

It is a lightweight console program, and has a command line TCL interface with elegant history saved between sessions.


Example
-------

```
netlist\_analyzer> read_verilog netlist.v
Done
netlist\_analyzer> read_liberty mylibrary.lib
Done
netlist\_analyzer> set_design_top MyTop
The module MyTop has been set as top module
netlist\_analyzer> link_design
Successfully linked design!
netlist\_analyzer> help

Available commands:
  all_registers
  create_clock
  exit
  get_cell
  get_fanin
  get_fanout
  get_pin
  help
  link_design
  list_libs
  report\_timing
  read\_verilog
  read\_liberty
  report\_clock
  restart
  set\_design\_top
  source
  traverse\_input
  quit
```

Pushing **Ctrl-R** enables searching through all old histories (also from previous sessions):
```
netlist\_analyzer>
(reverse-i-search)`read_v': read_verilog .zshrc

```
This is accomplished via the readline library.

You can also source a file with commands:
```
netlist\_analyzer> source run.tcl
...
```
