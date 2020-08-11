#include "muebtransmitter.h"

#include <QByteArray>
#include <QHostAddress>
#include <QThread>
#include <QUdpSocket>

#include "framecompressor.h"

class MuebTransmitterPrivate {
  Q_DECLARE_PUBLIC(MuebTransmitter)

  MuebTransmitter* q_ptr;

 public:
  explicit MuebTransmitterPrivate(MuebTransmitter* transmitter)
      : q_ptr(transmitter) {
    qInfo() << "[MuebTransmitter] UDP Socket will send frame to"
            << targetAddress.toString();

    QObject::connect(&compressor, &FrameCompressor::datagramReady, transmitter,
                     &MuebTransmitter::datagramCompressed);

    compressor.moveToThread(&compressorThread);
    compressorThread.start();
  }

  ~MuebTransmitterPrivate() {
    compressorThread.quit();
    compressorThread.wait();
  }

  QUdpSocket socket;
  QHostAddress targetAddress{libmueb::defaults::broadcastAddress};
  std::uint16_t targetPort{libmueb::defaults::port};
  FrameCompressor compressor;
  QThread compressorThread;
};

MuebTransmitter& MuebTransmitter::getInstance() {
  static MuebTransmitter instance;
  return instance;
}

MuebTransmitter::MuebTransmitter()
    : d_ptr(std::make_unique<MuebTransmitterPrivate>(this)) {}

MuebTransmitter::~MuebTransmitter() = default;

void MuebTransmitter::sendFrame(QImage frame) {
  Q_D(MuebTransmitter);

  using namespace libmueb::defaults;

  if (frame.width() != width || frame.height() != height) {
    qWarning() << "[MuebTransmitter] Frame has invalid size" << frame.size()
               << "must be" << libmueb::defaults::frame.size();
    return;
  }

  if (frame.format() == QImage::Format_Invalid) {
    qWarning() << "[MuebTransmitter] Frame is invalid";
    return;
  }

  if (frame.format() != libmueb::defaults::frame.format())
    frame = frame.convertToFormat(libmueb::defaults::frame.format());

  QMetaObject::invokeMethod(&d->compressor, "compressFrame",
                            Qt::QueuedConnection, Q_ARG(QImage, frame));
}

void MuebTransmitter::sendFrame(QPixmap frame) {
  sendFrame(frame.toImage().convertToFormat(QImage::Format_RGB888));
}

void MuebTransmitter::sendPixel(QRgb pixel, bool windowIdx,
                                std::uint8_t pixelIdx, QString targetAddress) {
  Q_D(MuebTransmitter);
  QHostAddress addr(targetAddress);
  if (addr.isNull()) {
    qWarning() << "[MuebTransmitter] IP address is invalid";
    return;
  }

  if (targetAddress.isNull()) return;

  const char data[] = {
      windowIdx, static_cast<char>(pixelIdx), static_cast<char>(qRed(pixel)),
      static_cast<char>(qGreen(pixel)), static_cast<char>(qBlue(pixel))};

  d->socket.writeDatagram(data, sizeof(data), addr,
                          libmueb::defaults::unicastPort);
}

void MuebTransmitter::datagramCompressed(QByteArray datagram) {
  using namespace libmueb::defaults;

  Q_D(MuebTransmitter);

  d->socket.writeDatagram(datagram, d->targetAddress, d->targetPort);
}
