#ifndef MUEBCONTROLLER_H
#define MUEBCONTROLLER_H

#include <QObject>

#include "libmueb_global.h"

class LIBMUEB_EXPORT MuebController final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(MuebController)

 public:
  static MuebController& getInstance();

 signals:

 private:
  MuebController() = default;
  ~MuebController() = default;
};

#endif  // MUEBCONTROLLER_H
