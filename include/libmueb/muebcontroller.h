#ifndef MUEBCONTROLLER_H
#define MUEBCONTROLLER_H

#include <QObject>
#include <memory>

#include "libmueb_global.h"

class MuebControllerPrivate;

class LIBMUEB_EXPORT MuebController final : public QObject {
  Q_OBJECT
  Q_DECLARE_PRIVATE(MuebController)
  Q_DISABLE_COPY(MuebController)

 public:
  static MuebController& getInstance();

 signals:

 private:
  std::unique_ptr<MuebControllerPrivate> const d_ptr;

  MuebController();
  ~MuebController();
};

#endif  // MUEBCONTROLLER_H
