#include "base.hh"

namespace util {

  string_vector const argparse::params::empty_vector;

  argparse::params::params (string_map<std::string> const& def, string_set const& non_default) {
    for (std::string const& v : non_default) {
      this->_data.emplace(v, empty_vector);
      this->_set[v] = false;
    }

    for (string_pair<std::string> const& kv : def) {
      this->_data.emplace(kv.first, empty_vector).first->second.emplace_back(kv.second);
      this->_set[kv.first] = false;
    }
  }

  bool argparse::params::is_set (std::string const& name) const {
    auto it = this->_set.find(name);
    return it != this->_set.end() && it->second;
  }

  void argparse::params::set (std::string const& name, std::string const& val, bool replace) {
    auto it = this->_data.find(name);
    bool is_set = this->is_set(name);

    replace |= !is_set;

    if (it == this->_data.end()) {
      it = this->_data.emplace(name, empty_vector).first;
    } else if (replace) {
      it->second.clear();
    }

    it->second.emplace_back(val);

    if (!is_set) {
      this->_set[name] = true;
    }
  }

  std::string& argparse::params::get_ref (std::string const& name) {
    auto it = this->_data.find(name);

    if (it != this->_data.end()) {
      return it->second.front();
    }

    throw std::out_of_range("Parameter '" + name + "' not found.");
  }

  std::string const& argparse::params::get_ref (std::string const& name) const {
    auto it = this->_data.find(name);

    if (it != this->_data.end()) {
      return it->second.front();
    }

    throw std::out_of_range("Parameter '" + name + "' not found.");
  }

  std::ostream& operator << (std::ostream& out, argparse::params const& args) {
    for (string_pair<string_vector> const& kv : args._data) {
      out << kv.first << ": ";
      bool first = true;

      for (std::string const& v : kv.second) {
        if (!first) {
          out << ", ";
        } else {
          first = false;
        }

        out << v;
      }

      out << std::endl;
    }

    return out;
  }

  bool argparse::valid_name (std::string const& name) {
    for (char const c : name) {
      if (!isalnum(c) && c != '-' && c != '_') {
        return false;
      }
    }

    return !name.empty();
  }

  void argparse::assert_param_exists (std::string const& name) const {
    if (!this->exists(name)) {
      throw param_name_does_not_exists(name);
    }
  }

  void argparse::assert_param_does_not_exists (std::string const& name) const {
    if (this->exists(name)) {
      throw param_name_exists(name);
    }
  }

  void argparse::assert_param_name_valid (std::string const& name) const {
    if (!argparse::valid_name(name)) {
      throw param_name_invalid(name);
    }
  }

  void argparse::assert_param_positional (std::string const& name, option const& opt) const {
    if (opt.is_enabler() || opt.is_disabler() || opt.is_counter()) {
      throw param_positional_invalid_options(name);
    }

    if (!this->_positional.empty()) {
      if (this->_options.at(this->_positional.back()).is_multiple()) {
        throw param_positional_multiple_exists(name, this->_positional.back());
      }
    }
  }

  void argparse::assert_options (std::string const& name, option const& opt) const {
    if (opt.is_enabler() || opt.is_disabler() || opt.is_counter()) {
      if (opt.is_multiple()) {
        throw options_invalid_edc_multiple(name);
      } else if (opt.is_required()) {
        throw options_invalid_edc_required(name);
      } else if (opt.is_enabler() && opt.is_counter()) {
        throw options_invalid_enable_count(name);
      } else if (opt.is_enabler() && opt.is_disabler()) {
        throw options_invalid_enable_disable(name);
      } else if (opt.is_disabler() && opt.is_counter()) {
        throw options_invalid_disable_count(name);
      }
    }
  }

  void argparse::assert_non_default (std::string const& name, option const& opt) const {
    if (!(opt.is_required() || opt.is_multiple() || opt.is_enabler() || opt.is_disabler() || opt.is_counter())) {
      throw param_non_default_invalid(name);
    }
  }

  void argparse::assert_choices (std::string const& name, option const& opt, string_set const& choices) const {
    if (!choices.empty() && (opt.is_enabler() || opt.is_disabler() || opt.is_counter())) {
      throw param_choices_edc(name);
    }
  }

  void argparse::assert_choices_arg (std::string const& name, std::string const& param) const {
    auto it = this->_choices.find(name);

    if (it != this->_choices.end() && !it->second.count(param)) {
      throw param_choices_not_exist(name, param);
    }
  }

  void argparse::assert_param_required (std::string const& name, option const& opt, argparse::params const& args) const {
    if (opt.is_required() && !args.is_set(name)) {
      throw param_is_required(name);
    }
  }

  void argparse::fix_assert_new_param (std::string const& name, option& opt, string_set const& choices) {

    this->assert_param_does_not_exists(name);
    this->assert_param_name_valid(name);
    this->assert_options(name, opt);
    this->assert_choices(name, opt, choices);

    if (name[0] != '-') {
      opt |= option::required;
      this->assert_param_positional(name, opt);
      this->_positional.emplace_back(name);
    }

  }

  argparse::params argparse::parse (uintmax_t const& argc, char const* const* argv) {
    argparse::params result(this->_default, this->_non_default);
    uintmax_t i = 1;

    while (i < argc) {
      std::string name;

      if (i <= this->_positional.size()) {
        name = this->_positional[i - 1];
      } else {
        name = argv[i++];
      }

      this->assert_param_exists(name);

      option const& opt = this->_options[name];

      if (opt.is_enabler()) {
        result.set(name, "1", true);
        continue;
      }

      if (opt.is_disabler()) {
        result.set(name, "0", true);
        continue;
      }

      if (opt.is_counter()) {
        std::string& value = result.get_ref(name);
        value = std::to_string(std::stoull(value) + 1);
        continue;
      }

      if (opt.is_multiple()) {
        while (i < argc) {
          std::string const arg(argv[i]);

          if (this->maybe_arg(arg)) {
            break;
          }

          this->assert_choices_arg(name, arg);
          result.set(name, arg);
          ++i;
        }
        continue;
      }

      if (i < argc) {
        std::string const arg(argv[i]);

        if (!this->maybe_arg(arg)) {
          this->assert_choices_arg(name, arg);
          result.set(name, arg, true);
          ++i;
        }
      }
    }

    for (string_pair<option> const& name_opt : this->_options) {
      this->assert_param_required(name_opt.first, name_opt.second, result);
    }

    return result;
  }

  void argparse::add (std::string const& name, std::string def, option opt, string_set const& choices) {
    this->fix_assert_new_param(name, opt, choices);

    if (opt.is_disabler()) {
      def = "1";
    } else if (opt.is_enabler() || (def.empty() && opt.is_counter())) {
      def = "0";
    }

    this->_options.emplace(name, std::move(opt));
    this->_default.emplace(name, def);

    if (!choices.empty()) {
      this->_choices.emplace(name, choices);
    }
  }

  void argparse::add (std::string const& name, option opt, string_set const& choices) {

    this->fix_assert_new_param(name, opt, choices);
    this->assert_non_default(name, opt);

    this->_options.emplace(name, std::move(opt));

    if (opt.is_enabler() || opt.is_counter()) {
      this->_default.emplace(name, "0");
    } else if (opt.is_disabler()) {
      this->_default.emplace(name, "1");
    } else {
      this->_non_default.emplace(name);
    }

    if (!choices.empty()) {
      this->_choices.emplace(name, choices);
    }
  }

};
