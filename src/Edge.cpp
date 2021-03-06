/*  Edge.cpp: an edge in an NFA and a path 

    Copyright (C) 2016  Eric Larson and Anna Kirk
    elarson@seattleu.edu

    This file is part of EGRET.

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
*/

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "Edge.h"
#include "StringPath.h"
using namespace std;

StringPath
Edge::get_substring()
{
  StringPath p;
  
  switch (type) {
  case CHARACTER_EDGE:
    p.add_char(character);
    return p;
  case CHAR_SET_EDGE:
    p.add_char(char_set->get_valid_character());
    return p;
  case STRING_EDGE:
    return regex_str->get_substring();
  case END_LOOP_EDGE:
    return regex_loop->get_substring();
  case BACKREFERENCE_EDGE:
    {
      p.add_backreference(num, id);
      return p;
    }
  case BEGIN_GROUP_EDGE:
    {
      p.add_begin_group(num);
      return p;
    }
  case END_GROUP_EDGE:
    {
      p.add_end_group(num);
      return p;
    }
  default:
    return p;
  }
}

void
Edge::process_min_iter_string(StringPath *min_iter_string)
{
  switch (type) {
  case CHARACTER_EDGE:
  case CHAR_SET_EDGE:
    {
      StringPath substring = get_substring();
      if(substring.path.size() > 0) {
        min_iter_string->add_path(substring); // add substring to path
      }
      break;
    }
  case STRING_EDGE:
    regex_str->process_min_iter_string(min_iter_string);
    break;
  case END_LOOP_EDGE:
    regex_loop->process_min_iter_string(min_iter_string);
    break;
    // todo - future work, add begin group, end group, and backreference to min iter string
    /*case BEGIN_GROUP_EDGE:
    {
      StringPath substring = get_substring();
      min_iter_string->add_path(substring);
      break;
    }
  case END_GROUP_EDGE:
    {
      StringPath substring = get_substring();
      min_iter_string->add_path(substring);
      break;
    }
  case BACKREFERENCE_EDGE:
    {
      StringPath substring = get_substring();
      min_iter_string->add_path(substring);
      break;
      }*/
  default:
    ;
  }
}

bool
Edge::process_edge_in_path(StringPath path_prefix, StringPath base_substring)
{
  if (type == BEGIN_LOOP_EDGE) {
    regex_loop->process_begin_loop(path_prefix, processed);
  }

  if (type == END_LOOP_EDGE) {
    regex_loop->process_end_loop(path_prefix, processed);
  }

  // no further work needed if transition already processed
  if (processed) return false;
  processed = true;

  // set the path prefix for nodes that need processing
  switch (type) {
    case CHAR_SET_EDGE:
      char_set->set_path_prefix(path_prefix);
      return true;
    case STRING_EDGE:
      regex_str->set_path_prefix(path_prefix);
      regex_str->set_substring(base_substring);
      return true;
    case END_LOOP_EDGE:
      return true;
    case BACKREFERENCE_EDGE:
      return true;
    case BEGIN_GROUP_EDGE:
      return true;
    default:
      return false;
  }
}

set <StringPath, spcompare>
Edge::gen_evil_strings(StringPath path_string, const set <char> &punct_marks)
{
  switch (type) {
    case CHAR_SET_EDGE:
      return char_set->gen_evil_strings(path_string, punct_marks);
    case STRING_EDGE:
      return regex_str->gen_evil_strings(path_string, punct_marks);
    case END_LOOP_EDGE:
      return regex_loop->gen_evil_strings(path_string);
    case BACKREFERENCE_EDGE:
    {
      set <StringPath, spcompare> empty;
      return empty;
    }
  case BEGIN_GROUP_EDGE:
    default:
    {
      set <StringPath, spcompare> empty;
      return empty;
    }
  }
}

string
Edge::get_charset_as_string()
{
  string s = "";
  
  if (char_set->only_has_characters()) {
    s = char_set->get_charset_as_string();
  }
  
  return s;
}

bool
Edge::is_charset_complemented()
{
  return char_set->is_charset_complemented();
}

void
Edge::print()
{
  switch (type) {
  case CHARACTER_EDGE:
    cout << "CHARACTER " << character << endl;
    break;
  case CHAR_SET_EDGE:
    cout << "CHAR_SET ";
    char_set->print();
    cout << endl;
    break;
  case STRING_EDGE:
    cout << "STRING ";
    regex_str->print();
    cout << endl;
    break;
  case BEGIN_LOOP_EDGE:
    cout << "BEGIN_LOOP ";
    regex_loop->print();
    cout << endl;
    break;
  case END_LOOP_EDGE:
    cout << "END_LOOP ";
    regex_loop->print();
    cout << endl;
    break;
  case CARET_EDGE:
    cout << "CARET" << endl;
    break;
  case DOLLAR_EDGE:
    cout << "DOLLAR" << endl;
    break;
  case EPSILON_EDGE:
    cout << "EPSILON" << endl;
    break;
  case BACKREFERENCE_EDGE:
    cout << "BACKREFERENCE id:" << id << " " << name << " " << num << endl;
    break;
  case BEGIN_GROUP_EDGE:
    cout << "BEGIN GROUP " << num << endl;
    break;
  case END_GROUP_EDGE:
    cout << "END GROUP " << num << endl;
  } 
}
