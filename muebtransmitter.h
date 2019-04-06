#ifndef MUEBTRANSMITTER_H
#define MUEBTRANSMITTER_H

#include <QObject>
#include <QString>
#include <QUdpSocket>
#include <QImage>

class MUEBTransmitter : public QObject {
  Q_OBJECT

  QUdpSocket socket_;
  QHostAddress address_;
  uint16_t port_;

 public:
  explicit MUEBTransmitter(QObject* parent = nullptr,
                           QString addr = "10.6.255.255",
                           uint16_t port = 10000);
 public slots:
  void sendFrame(QImage frame);
};

#endif  // MUEBTRANSMITTER_H
