#ifndef MUEBRECEIVER_H
#define MUEBRECEIVER_H

#include <QImage>
#include <QObject>
#include <QUdpSocket>

#include "libmueb_global.h"

class LIBMUEB_EXPORT MuebReceiver : public QObject {
  Q_OBJECT

 public:
  explicit MuebReceiver(QObject* parent = nullptr);

 signals:
  void frameChanged(QImage f);

 private:
  QUdpSocket m_socket{this};
  quint16 m_port{libmueb::defaults::port};
  QImage m_frame{libmueb::defaults::frame};
  bool updateFrame();
  void readPendingDatagrams();
};

#endif  // MUEBRECEIVER_H
