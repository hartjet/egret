/*  CharSet.cpp: Character set

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

#include <cassert>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>
#include "CharSet.h"
#include "error.h"
#include "StringPath.h"
#include <string>
#include <algorithm>
using namespace std;

void
CharSet::add_item(CharSetItem item)
{
  items.push_back(item);
}

bool
CharSet::is_string_candidate()
{
  if (complement) return true;

  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    switch (it->type) {
      case CHARACTER_ITEM:
        break; 	// Ignore
      case CHAR_CLASS_ITEM:
        switch (it->character) {
          case 'w':	return true;
          case 'D':	return true;
          case 'S':	return true;
          case '.':	return true;
          default:	;
	}
        break;
      case CHAR_RANGE_ITEM:
        if (it->range_start == 'a' && it->range_end == 'z') return true;
        if (it->range_start == 'A' && it->range_end == 'Z') return true;
	break;
    }
  }

  return false;
}

char
CharSet::get_valid_character()
{
  vector <CharSetItem>::iterator it;

  if (!complement) {
    // If set is not complemented, choose the first explicit character.
    for (it = items.begin(); it != items.end(); it++) {
      if (it->type == CHARACTER_ITEM) {
	return it->character;
      }
    }
    // If set is not complemented and there are no explicit character,
    // choose the first character that meets the criteria.
    for (it = items.begin(); it != items.end(); it++) {
      switch (it->type) {
	case CHARACTER_ITEM:
	  break; 	// Ignore - alreay processed in earlier loop.
        case CHAR_CLASS_ITEM:
          switch (it->character) {
  	    case 'w':	return 'a';
	    case 'd':	return '0';
	    case 's':	return ' ';
	    case 'W':	return ';';
	    case 'D':	return 'a';
	    case 'S':	return 'a';
	    case '.':	return 'a';
	    default:
	    {
	      stringstream s;
	      s << "ERROR (internal): Invalid character class in character set: "
	           << it->character;
	      throw EgretException(s.str());
  	    }
	  }
          break;
        case CHAR_RANGE_ITEM:
  	  return it->range_start;
	  break;
      }
    }
    throw EgretException("ERROR (internal): Could not find good char in regular char set");
  }

  // At this point, the character set is complemented. Find the first valid character.
  for (char c = 'a'; c <= 'z'; c++) {
    if (is_valid_character(c)) return c;
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    if (is_valid_character(c)) return c;
  }
  for (char c = '0'; c <= '9'; c++) {
    if (is_valid_character(c)) return c;
  }
  if (is_valid_character(' ')) return ' ';

  for (char c = 33; c <= 47; c++) {
    if (is_valid_character(c)) return c;
  }
  for (char c = 58; c <= 64; c++) {
    if (is_valid_character(c)) return c;
  }
  for (char c = 91; c <= 96; c++) {
    if (is_valid_character(c)) return c;
  }
  for (char c = 123; c <= 126; c++) {
    if (is_valid_character(c)) return c;
  }

  throw EgretException("ERROR (internal): Could not find good char in complemented char set");
}

void
CharSet::check_invalid_punctuation()
{
  check_single_punctuation();
  check_only_digits_and_punctuation();
}

void
CharSet::check_only_digits_and_punctuation()
{
  bool found_other = false;
  bool found_digit = false;
  bool found_punctuation = false;

  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    if (it->type == CHAR_RANGE_ITEM) {
      if (((it->range_start >= 48) && (it->range_start <= 57)) || ((it->range_end >= 48) && (it->range_end <= 57))) {
	found_digit = true;
      }
      else {
	found_other = true;
      }
    }
    else if (it->type == CHAR_CLASS_ITEM) {
      if (it->character == 'd') {
	found_digit = true;
      }
      else {
	found_other = true;
      }
    }
    else if (it->type == CHARACTER_ITEM) {
      char c = it->character;
      if (((c >= 33) && (c <= 47)) ||
          ((c >= 58) && (c <= 64)) ||
          ((c >= 91) && (c <= 96)) ||
          ((c >= 123) && (c <= 126))) {
        found_punctuation = true;
      }
      else if ((c >= 48) && (c <= 57)) {
	found_digit = true;
      }
      else {
	found_other = true;
      }
    }
  }

  if (found_digit && found_punctuation && !found_other) {
    stringstream s;
    s << "WARNING: Character set contains only digits and punctuation marks";
    addWarning(s.str());
  }
}

void
CharSet::check_single_punctuation()
{
  bool found_punctuation = false;
  char punctuation_mark;
  bool found_another_punctuation = false;
  bool other_chars = false;
  bool ignore = false;
  
  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    if (!complement) {
      if ((it->type == CHAR_CLASS_ITEM) && (it->character == 'w')) {
        ignore = true;
      }
      if (it->type == CHARACTER_ITEM) {
        char c = it->character;
        if (((c >= 33) && (c <= 47)) ||
	    ((c >= 58) && (c <= 64)) ||
	    ((c >= 91) && (c <= 96)) ||
	    ((c >= 123) && (c <= 126))) {
	  if (!found_punctuation) {
	    punctuation_mark = c;
	    found_punctuation = true;
	  }
	  else if (c != punctuation_mark) { // if different
	    found_another_punctuation = true;
	    ignore = true;
	  }
        }
	else {
	  other_chars = true;
	}
      }
    }
  }

  if (found_punctuation && !found_another_punctuation && !ignore && other_chars) {
    stringstream s;
    s << "WARNING: Only punctuation mark inside a character set is: " << string(1, punctuation_mark);
    addWarning(s.str());
  }
}

string
CharSet::get_charset_as_string()
{
  vector <CharSetItem>::iterator it;
  string ret = "";
  vector<CharSetItem> sorted_items = items;

  struct sortClass {
    bool operator() (CharSetItem x, CharSetItem y) { return (x.character <= y.character); }
  } sortObject;
  
  std::sort(sorted_items.begin(), sorted_items.end(), sortObject);

  for (it = sorted_items.begin(); it != sorted_items.end(); it++) {
    ret.push_back(it->character);
  }
  
  return ret;
}

bool
CharSet::only_has_characters()
{
  bool only_characters = true;
  vector <CharSetItem>::iterator it;

  for (it = items.begin(); it != items.end(); it++) {
    if (it->type != CHARACTER_ITEM) {
      only_characters = false;
    }
    else {
      char c = it->character;
      if ((c < 48) || ((c > 57) && (c < 65)) || ((c > 90) && (c < 97)) || (c > 122)) {
	only_characters = false;
      }
    }
  }
  
  return only_characters;
}

bool
CharSet::is_charset_complemented()
{
  return complement;
}

bool
CharSet::is_valid_character(char character)
{
  assert(complement);

  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    switch (it->type) {
      case CHARACTER_ITEM:
	if (character == it->character) return false;
	break;
      case CHAR_CLASS_ITEM:
        switch (it->character) {
          case 'w':
	    if (character >= 'a' && character <= 'z') return false; 
	    if (character >= 'A' && character <= 'Z') return false; 
	    if (character >= '0' && character <= '9') return false; 
	    if (character == '_') return false; 
	    break;
	  case 'd':
	    if (character >= '0' && character <= '9') return false; 
	    break;
	  case 's':
	    if (character == ' ') return false;
	    break;
	  case 'W':
	  {
	    bool matches_w = false;
	    if (character >= 'a' && character <= 'z') matches_w = true;
	    if (character >= 'A' && character <= 'Z') matches_w = true;
	    if (character >= '0' && character <= '9') matches_w = true;
	    if (character == '_') matches_w = true;
	    if (!matches_w) return false;
	    break;
	  }
	  case 'D':
	    if (!(character >= '0' && character <= '9')) return false; 
	    break;
	  case 'S':
	    if (character != ' ') return false;
	    break;
	  case '.':		
	    throw EgretException("ERROR (internal): Wilcard '.' should never be in complemented char set");
	  default:
	  {
	    stringstream s;
	    s << "ERROR (internal): Invalid character class in character set: "
	      << it->character;
	    throw EgretException(s.str());
	  }
  	}
        break;

      case CHAR_RANGE_ITEM:
        if (character >= it->range_start && character <= it->range_end) return false;
        break;
    }
  }
  return true;
}

set <StringPath, spcompare>
CharSet::gen_evil_strings(StringPath path_string, const set <char> &punct_marks)
{
  set <char> test_chars  = create_test_chars(punct_marks);
  StringPath path_suffix;
  int initial = path_prefix.path.size() + 1;
  int final = path_string.path.size();
  for(int i = initial; i < final; i++) path_suffix.add_path_item(path_string.path[i]); 

  set <StringPath, spcompare> evil_strings;
  set <char>::iterator cs;
  for (cs = test_chars.begin(); cs != test_chars.end(); cs++) {
    StringPath new_string;
    new_string.add_path(path_prefix);
    new_string.add_string(string(1,*cs));
    new_string.add_path(path_suffix);
    evil_strings.insert(new_string);
  }
  return evil_strings;
}

set <char>
CharSet::create_test_chars(const set<char> &punct_marks)
{
  check_invalid_punctuation();
  
  set <char> test_chars;
  set <char> duplicates;
  bool lowercase_flag = false;
  bool uppercase_flag = false;
  bool digit_flag = false;
  bool punct_flag = false;

  bool lowercase[26];
  bool uppercase[26];
  bool digits[10];

  for (int i = 0; i < 26; i++) {
    lowercase[i] = false;
    uppercase[i] = false;
  }
  for (int i = 0; i < 10; i++) {
    digits[i] = false;
  }

  // Process individual characters first
  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    if (it->type == CHARACTER_ITEM) {
      char c = it->character;

      // Check for duplicates
      if (test_chars.find(c) != test_chars.end()) {
	duplicates.insert(c);
      }
      else {
        test_chars.insert(c);
      }

      // Set flags properly
      if (islower(c)) {
	lowercase_flag = true;
	lowercase[c - 'a'] = true;
      }
      else if (isupper(c)) {
	uppercase_flag = true;
	uppercase[c - 'A'] = true;
      }
      else if (isdigit(c)) {
	digit_flag = true;
	digits[c - '0'] = true;
      }
    }
  }

  // Process ranges second
  for (it = items.begin(); it != items.end(); it++) {
    if (it->type == CHAR_RANGE_ITEM) {
      char start = it->range_start;
      char end = it->range_end;

      // Lowercase range
      if (start >= 'a' && end <= 'z') {
	lowercase_flag = true;
	bool found_letter = false;
	for (char c = start; c <= end; c++) {
	  if (found_letter == false && lowercase[c - 'a'] == false) {
	    test_chars.insert(c);
	    found_letter = true;
	  }
	  if (lowercase[c - 'a'] == true) {
	    duplicates.insert(c);
	  }
	  lowercase[c - 'a'] = true;
	}
      }

      // Uppercase range
      else if (start >= 'A' && end <= 'Z') {
	uppercase_flag = true;
	bool found_letter = false;
	for (char c = start; c <= end; c++) {
	  if (found_letter == false && uppercase[c - 'A'] == false) {
	    test_chars.insert(c);
	    found_letter = true;
	  }
	  if (uppercase[c - 'A'] == true) {
	    duplicates.insert(c);
	  }
	  uppercase[c - 'A'] = true;
	}
      }

      // Digit range
      else if (start >= '0' && end <= '9') {
	digit_flag = true;
	bool found_letter = false;
	for (char c = start; c <= end; c++) {
	  if (found_letter == false && digits[c - '0'] == false) {
	    test_chars.insert(c);
	    found_letter = true;
	  }
	  if (digits[c - '0'] == true) {
	    duplicates.insert(c);
	  }
	  digits[c - '0'] = true;
	}
      }
      else {
	stringstream s;
        s << "ERROR (internal): Invalid range: " << start << "-" << end;
        throw EgretException(s.str());
      }
    }
  }
  
  // Process character classes third
  for (it = items.begin(); it != items.end(); it++) {
    if (it->type == CHAR_CLASS_ITEM) {
      switch (it->character) {
        // \w - add digit, lowercase, uppercase, and '_'
        case 'w':
	  uppercase_flag = true;
	  lowercase_flag = true;
	  digit_flag = true;
	  test_chars.insert('_');
	  break;

	// \d - add digit
	case 'd':
	  digit_flag = true;
	  break;

        // \s - add space
        case 's':
	  test_chars.insert(' ');
	  break;

        // \W, \D, \S - add digit, lowercase, uppercase, '_', space, and punctuation
	case 'W':
	case 'D':
	case 'S':
	  uppercase_flag = true;
	  lowercase_flag = true;
	  digit_flag = true;
	  punct_flag = true;
	  test_chars.insert('_');
	  test_chars.insert(' ');
	  break;

	// . (wildcard) - add digit, lowercase, space, and punctuation
        case '.':
	  uppercase_flag = true;
	  lowercase_flag = true;
	  digit_flag = true;
	  punct_flag = true;
	  test_chars.insert(' ');
	  break;

	default:
	{
	  stringstream s;
	  s << "ERROR (internal): Invalid character class in character set: "
	    << it->character;
	  throw EgretException(s.str());
	}
      }
    }
  }

  // Process complemented sets
  if (complement) {
    uppercase_flag = true;
    lowercase_flag = true;
    digit_flag = true;
    punct_flag = true;
    test_chars.insert(' ');
  }

  // If lowercase is flagged, then add one more lower case letter.
  if (lowercase_flag) {
    for (char c = 'a'; c <= 'z'; c++)  {
      if (lowercase[c - 'a'] == false)  {
	test_chars.insert(c);
	break;
      }
    }
  }

  // If uppercase is flagged, then add one more upper case letter.
  if (uppercase_flag) {
    for (char c = 'A'; c <= 'Z'; c++)  {
      if (uppercase[c - 'A'] == false)  {
	test_chars.insert(c);
	break;
      }
    }
  }

  // If digit is flagged, then add one more digit.
  if (digit_flag) {
    for (char c = '0'; c <= '9'; c++)  {
      if (digits[c - '0'] == false)  {
	test_chars.insert(c);
	break;
      }
    }
  }

  // If punctuation is flagged, then add all punctuation marks that appear
  // in the regular expression.
  if (punct_flag) {
    set<char>::iterator si;
    for (si = punct_marks.begin(); si != punct_marks.end(); si++) {
      test_chars.insert(*si);
    }

    // if no punctuation marks, add underscore
    if (punct_marks.empty()) {
      test_chars.insert('_');
    }
  }

  // Print warning for duplicates if necessary
  if (!duplicates.empty()) {
    stringstream s;
    s << "DUPLICATE WARNING: Duplicate characters in character set:";
    set <char>::iterator si;
    for (si = duplicates.begin(); si != duplicates.end(); si++) {
      s << " " << *si;
    }
    addWarning(s.str());
  }

  return test_chars;
}

bool
CharSet::allows_punctuation()
{
  if (complement) return true;

  vector <CharSetItem>::iterator it;
  for (it = items.begin(); it != items.end(); it++) {
    switch (it->type) {
      case CHARACTER_ITEM:
	if (ispunct(it->character)) return true;
        break;
      case CHAR_CLASS_ITEM:
        switch (it->character) {
          case 'E':
          case 'D':
          case 'S':
          case '.':
	    return true;
          default:	;
	}
        break;
      case CHAR_RANGE_ITEM:
	break;	// ignore
    }
  }

  return false;
}

void
CharSet::print()
{
  if (complement) cout << "^";

  vector <CharSetItem>::reverse_iterator it;
  for (it = items.rbegin(); it != items.rend(); it++) {
    switch (it->type) {
    case CHARACTER_ITEM:
      cout << it->character;
      break;
    case CHAR_CLASS_ITEM:
      cout << "\\" << it->character;
      break;
    case CHAR_RANGE_ITEM:
      cout << it->range_start << "-" << it->range_end;
      break;
    }
  }
}
