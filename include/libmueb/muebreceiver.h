#ifndef MUEBRECEIVER_H
#define MUEBRECEIVER_H

#include <QImage>
#include <QObject>
#include <memory>

#include "libmueb_global.h"

class MuebReceiverPrivate;

class LIBMUEB_EXPORT MuebReceiver final : public QObject {
  Q_OBJECT
  Q_DECLARE_PRIVATE(MuebReceiver)
  Q_DISABLE_COPY(MuebReceiver)

 public:
  static MuebReceiver& getInstance();

 signals:
  void frameChanged(QImage f);

 private:
  std::unique_ptr<MuebReceiverPrivate> const d_ptr;

  MuebReceiver();
  ~MuebReceiver();
  bool updateFrame(const QByteArray data);
  void readPendingDatagrams();
};

#endif  // MUEBRECEIVER_H
