#include "muebcontroller.h"

MuebController &MuebController::getInstance() {
  static MuebController controller;
  return controller;
}
