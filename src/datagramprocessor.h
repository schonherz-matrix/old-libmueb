#ifndef DATAGRAMPROCESSOR_H
#define DATAGRAMPROCESSOR_H

#include <QObject>
#include <QImage>
#include <libmuebconfig.h>

class DatagramProcessor : public QObject {
  Q_OBJECT

 public:
  DatagramProcessor();

 public slots:
  void processDatagram(const QByteArray datagram);

 signals:
  void frameReady(QImage frame);

 private:
  QImage m_frame{libmueb::defaults::frame};
};

#endif  // DATAGRAMPROCESSOR_H
