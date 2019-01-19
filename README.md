netlist-analyzer
================


Import a verilog netlist to a model in memory and trace signals.

It is a lightweight console program, and has a command line TCL interface with elegant history saved between sessions.


Example
-------

```
netlist_analyzer> read_verilog netlist.v
Done
netlist_analyzer> read_liberty mylibrary.lib
Done
netlist_analyzer> set_design_top MyTop
The module MyTop has been set as top module
netlist_analyzer> link_design
Successfully linked design!
netlist_analyzer> help

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
  report_timing
  read_verilog
  read_liberty
  report_clock
  restart
  set_design_top
  source
  traverse_input
  quit
```

Pushing **Ctrl-R** enables searching through all old history (also from previous sessions):
```
netlist_analyzer>
(reverse-i-search)`read_v': read_verilog .zshrc
```
This is accomplished via the readline library.

You can also source a file with commands:
```
netlist_analyzer> source run.tcl
...
```

License
-------

This program is GPL-3+ licensed:

```
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
```
