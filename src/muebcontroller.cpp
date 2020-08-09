#include "muebcontroller.h"

class MuebControllerPrivate {
  Q_DISABLE_COPY(MuebControllerPrivate)
  Q_DECLARE_PUBLIC(MuebController)

  MuebController* q_ptr;

 public:
  explicit MuebControllerPrivate(MuebController* controller)
      : q_ptr(controller) {
    Q_Q(MuebController);
  }
};

MuebController& MuebController::getInstance() {
  static MuebController controller;
  return controller;
}

MuebController::MuebController()
    : d_ptr(std::make_unique<MuebControllerPrivate>(this)) {}

MuebController::~MuebController() = default;
