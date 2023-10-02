#pragma once

#include "constraints.hpp"
#include "grover_cli_tools.hpp"

#include <cstdio>
#include <iostream>

#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace classiq = cudaq::classiq;

classiq::AllConstraints deduce_constraint(const std::string& arg);
std::tuple<std::vector<std::string>, size_t, classiq::Constraints, bool, bool> add_cli_options(int argc, char* argv[]);
std::pair<std::string, std::unordered_map<std::string, size_t>> parse_oracle(
      const std::vector<std::string>& cli_oracle);