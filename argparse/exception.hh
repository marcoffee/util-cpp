#pragma once

#include <exception>
#include <stdexcept>

namespace util {

  class exception : public std::runtime_error {
  public:
    exception (std::string const& what)
    : std::runtime_error(what) {}
  };

  class param_exception : public exception {
  public:
    param_exception (std::string const& what)
    : exception(what) {};
  };

  class param_unknown : public param_exception {
  public:
    param_unknown (std::string const& name)
    : param_exception("Unknown param '" + name + "'.") {}
  };

  class param_name_exists : public param_exception {
  public:
    param_name_exists (std::string const& name)
    : param_exception("Param '" + name + "' exists.") {};
  };

  class param_name_does_not_exists : public param_exception {
  public:
    param_name_does_not_exists (std::string const& name)
    : param_exception("Param '" + name + "' does not exists.") {};
  };

  class param_name_invalid : public param_exception {
  public:
    param_name_invalid (std::string const& name)
    : param_exception("Invalid param '" + name + "'.") {};
  };

  class param_positional_invalid : public param_exception {
  public:
    param_positional_invalid (std::string const& what)
    : param_exception(what) {};
  };

  class param_positional_invalid_options : public param_positional_invalid {
  public:
    param_positional_invalid_options (std::string const& name)
    : param_positional_invalid(
      "Positional arg '" + name + "' should not be enable, disable or count."
    ) {};
  };

  class param_positional_multiple_exists : public param_positional_invalid {
  public:
    param_positional_multiple_exists (
      std::string const& name, std::string const& posit
    ) : param_positional_invalid(
      "Can not add positional arg '" + name +
      "' after positional multiple arg '" + posit + "'."
    ) {};
  };

  class param_non_default_invalid : public param_exception {
  public:
    param_non_default_invalid (std::string const& name)
    : param_exception(name +
      " should be required, multiple, enable, disable or count"
      " to not require a default."
    ) {};
  };

  class param_choices_invalid : public param_exception {
  public:
    param_choices_invalid (std::string const& what)
    : param_exception(what) {};
  };

  class param_is_required : public param_exception {
  public:
    param_is_required (std::string const& name)
    : param_exception("Param '" + name + "' is required.") {};
  };


  class param_choices_edc : public param_choices_invalid {
  public:
    param_choices_edc (std::string const& name)
    : param_choices_invalid(
      "Enable, disable or count param '" + name + "' can not have choices."
    ) {};
  };

  class param_choices_not_exist : public param_choices_invalid {
  public:
    param_choices_not_exist (std::string const& name, std::string const& value)
    : param_choices_invalid(
      "Choice '" + value + "' not enabled for param '" + name + "'."
    ) {};
  };

  class option_exception : public exception {
  public:
    option_exception (std::string const& what)
    : exception(what) {};
  };

  class options_invalid_edc_multiple : public option_exception {
  public:
    options_invalid_edc_multiple (std::string const& name)
    : option_exception(
      "Enable, disable or count param '" + name +
      "' can not have multiple arguments."
    ) {};
  };

  class options_invalid_edc_required : public option_exception {
  public:
    options_invalid_edc_required (std::string const& name)
    : option_exception(
      "Enable, disable or count param '" + name + "' can not be required."
    ) {};
  };

  class options_invalid_enable_count : public option_exception {
  public:
    options_invalid_enable_count (std::string const& name)
    : option_exception(
      "Enable param '" + name + "' can not be count."
    ) {};
  };

  class options_invalid_enable_disable : public option_exception {
  public:
    options_invalid_enable_disable (std::string const& name)
    : option_exception(
      "Enable param '" + name + "' can not be disable."
    ) {};
  };

  class options_invalid_disable_count : public option_exception {
  public:
    options_invalid_disable_count (std::string const& name)
    : option_exception(
      "Disable param '" + name + "' can not be count."
    ) {};
  };

};
