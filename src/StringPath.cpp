#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "StringPath.h"
using namespace std;

StringPath
StringPath::path_from_string(string s)
{
  StringPath p;
  StringPathItem spi;
  for(unsigned int i = 0; i < s.length(); i++) {
    spi.item = s[i];
    spi.type = CHAR;
    p.path.push_back(spi);
  }
  return p;
}

string
StringPath::get_string()
{
  vector <int> groups;
  vector <int>::iterator git;
  unordered_map <int, string> group_strings {};  
  string s = "";
  vector <StringPathItem>::iterator it;
  
  for (it = path.begin(); it != path.end(); it++) {
    if(it->type == CHAR) {
      s += it->item;
      for(git = groups.begin(); git != groups.end(); git++) {
        group_strings[*git] += it->item;
      }
    }
    else if(it->type == BG) {
      groups.push_back(it->num);
    }
    else if(it->type == EG) {
      if(!groups.empty()) {
        groups.pop_back();
      }
    }
    else if(it->type == BR) {
      string ns = group_strings[it->num];
      s += ns;
      for(git = groups.begin(); git != groups.end(); git++) {
	group_strings[*git] += group_strings[it->num];
      }
    }
  }
  return s;
}

vector<string>
StringPath::gen_evil_backreference_strings(vector <int> &backrefs_done)
{
  vector <string> ret_strings;
  vector <int> backrefs;
  vector <StringPathItem>::iterator it;
  
  // get list of group strings and of local backreferences
  vector <int> groups;
  vector <int>::iterator git;
  unordered_map <int, string> group_strings {};
  
  for (it = path.begin(); it != path.end(); it++) {
    if(it->type == CHAR) {
      for(git = groups.begin(); git != groups.end(); git++) {
	group_strings[*git] += it->item;
      }
    }
    else if(it->type == BG) {
      groups.push_back(it->num);
    }
    else if(it->type == EG) {
      if(!groups.empty()) {
        groups.pop_back();
      }
    }
    else if(it->type == BR) {
      if(!(std::find(backrefs_done.begin(), backrefs_done.end(), it->id) != backrefs_done.end())) {
	backrefs.push_back(it->id);
	backrefs_done.push_back(it->id);
      }
      for(git = groups.begin(); git != groups.end(); git++) {
	group_strings[*git] += group_strings[it->num];
      }
    }
  }

  // generate evilness
  vector <int>::iterator bit;
  string add;
  string remove;
  string modify;
  string temp;
  for (bit = backrefs.begin(); bit != backrefs.end(); bit++) {
    add = "";
    remove = "";
    modify = "";
    for (it = path.begin(); it != path.end(); it++) {
      if(it->type == CHAR) {
	temp = it->item;
	add += temp;
	remove += temp;
	modify += temp;
      }
      else if(it->type == BR) {
	if(it->id == *bit) {
	  // found backref to generate evilness for
	  // add
	  temp = group_strings[it->num];
	  temp.push_back(temp.back());
	  add += temp;
	  // remove
	  temp = group_strings[it->num];
	  if(temp.length() > 0) {
	    temp.pop_back();
	  }
	  remove += temp;
	  // modify
	  temp = group_strings[it->num];
	  int edit = temp.length()/2;
	  char c = temp[edit];
	  c += 1;
	  temp[edit] = c;
	  modify += temp;
	}
	else {
	  // normal backref substitution
	  temp = group_strings[it->num];
	  add += temp;
	  remove += temp;
	  modify += temp;
	}
      }
    }
    ret_strings.push_back(add);
    ret_strings.push_back(remove);
    ret_strings.push_back(modify);
  }
  
  // return final list
  return ret_strings;
}

void
StringPath::add_path(StringPath path2)
{
  path.insert(path.end(), path2.path.begin(), path2.path.end());
}

void
StringPath::add_path_item(StringPathItem item)
{
  path.push_back(item);
}

void StringPath::add_backreference(int _num, int _id)
{
  StringPathItem spi;
  spi.type = BR;
  spi.num = _num;
  spi.id = _id;
  path.push_back(spi);
}

void StringPath::add_begin_group(int _num)
{
  StringPathItem spi;
  spi.type = BG;
  spi.num = _num;
  path.push_back(spi);
}

void StringPath::add_end_group(int _num)
{
  StringPathItem spi;
  spi.type = EG;
  spi.num = _num;
  path.push_back(spi);
}

void
StringPath::add_string(string s)
{
  StringPathItem spi;
  for(unsigned int i = 0; i < s.length(); i++) {
    spi.item = s[i];
    spi.type = CHAR;
    spi.num = -1;
    path.push_back(spi);
  }
}

void
StringPath::add_char(char c)
{
  StringPathItem spi;
  spi.item = c;
  spi.type = CHAR;
  spi.num = -1;
  try
    {
      path.push_back(spi);
    }
  catch (std::bad_alloc& ba)
    {
      cout << "bad alloc caught: " << ba.what() << "\n";
    }
}
