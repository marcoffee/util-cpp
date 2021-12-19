#include "base.hh"

namespace util {

  string_vector const argparse::params::empty_vector;

  argparse::params::params (string_map<std::string> const& def, string_set const& non_default) {
    for (std::string const& v : non_default) {
      this->data_.emplace(v, empty_vector);
      this->set_[v] = false;
    }

    for (auto const& [ key, value ] : def) {
      this->data_.emplace(key, empty_vector).first->second.emplace_back(value);
      this->set_[key] = false;
    }
  }

  bool argparse::params::is_set (std::string const& name) const {
    auto it = this->set_.find(name);
    return it != this->set_.end() and it->second;
  }

  void argparse::params::set (std::string const& name, std::string const& val, bool replace) {
    auto it = this->data_.find(name);
    bool is_set = this->is_set(name);

    replace |= !is_set;

    if (it == this->data_.end()) {
      it = this->data_.emplace(name, empty_vector).first;
    } else if (replace) {
      it->second.clear();
    }

    it->second.emplace_back(val);

    if (!is_set) {
      this->set_[name] = true;
    }
  }

  std::string& argparse::params::get_ref (std::string const& name) {
    auto it = this->data_.find(name);

    if (it != this->data_.end()) {
      return it->second.front();
    }

    throw std::out_of_range("Parameter '" + name + "' not found.");
  }

  std::string const& argparse::params::get_ref (std::string const& name) const {
    auto it = this->data_.find(name);

    if (it != this->data_.end()) {
      return it->second.front();
    }

    throw std::out_of_range("Parameter '" + name + "' not found.");
  }

  template <>
  std::string const& argparse::params::get<std::string const&> (
    std::string const& name, conversor<std::string const&> const&
  ) const {
    return this->get_ref(name);
  }

  std::ostream& operator << (std::ostream& out, argparse::params const& args) {
    for (auto const& [ key, value ] : args.data_) {
      out << key << ": ";
      bool first = true;

      for (std::string const& v : value) {
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
      if (!isalnum(c) and c != '-' and c != '_') {
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
    if (opt.is_enabler() or opt.is_disabler() or opt.is_counter()) {
      throw param_positional_invalid_options(name);
    }

    if (!this->positional_.empty()) {
      if (this->options_.at(this->positional_.back()).is_multiple()) {
        throw param_positional_multiple_exists(name, this->positional_.back());
      }
    }
  }

  void argparse::assert_options (std::string const& name, option const& opt) const {
    if (opt.is_enabler() or opt.is_disabler() or opt.is_counter()) {
      if (opt.is_multiple()) {
        throw options_invalid_edc_multiple(name);
      } else if (opt.is_required()) {
        throw options_invalid_edc_required(name);
      } else if (opt.is_enabler() and opt.is_counter()) {
        throw options_invalid_enable_count(name);
      } else if (opt.is_enabler() and opt.is_disabler()) {
        throw options_invalid_enable_disable(name);
      } else if (opt.is_disabler() and opt.is_counter()) {
        throw options_invalid_disable_count(name);
      }
    }
  }

  void argparse::assert_non_default (std::string const& name, option const& opt) const {
    if (!(opt.is_required() or opt.is_multiple() or opt.is_enabler() or opt.is_disabler() or opt.is_counter())) {
      throw param_non_default_invalid(name);
    }
  }

  void argparse::assert_choices (std::string const& name, option const& opt, string_set const& choices) const {
    if (!choices.empty() and (opt.is_enabler() or opt.is_disabler() or opt.is_counter())) {
      throw param_choices_edc(name);
    }
  }

  void argparse::assert_choices_arg (std::string const& name, std::string const& param) const {
    auto it = this->choices_.find(name);

    if (it != this->choices_.end() and !it->second.count(param)) {
      throw param_choices_not_exist(name, param);
    }
  }

  void argparse::assert_param_required (std::string const& name, option const& opt, argparse::params const& args) const {
    if (opt.is_required() and !args.is_set(name)) {
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
      this->positional_.emplace_back(name);
    }

  }

  argparse::params argparse::parse (uintmax_t const& argc, char const* const* argv) {
    argparse::params result(this->default_, this->non_default_);
    uintmax_t pos_filled = 0;
    uintmax_t i = 1;

    while (i < argc) {
      std::string name(argv[i]);

      if (this->maybe_arg(name)) {
        i++;
      } else if (pos_filled < this->positional_.size()) {
        name = this->positional_[pos_filled++];
      } else {
        throw param_unknown(name);
      }

      this->assert_param_exists(name);

      option const& opt = this->options_[name];

      if (opt.is_enabler()) {
        result.set(name, "1", true);

      } else if (opt.is_disabler()) {
        result.set(name, "0", true);

      } else if (opt.is_counter()) {
        std::string& value = result.get_ref(name);
        value = std::to_string(std::stoull(value) + 1);

      } else if (opt.is_multiple()) {
        while (i < argc) {
          std::string const arg(argv[i]);

          if (this->maybe_arg(arg)) {
            break;
          }

          this->assert_choices_arg(name, arg);
          result.set(name, arg);
          ++i;
        }

      } else if (i < argc) {
        std::string const arg(argv[i]);

        if (!this->maybe_arg(arg)) {
          this->assert_choices_arg(name, arg);
          result.set(name, arg, true);
          ++i;
        }
      }
    }

    for (auto const& [ name, opt ] : this->options_) {
      this->assert_param_required(name, opt, result);
    }

    return result;
  }

  void argparse::add (std::string const& name, std::string def, option opt, string_set const& choices) {
    this->fix_assert_new_param(name, opt, choices);

    if (opt.is_disabler()) {
      def = "1";
    } else if (opt.is_enabler() or (def.empty() and opt.is_counter())) {
      def = "0";
    }

    this->options_.emplace(name, std::move(opt));
    this->default_.emplace(name, def);

    if (!choices.empty()) {
      this->choices_.emplace(name, choices);
    }
  }

  void argparse::add (std::string const& name, option opt, string_set const& choices) {

    this->fix_assert_new_param(name, opt, choices);
    this->assert_non_default(name, opt);

    this->options_.emplace(name, std::move(opt));

    if (opt.is_enabler() or opt.is_counter()) {
      this->default_.emplace(name, "0");
    } else if (opt.is_disabler()) {
      this->default_.emplace(name, "1");
    } else {
      this->non_default_.emplace(name);
    }

    if (!choices.empty()) {
      this->choices_.emplace(name, choices);
    }
  }

};
