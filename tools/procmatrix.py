#! /usr/bin/env python
#
# Keyboard Upgrade -- Firmware for homebrew computer keyboard controllers.
# Copyright (C) 2009  Robert Homann
#
# This file is part of the Keyboard Upgrade package.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the Keyboard Upgrade package; if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301  USA
#

import getopt
import sys
import re
import string

class Defaults:
  keymap_varname="standard_stored_keymap"
  usbkeycode_file="usbkeycodes.h"

class GenOptions:
  def __init__(this):
    this.kbdef=None
    this.progmem_attribute=" PROGMEM"
    this.progmem_prefix="prog_"
    this.keymap_name=None
    this.keymap_varname=Defaults.keymap_varname
    this.matfile_permutations=True

def error(line):
  sys.stderr.write('Error: '+line+'\n')
  sys.exit(1)

def new_file(filename,fn,args):
  if filename == None: return None
  if filename == "-": outfile=sys.stdout
  else:               outfile=open(filename,"w")
  retval=fn(outfile,args)
  if filename != "-": outfile.close()
  return retval

def read_keycode_enumeration(file):
  ok=False
  while True:
    line=file.readline()
    if line == "": break
    elif line == "enum keycodes {\n":
      ok=True
      break

  if not ok: error("Enumeration not found in "+file.name)

  # inside enumeration
  names={}
  num_of_keys=0
  num_of_mods=0
  while True:
    line=file.readline()
    if line == "": error("Unexpected end of file.")
    elif line == "};\n": return [names,num_of_keys,num_of_mods]

    line=line.lstrip().rstrip()
    if not re.match("KEY_|MOD_",line): continue
    line=re.split("[,= \t]",line,1)[0]
    names[line]=None
    if re.match("KEY_",line): num_of_keys+=1
    else:                     num_of_mods+=1

class Key:
  def __init__(this,row,name):
    this.row=row
    this.col=None
    this.name=name

  def __repr__(this):
    return 'Row '+str(this.row)+'/Column '+str(this.col)

  def compare(this,other):
    if this.row != other.row: return this.row-other.row
    return this.col-other.col

  def compkey(this): return this.row*100+this.col

def canonic_keycode_name(name):
  return name.rstrip('*')

def assert_valid_keycode(code,valid_keycodes,filename):
  temp=canonic_keycode_name(code)
  if filename == None:
    assert temp in valid_keycodes, "Encountered invalid keycode \""+code+"\" ("+temp+")."
  else:
    if temp not in valid_keycodes:
      msg="\""+temp+"\" is not a valid name for a keycode ("
      if temp != code:
        msg+="used in non-canonic form \""+code+"\" "
      msg+="in file "+filename+")."
      error(msg)

def expand_ranges(rangespec):
  pins=[]
  for elem in re.split(',',rangespec):
    rg=re.split('\.\.',elem)
    if re.match('\d+$',rg[0]) == None: error('Not a number: "'+rg[0]+'".')
    if len(rg) == 1:
      # just a single number
      pins.append(int(rg[0]))
    elif len(rg) == 2:
      # real range
      if re.match('\d+$',rg[1]) == None: error('Not a number: "'+rg[1]+'".')

      a=int(rg[0])
      b=int(rg[1])
      if a <= b: s=1
      else:      s=-1
      for i in range(a,b+s,s): pins.append(i)
    else:
      error('Invalid range specification: "'+elem+'".')
  if not pins: error('Empty or invalid permutation: "'+rangespec+'".')
  return pins

def add_swap_to_dict(dict,src,dest):
  if src == 0 or dest == 0: error('Pin numbering starts at 1.')
  if src in dict:
    error('Pin '+str(src)+' has been swapped already (with '+
          str(dict[src])+').')
  dict[src]=dest

class KB:
  def __init__(this,inputfile,valid_keycodes):
    this.inputfile=inputfile
    this.name=None
    this.keys={}
    this.rows=-1
    this.columns=-1
    this.longest_name=0
    this.remappings=None
    this.sparse_mat=None
    this.valid_keycodes=valid_keycodes
    this.max_name_length=20
    this.swap_rows_spec={}
    this.swap_cols_spec={}

  def finalize(this):
    this.check()
    this.rows+=1
    this.columns+=1

  # adapt to 0-based indexing, check for errors
  def fix_permutations(this):
    if this.swap_rows_spec:
      temp={}
      for x in this.swap_rows_spec:
        if x > this.rows or this.swap_rows_spec[x] > this.rows:
          error('Number of rows is '+str(this.rows)+
                ', but swapping of row '+str(x)+' and '+
                str(this.swap_rows_spec[x])+' specified.')
        temp[x-1]=this.swap_rows_spec[x]-1
      this.swap_rows_spec=temp
    if this.swap_cols_spec:
      temp={}
      for x in this.swap_cols_spec:
        if x > this.columns or this.swap_cols_spec[x] > this.columns:
          error('Number of columns is '+str(this.columns)+
                ', but swapping of column '+str(x)+' and '+
                str(this.swap_cols_spec[x])+' specified.')
        temp[x-1]=this.swap_cols_spec[x]-1
      this.swap_cols_spec=temp

  def apply_swaps(this):
    this.fix_permutations()
    if this.swap_rows_spec or this.swap_cols_spec:
      for key in this.keys.values():
        if key.row in this.swap_rows_spec: key.row=this.swap_rows_spec[key.row]
        if key.col in this.swap_cols_spec: key.col=this.swap_cols_spec[key.col]

  def set_name(this,name): this.name=name

  def set_remappings(this,remappings): this.remappings=remappings

  def add_swap_rule(this,rule,add_for_real):
    try:
      what,src,dest=re.split('\s+',rule,2)
    except ValueError as err:
      error('Need two pin specifications in this line:\n'+rule)

    src_pins=expand_ranges(src)
    dest_pins=expand_ranges(dest)
    if len(src_pins) != len(dest_pins):
      error('Source and destination lists do not define the same number of pins:\n  '+str(src_pins)+'\n  '+str(dest_pins))

    if what == 'row':      dict=this.swap_rows_spec
    elif what == 'column': dict=this.swap_cols_spec
    else:
      error('We can swap either a "row" or a "column", but not a "'+what+'".')

    if add_for_real:
      for x,y in zip(src_pins,dest_pins): add_swap_to_dict(dict,x,y)

  def add_row(this): this.rows+=1

  def add_column(this): this.columns+=1

  def add_to_row(this,code):
    assert_valid_keycode(code,this.valid_keycodes,this.inputfile.name)
    if code in this.keys:
      error("Code "+code+" appears in multiple matrix rows.")

    this.keys[code]=Key(this.rows,code)
    if len(code) > this.longest_name: this.longest_name=len(code)

  def add_to_column(this,code):
    assert_valid_keycode(code,this.valid_keycodes,this.inputfile.name)
    if code not in this.keys:
      error("Code "+code+" is defined in a matrix column, but does not appear in any matrix row.")

    if this.keys[code].col != None:
      error("Code "+code+" appears in multiple matrix columns.")

    this.keys[code].col=this.columns

  def check(this):
    for code in this.keys:
      assert_valid_keycode(code,this.valid_keycodes,None)
      key=this.keys[code]
      assert key.row != None, ""
      if key.col == None:
        error("Code "+code+" is defined in a matrix row, but does not appear in any matrix column.")

  def num_of_keys(this): return len(this.keys)

  def num_of_codes(this):
    names={}
    for code in this.keys.keys(): names[canonic_keycode_name(code)]=None
    return len(names)

  def get_sparse_matrix(this):
    if not this.sparse_mat:
      # build the sparse matrix only once
      this.sparse_mat=sorted(this.keys.values(),key=Key.compkey)
      for k in range(1,len(this.sparse_mat)):
        if this.sparse_mat[k-1].col == this.sparse_mat[k].col and this.sparse_mat[k-1].row == this.sparse_mat[k].row:
          error("Duplicate matrix assignment at row "+
                str(this.sparse_mat[k].row+1)+", column "+
                str(this.sparse_mat[k].col+1)+": "+this.sparse_mat[k-1].name+
                " and "+this.sparse_mat[k].name+".")

      if this.remappings:
        for k in this.sparse_mat:
          if k.name in this.remappings: k.name=this.remappings[k.name]
    # return copy
    return this.sparse_mat[:]

def unexpected_eof(what):
  if what: error('Unexpected end of file while reading the '+what+'.')

def read_strip_line(file,what):
  while True:
    line=file.readline()
    if line == '':
      unexpected_eof(what)
      return None
    line=line.lstrip().rstrip()
    if not line or line[0] != '#': return line

def skip_empty_lines(file,what):
  while True:
    line=read_strip_line(file,what)
    if line: return line
    elif line == None: return None

def read_header(file,kbdef,options):
  line=skip_empty_lines(file,'header')
  while True:
    if not line: break

    line=re.split('\W+',line,1)
    if line[0] == 'Device': kbdef.set_name(line[1])
    elif line[0] == 'Swap': kbdef.add_swap_rule(line[1],
                                                options.matfile_permutations)
    else: error('Unknown tag "'+line[0]+'"')

    line=read_strip_line(file,'header')

def parse_matrix(file,valid_keycodes,options):
  kbdef=KB(file,valid_keycodes)
  read_header(file,kbdef,options)

  # read rows
  line=skip_empty_lines(file,'matrix rows')
  while True:
    if not line: break

    codes=re.split('[ \t]+',line)
    if not codes: error('No keycodes defined in row.')

    kbdef.add_row()
    for code in codes:
      kbdef.add_to_row(code)

    line=read_strip_line(file,'matrix rows')

  # read columns
  line=skip_empty_lines(file,'matrix columns')
  while True:
    if not line: break

    codes=re.split('[ \t]+',line)
    if not codes: error('No keycodes defined in column.')

    kbdef.add_column()
    for code in codes:
      kbdef.add_to_column(code)

    line=read_strip_line(file,None)

  line=skip_empty_lines(file,None)
  if line != None:
    error("Matrix "+file.name+" contains garbage after matrix rows:\n"+line)

  kbdef.finalize()
  return kbdef

def parse_perm_file(file,kbdef):
  for line in file:
    line=line.lstrip().rstrip()
    if not line or line[0] == '#': continue
    kbdef.add_swap_rule(line,True)

def show_matrix_layout(kbdef,simple):
  sparse_matrix=kbdef.get_sparse_matrix()

  if simple:
    hfmt=" %2d"
    xfmt="  x"
    free="  ."
  else:
    hfmt=" | %"+str(kbdef.longest_name)+"d"
    xfmt=" | %-"+str(kbdef.longest_name)+"s"
    free=" | "+" "*kbdef.longest_name

  sys.stdout.write("Keyboard matrix:\n  ")
  for x in range(1,kbdef.columns+1): sys.stdout.write(hfmt % x)
  sys.stdout.write("\n")
  if not simple:
    print("---"+("+"+"-"*(kbdef.longest_name+2))*kbdef.columns)

  for y in range(0,kbdef.rows):
    sys.stdout.write("%2d" % (y+1))
    for x in range(0,kbdef.columns):
      if sparse_matrix and sparse_matrix[0].row == y and sparse_matrix[0].col == x:
        if simple: sys.stdout.write(xfmt)
        else:      sys.stdout.write(xfmt % sparse_matrix[0].name)
        sparse_matrix.pop(0)
      else:
        sys.stdout.write(free)
    sys.stdout.write("\n")

def write_common_header(file,options):
  guard=file.name.upper().replace(".","_")
  bvlen=int((options.kbdef.rows*options.kbdef.columns)/8)
  if bvlen*8 < options.kbdef.rows*options.kbdef.columns: bvlen+=1
  if options.kbdef.columns <= 8:
    colstatetype="uint8_t"
    colstateempty="0xff"
  else:
    colstatetype="uint16_t"
    colstateempty="0xffff"
  file.write("""\
#ifndef """+guard+"""
#define """+guard+"""
#include <inttypes.h>

#define NUM_OF_KEYS           """+str(options.kbdef.num_of_keys())+"""
#define NUM_OF_ROWS           """+str(options.kbdef.rows)+"""
#define NUM_OF_COLUMNS        """+str(options.kbdef.columns)+"""
#define MATRIX_BITVECTOR_LEN  """+str(bvlen)+"""
#define KEYMAP_NAME_LENGTH    """+str(options.kbdef.max_name_length)+"""

#define COLUMNSTATE_EMPTY     """+colstateempty+"""

typedef struct
{
  uint8_t mat[NUM_OF_ROWS][NUM_OF_COLUMNS];
} Map;

typedef struct
{
  const char name[KEYMAP_NAME_LENGTH];
  uint8_t codes[NUM_OF_KEYS];
} Storedmap;

typedef """+colstatetype+""" Columnstate;
#endif /* !"""+guard+""" */
""")

def canonic_name_detrash(name):
  name=canonic_keycode_name(name)
  if name == 'KEY_trash': return '0'
  return name

def write_stored_definition(file,options):
  assert options.keymap_name != None
  if(options.kbdef.max_name_length < len(options.keymap_name)):
    error("Keyboard name too long (length is "+
          str(len(options.keymap_name))+", limit is "+
          str(options.kbdef.max_name_length)+" characters).")
  name="','".join(options.keymap_name)
  varname=options.keymap_varname+options.progmem_attribute

  file.write("""\
static const Storedmap """+varname+"""=
{
  {'"""+name+"""'},
  {""")
  sparse_matrix=options.kbdef.get_sparse_matrix()
  file.write(", ".join(map(lambda x: canonic_name_detrash(x.name),
                           sparse_matrix))+"}\n};\n")

def write_expanded_definition(file,options):
  file.write("static const Map keymap"+options.progmem_attribute+"=\n{{\n")
  sparse_matrix=options.kbdef.get_sparse_matrix()
  lines=[]
  for y in range(0,options.kbdef.rows):
    line=[]
    for x in range(0,options.kbdef.columns):
      if sparse_matrix and sparse_matrix[0].row == y and sparse_matrix[0].col == x:
        line.append(canonic_keycode_name(sparse_matrix[0].name))
        sparse_matrix.pop(0)
      else:
        line.append("0")
    lines.append(line)
  file.write("},\n".join(map(lambda x: "  {"+", ".join(x), lines))+"}\n}};\n")

def read_remappings(files,valid_keycodes):
  if not files: return None

  mappings={}
  for filename in files:
    print("Loading mappings from "+filename)
    with open(filename) as file:
      for line in file:
        line=line.lstrip().rstrip()
        if line and line[0] != "#":
          line=re.split("[ \t]+",line)
          if len(line) != 2: error("Invalid mapping: "+str(line)+".")
          if line[0] in mappings:
            error("Key "+line[0]+" has been re-mapped already.")
          assert_valid_keycode(line[0],valid_keycodes,filename)
          assert_valid_keycode(line[1],valid_keycodes,filename)
          mappings[line[0]]=line[1]

  if mappings != {}: return mappings
  return None

def write_matrix_bitvector(file,options):
  sparse_matrix=options.kbdef.get_sparse_matrix()
  bytes=[0]
  bit=0
  for y in range(0,options.kbdef.rows):
    for x in range(0,options.kbdef.columns):
      if sparse_matrix and sparse_matrix[0].row == y and sparse_matrix[0].col == x:
        bytes[-1]|=1
        sparse_matrix.pop(0)
      if bit < 7:
        bytes[-1]<<=1
        bit+=1
      else:
        bytes.append(0)
        bit=0
  if bit == 0: bytes.pop()
  else: bytes[-1]<<=7-bit

  file.write("static const "+options.progmem_prefix+
             "uint8_t matrix_bits[MATRIX_BITVECTOR_LEN]"+
             "=\n{\n  0x%02x"%bytes[0])
  for x in range(1,len(bytes)): file.write(", 0x%02x"%bytes[x])
  file.write("\n};\n")

def usage():
  sys.stderr.write("""\
Read keyboard matrix definition, generate various files.
Usage: """ + sys.argv[0] + """ -d <matrix definition file> [more options]
  -d file  Name of a file containing a keyboard matrix definition.
  -U file  Read keycode names from enumeration stored in the given file
           (default: \""""+Defaults.usbkeycode_file+"""\").
  -m file  Optional key re-mappings to be applied to the selected matrix in
           order to obtain a custom key map. This option can be given multiple
           times for cumulated effect.
  -k file  Write a header file containing some standard definitions describing
           the keyboard.
  -h file  Write C definition of the key map as stored in EEPROM.
  -n str   Short descriptive name of the generated key map (written to the
           definition generated by -h). If not given, it will be taken from the
           matrix definition file.
  -N str   Variable name to use for the key map definition generated by -h. If
           omitted, then the default name \""""+Defaults.keymap_varname+"""\" will be used.
  -H file  Write C definition of the key map expanded as a matrix.
  -c file  Write bit vector that defines the keyboard matrix (used for mapping
           the stored key map to the keyboard's matrix layout).
  -P       Put all generated definitions into RAM, and NOT into program memory
           (for testing only).
  -s       Show keyboard matrix (just the connections).
  -S       Show keyboard matrix (including keycode names).
  -p file  Apply row and column permutations as specified in the given file.
  -x       Do not apply row and column permutations as specified in the matrix
           definition file.
""")
  sys.exit(1)

def main():
  try:
    opts, args=getopt.getopt(sys.argv[1:],"c:d:h:H:k:m:n:N:p:PsSU:x")
  except getopt.GetoptError as err:
    print(err)
    usage()

  options=GenOptions()

  matrix_def_file=None
  mapping_def_files=[]
  header_file_stored=None
  header_file_expanded=None
  decoder_header_file=None
  keyboard_header_file=None
  permutation_file=None
  show_matrix=False
  usbkeycode_file=Defaults.usbkeycode_file
  for o, a in opts:
    if o == "-d":
      matrix_def_file=a
    elif o == "-c":
      decoder_header_file=a
    elif o == "-h":
      header_file_stored=a
    elif o == "-H":
      header_file_expanded=a
    elif o == "-k":
      keyboard_header_file=a
    elif o == "-m":
      mapping_def_files.append(a)
    elif o == "-n":
      options.keymap_name=a
    elif o == "-N":
      options.keymap_varname=a
    elif o == "-p":
      permutation_file=a
    elif o == "-P":
      options.progmem_attribute=""
      options.progmem_prefix=""
    elif o == "-s":
      show_matrix=True
      simple_matrix=True
    elif o == "-S":
      show_matrix=True
      simple_matrix=False
    elif o == "-U":
      usbkeycode_file=a
    elif o == "-x":
      options.matfile_permutations=False
    else:
      assert False, "unhandled option "+o

  if not matrix_def_file: usage()

  with open(usbkeycode_file) as file:
    valid_keycodes, numkeys, nummods=read_keycode_enumeration(file)
    print("Loaded "+str(numkeys)+" keys and "+str(nummods)+
          " modifiers from \""+usbkeycode_file+"\".")

  with open(matrix_def_file) as matrix_file:
    kbdef=parse_matrix(matrix_file,valid_keycodes,options)
    options.kbdef=kbdef

  if permutation_file:
    with open(permutation_file) as file: parse_perm_file(file,kbdef)

  kbdef.apply_swaps()

  print(kbdef.name+": "+str(kbdef.num_of_codes())+" unique keycodes on "+
        str(kbdef.num_of_keys())+" keys in a "+str(kbdef.rows)+"x"+
        str(kbdef.columns)+" matrix.")

  if not options.keymap_name: options.keymap_name=kbdef.name

  kbdef.set_remappings(read_remappings(mapping_def_files,valid_keycodes))

  if show_matrix: show_matrix_layout(kbdef,simple_matrix)

  new_file(keyboard_header_file,write_common_header,options)
  new_file(decoder_header_file,write_matrix_bitvector,options)
  new_file(header_file_stored,write_stored_definition,options)
  new_file(header_file_expanded,write_expanded_definition,options)

if __name__ == '__main__': main()
