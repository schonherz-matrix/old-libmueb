#ifndef MUEBTRANSMITTER_H
#define MUEBTRANSMITTER_H

#include <QHostAddress>
#include <QImage>
#include <QObject>
#include <QPixmap>
#include <memory>

#include "libmueb_global.h"

class MuebTransmitterPrivate;

class LIBMUEB_EXPORT MuebTransmitter : public QObject {
  Q_OBJECT
  Q_DECLARE_PRIVATE(MuebTransmitter)
  Q_DISABLE_COPY(MuebTransmitter)

 public:
  explicit MuebTransmitter(QObject* parent = nullptr);
  ~MuebTransmitter();

 public slots:
  void sendFrame(QImage frame);
  void sendFrame(QPixmap frame);
  void sendPixel(QRgb pixel, bool windowIdx, quint8 pixelIdx,
                 QHostAddress targetAddress);

 private:
  std::unique_ptr<MuebTransmitterPrivate> d_ptr;
};

#endif  // MUEBTRANSMITTER_H
