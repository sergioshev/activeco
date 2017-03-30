#include "logger.h"

std::unordered_map<std::string, int> logLevelToSwitch
{
  {"WARN", WARN},
  {"ERROR", ERROR},
  {"DEBUG", DEBUG},
  {"INFO", INFO},
  {"TRACE", TRACE}
};

