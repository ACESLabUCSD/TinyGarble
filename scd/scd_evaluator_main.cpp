/*
 This file is part of TinyGarble. It is modified version of JustGarble
 under GNU license.

 TinyGarble is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TinyGarble is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TinyGarble.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/program_options.hpp>
#include "scd/scd_evaluator.h"
#include "scd/scd.h"
#include "util/common.h"
#include "util/log.h"
#include "util/util.h"

namespace po = boost::program_options;
using std::string;

int main(int argc, char*argv[]) {
  LogInitial(argc, argv);

  string scd_file_address;
  string p_init_str;
  string g_init_str;
  string e_init_str;
  string p_input_str;
  string g_input_str;
  string e_input_str;
  uint64_t clock_cycles;
  string output_mode_str;
  OutputMode output_mode = OutputMode::consecutive;
  po::options_description desc("");
  desc.add_options()("help,h", "produce help message")  //
  ("scd_file,i",
   po::value<string>(&scd_file_address)->default_value(
       "../scd/netlists/sum_nbit_ncc.scd"),
   "scd address")  //
  ("clock_cycles", po::value<uint64_t>(&clock_cycles)->default_value(1),
   "Number of clock cycles to evaluate the circuit.")  //
  ("p_init", po::value<string>(&p_init_str)->default_value("0"),
   "p_init in hexadecimal.")  //
  ("g_init", po::value<string>(&g_init_str)->default_value("0"),
   "g_init in hexadecimal.")  //
  ("e_init", po::value<string>(&e_init_str)->default_value("0"),
   "e_init in hexadecimal.")  //
  ("p_input", po::value<string>(&p_input_str)->default_value("5"),
   "p_input in hexadecimal.")  //
  ("g_input", po::value<string>(&g_input_str)->default_value("5"),
   "g_input in hexadecimal.")  //
  ("e_input", po::value<string>(&e_input_str)->default_value("4"),
   "e_input in hexadecimal.")  //
  ("output_mode", po::value<string>(&output_mode_str),
   "output print mode: {0:consecutive, 1:separated_clock, "
   "2:last_clock}, e.g., consecutive, 0, 1");

  po::variables_map vm;
  try {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(
        desc).allow_unregistered().run();
    po::store(parsed, vm);
    if (vm.count("help")) {
      std::cout << desc << endl;
      return SUCCESS;
    }
    po::notify(vm);
  } catch (po::error& e) {
    LOG(ERROR) << "ERROR: " << e.what() << endl << endl;
    std::cout << desc << endl;
    return FAILURE;
  }

  if (vm.count("output_mode")) {
    if (vm["output_mode"].as<string>() == "0"
        || vm["output_mode"].as<string>() == "consecutive") {
      output_mode = OutputMode::consecutive;
    } else if (vm["output_mode"].as<string>() == "1"
        || vm["output_mode"].as<string>() == "separated_clock") {
      output_mode = OutputMode::separated_clock;
    } else if (vm["output_mode"].as<string>() == "2"
        || vm["output_mode"].as<string>() == "last_clock") {
      output_mode = OutputMode::last_clock;
    } else {
      LOG(ERROR) << "ERROR: output_mode should be in "
                 "{0:consecutive, 1:separated_clock, "
                 "2:last_clock}, e.g., consecutive, 0, 1"
                 << endl;
      std::cout << desc << endl;
      return FAILURE;
    }
  }

  string output_str;
  EvalauatePlaintextStr(scd_file_address, p_init_str, g_init_str, e_init_str,
                        p_input_str, g_input_str, e_input_str, clock_cycles,
                        output_mode, &output_str);

  std::cout << output_str << endl;

  LogFinish();
  return SUCCESS;
}

