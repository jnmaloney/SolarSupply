#pragma once


#include <algorithm>
#include <string>


bool iequals(const std::string& a, const std::string& b)
{
    return std::equal(a.begin(), a.end(),
                      b.begin(), b.end(),
                      [](char a, char b) {
                          return tolower(a) == tolower(b);
                      });
}


bool ifind(const std::string& a, const std::string& b)
{
  int n = b.size();
    return std::equal(a.begin(), a.begin() + n,
                      b.begin(), b.end(),
                      [](char a, char b) {
                          return tolower(a) == tolower(b);
                      });
}
