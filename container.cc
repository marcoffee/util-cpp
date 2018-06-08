#include "container.hh"

std::string
  default_sep(", "), default_align(""),
  default_border(" "), default_open("["), default_close("]"),
  vector_sep(", "), vector_align(""),
  vector_border(" "), vector_open("["), vector_close("]"),
  map_sep("\n"), map_align("\t"),
  map_border("\n"), map_open("{"), map_close("}");

uintmax_t default_max_print = 20, vector_max_print = 20, map_max_print = 20;
