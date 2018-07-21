/*
 *   Copyright (C) 2017-2018  Ruben Undheim <ruben.undheim@gmail.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>

int verilog_preprocess(char *buf, int n, FILE *filein)
{
    //int j = fread(buf,1,n,filein);
    //return j;
    int i;
    char tmp;
    int produced;
    int mode;
    char last;
    produced = 0;
    mode = 0;
    for(i=0;i<n ;i++) {
        tmp = getc(filein);
        if (tmp == EOF) break;
        if (mode == 0) {
            if (tmp == '/') {
                mode = 1;
                last = tmp;
            }
        }
        else if (mode == 1) {
            if (tmp == '/') {
                mode = 2;
            }
            else if (tmp == '*') {
                mode = 3;
            }
            else {
                mode = 0;
                *(buf++) = last;
                produced++;
            }
        }
        if(mode == 0) {
            *(buf++) = tmp;
            produced++;
        }
        else {
            if (tmp == '\n') {
                *(buf++) = tmp;
                produced++;
            }
        }

        if (mode == 2) {
            if (tmp == '\n') { 
                mode = 0;
            }
        }
        else if (mode == 3) {
            if (tmp == '*') { 
                mode = 4;
            }
        }
        else if (mode == 4) {
            if (tmp == '/') { 
                mode = 0;
            }
            else {
                mode = 3;
            }
        }
    }
    return produced;
}
