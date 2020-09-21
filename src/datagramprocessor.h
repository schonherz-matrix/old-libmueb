#pragma once

#include <QImage>
#include <QObject>

#include "libmuebconfig.h"

class DatagramProcessor : public QObject {
  Q_OBJECT

 public:
  DatagramProcessor();

  QImage frame() const;

 public slots:
  void processDatagram(const QByteArray datagram);

 signals:
  void frameReady(QImage frame);

 private:
  QImage m_frame{libmueb::defaults::frame};
};
