#include <boost/program_options.hpp>
#include "iniReader.h"

int main (int argc, char **argv) {

  po::options_description od("Opciones disponibles");
  od.add_options()
    ("help", "activeco [--help] [ [ --config | -c ] archivo_ini ]")
    ("config,c", po::value<std::string>(), "Archivo de configuracion de activeco")
  ;

  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, od), vm);
    po::notify(vm);
  } catch (std::exception& e) {
    std::cout << "Opciones desconocidas" << std::endl;
    std::cout << od << std::endl;
  }
  if (vm.count("help")) {
    std::cout << od << std::endl;
    return 0;
  }
  if (!vm.count("config")) {
    std::cout << "Falta el archivo de configuracion (.ini) de activeco" << std::endl;
    return 1;
  }
  std::cout << "Archivo de configuracion " << vm["config"].as<std::string>() << std::endl;

  iniReader ir(vm["config"].as<std::string>());
  if (ir.getMaps()) { 
    po::variables_map ivm = *ir.getMap();
    for (const auto & it : ivm) {
      const auto & value = it.second.value();
      if (auto pvalue = boost::any_cast<std::string>(&value)) {
        std::cout << it.first << " = " ;
        std::cout << *pvalue << std::endl;
      } else if (auto pvalue = boost::any_cast<std::vector<std::string>>(&value)) {
        std::cout << it.first << " = " << std::endl ;
        for (const auto & listIterator : *pvalue) {
          std::cout << "    " << listIterator << std::endl ;
        }
      }
    }
  }
  return 0;
}
