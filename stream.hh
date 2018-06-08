#pragma once

#include <iostream>

template <typename CharT = char>
class null_buffer : public std::basic_streambuf<CharT> {
public:
  int overflow (int c) { return c == std::char_traits<CharT>::eof() ? '\0' : c; }
};

template <typename CharT = char>
class null_ostream : public std::basic_ostream<CharT> {
  null_buffer<CharT> buff;
public:
  null_ostream (void) : std::basic_ostream<CharT>(&(this->buff)) {}
  null_buffer<CharT> const* rdbuf() const { return &(this->buff); }
};

extern  null_buffer<char> null_buff;
extern null_ostream<char> null_ostr;
