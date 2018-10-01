#pragma once

#include <array>
#include <iostream>

// Create a shared buffer
extern char buffer[1024];

// Thanks to https://stackoverflow.com/a/8244052/6441345

template <typename CharT = char>
class null_buffer : public std::basic_streambuf<CharT> {
 public:
  int overflow (int c) override {
    // Calculate borders of shared buffer
    constexpr CharT* start = reinterpret_cast<CharT*>(buffer);
    constexpr uintmax_t size = sizeof(buffer) / sizeof(CharT);

    // Sets 'new buffer' borders
    this->setp(start, start + size);
    return c == std::char_traits<CharT>::eof() ? '\0' : c;
  }
};

extern null_buffer<char> null_buff;

template <typename CharT = char>
class null_ostream : public std::basic_ostream<CharT> {
 public:
  null_ostream (void) : std::basic_ostream<CharT>(&null_buff) {}
  null_buffer<CharT>* rdbuf (void) const { return &null_buff; }
};

extern null_ostream<char> null_ostr;
