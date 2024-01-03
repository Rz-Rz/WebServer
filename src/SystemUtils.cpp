#include "SystemUtils.hpp"
#include <unistd.h>

void SystemUtils::closeUtil(int& fd) {
  if (fd >= 0)
    close(fd);
  fd = -1;
}
