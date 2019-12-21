#ifndef MUEBTRANSMITTER_H
#define MUEBTRANSMITTER_H

#include <QHostAddress>
#include <QImage>
#include <QObject>
#include <QPixmap>
#include <QUdpSocket>

#include "libmueb_global.h"

class LIBMUEB_EXPORT MuebTransmitter : public QObject {
  Q_OBJECT

 public:
  explicit MuebTransmitter(QObject* parent = nullptr);

  void sendFrame(QImage frame);
  void sendFrame(QPixmap frame);
  void sendPixel(QRgb pixel, bool windowIdx, quint8 pixelIdx,
                 QHostAddress targetAddress);

 private:
  QHostAddress m_targetAddress{libmueb::defaults::broadcastAddress};
  quint16 m_targetPort = libmueb::defaults::port;
  QUdpSocket m_socket{this};
};

#endif  // MUEBTRANSMITTER_H
