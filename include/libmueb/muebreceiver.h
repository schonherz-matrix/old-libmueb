#pragma once

#include <QImage>
#include <QObject>
#include <memory>

#include "libmueb_global.h"

class MuebReceiverPrivate;

class LIBMUEB_EXPORT MuebReceiver final : public QObject {
  Q_OBJECT
  Q_DECLARE_PRIVATE(MuebReceiver)
  Q_DISABLE_COPY(MuebReceiver)
  Q_PROPERTY(QImage frame READ frame NOTIFY frameChanged)

 public:
  static MuebReceiver& getInstance();

  QImage frame() const;

 signals:
  void frameChanged(QImage f);

 private:
  std::unique_ptr<MuebReceiverPrivate> const d_ptr;

  MuebReceiver();
  ~MuebReceiver();
  void readPendingDatagrams();
};
