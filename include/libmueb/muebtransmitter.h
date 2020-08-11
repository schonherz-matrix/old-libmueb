#pragma once

#include <QImage>
#include <QObject>
#include <QPixmap>
#include <cstdint>
#include <memory>

#include "libmueb_global.h"

class MuebTransmitterPrivate;

class LIBMUEB_EXPORT MuebTransmitter final : public QObject {
  Q_OBJECT
  Q_DECLARE_PRIVATE(MuebTransmitter)
  Q_DISABLE_COPY(MuebTransmitter)

 public:
  enum class Window { LEFT, RIGHT };

  static MuebTransmitter& getInstance();

 public slots:
  void sendFrame(QImage frame);
  void sendFrame(QPixmap frame);
  void sendPixel(const QColor color, Window window, const std::uint8_t pixelIdx,
                 const QString target);

 private slots:
  void datagramCompressed(QByteArray datagram);

 private:
  std::unique_ptr<MuebTransmitterPrivate> d_ptr;

  MuebTransmitter();
  ~MuebTransmitter();
};
