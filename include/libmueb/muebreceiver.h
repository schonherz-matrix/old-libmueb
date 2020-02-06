#ifndef MUEBRECEIVER_H
#define MUEBRECEIVER_H

#include <QImage>
#include <QObject>
#include <memory>

#include "libmueb_global.h"

class MuebReceiverPrivate;

class LIBMUEB_EXPORT MuebReceiver : public QObject {
  Q_OBJECT
  Q_DECLARE_PRIVATE(MuebReceiver)
  Q_DISABLE_COPY(MuebReceiver)

 public:
  explicit MuebReceiver(QObject* parent = nullptr);
  ~MuebReceiver();

 signals:
  void frameChanged(QImage f);

 private:
  std::unique_ptr<MuebReceiverPrivate> const d_ptr;

  bool updateFrame(const QByteArray data);
  void readPendingDatagrams();
};

#endif  // MUEBRECEIVER_H
