// file predefgc.cc - "compile-time" garbage collection of predefined

/**   Copyright (C)  2016  Basile Starynkevitch and later the FSF
    MONIMELT is a monitor for MELT - see http://gcc-melt.org/
    This file is part of GCC.
  
    GCC is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.
  
    GCC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with GCC; see the file COPYING3.   If not see
    <http://www.gnu.org/licenses/>.
**/

#include <ostream>
#include <fstream>
#include <istream>
#include <iostream>
#include <set>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <cctype>
#include <unistd.h>

int main(int argc, char**argv)
{
  if (argc != 2) {
    fprintf(stderr, "%s expects one argument\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  if (::access(argv[1], R_OK)) {
      fprintf(stderr, "cannot access %s (%m)\n", argv[1]);
      exit (EXIT_FAILURE);
  };
  int nblin=0;
  std::ifstream inp(argv[1]);
  std::set<std::string> nameset;
  std::set<std::string> predefset;
#define MOM_HAS_PREDEFINED(Nam,Hash) predefset.insert(#Nam);
#include "_mom_predef.h"
  do {
    std::string nam;
    std::getline(inp,nam);
    if (nam.empty())
      continue;
    auto nsz = nam.size();
    assert (nsz>0);
    assert (std::isalpha(nam[0]));
    assert (std::isalnum(nam[nsz-1]));
    nameset.insert(nam);
  } while (inp);
  inp.close();
  int nbuseless=0;
  for (auto pnam: predefset)
    if (nameset.find(pnam) == namset.end()) {
      std::cout << pnam << std::endl;
      nbuseless++;
    };
  if (nbuseless>0) {
    std::cerr << nbuseless << " useless predefined items out of " << predefset.size() << std::endl;
    exit(EXIT_FAILURE);
  }
  return 0;
} // end of main
