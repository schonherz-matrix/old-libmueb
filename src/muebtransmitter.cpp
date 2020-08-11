#include "muebtransmitter.h"

#include <QByteArray>
#include <QHostAddress>
#include <QThread>
#include <QUdpSocket>

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
  std::uint16_t targetPort = libmueb::defaults::port;
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

/* Reference:
 * http://threadlocalmutex.com/?p=48
 * http://threadlocalmutex.com/?page_id=60
 */
inline static std::uint8_t reduceColor(std::uint8_t c) {
  if (libmueb::defaults::colorDepth == 3)
    return (c * 225 + 4096) >> 13;
  else if (libmueb::defaults::colorDepth == 4)
    return (c * 15 + 135) >> 8;

  return c;
}

static void compressColor(QByteArray& datagram,
                          const std::uint8_t* const& frameData,
                          std::uint32_t redIdx, std::uint32_t x) {
  using namespace libmueb::defaults;

  if (colorDepth < 5) {  // < 5 bit color compression
    if (x % 2 == 0) {
      datagram.append(reduceColor(frameData[redIdx]) << factor |
                      reduceColor(frameData[redIdx + 1]));            // RG
      datagram.append(reduceColor(frameData[redIdx + 2]) << factor);  // B
    } else {
      datagram.back() =  // Previous Blue color
          datagram.back() | reduceColor(frameData[redIdx]);  // (B)R
      datagram.append(reduceColor(frameData[redIdx + 1]) << factor |
                      reduceColor(frameData[redIdx + 2]));  // GB
    }
  } else {                                   // 8 bit color not compressed
    datagram.append(frameData[redIdx]);      // R
    datagram.append(frameData[redIdx + 1]);  // G
    datagram.append(frameData[redIdx + 2]);  // B
  }
}

void FrameCompressor::compressFrame(QImage frame) {
  using namespace libmueb::defaults;

  const auto frameData = frame.constBits();
  qint8 packetNumber = 0;

  QByteArray datagram;
  datagram.reserve(maxWindowPerDatagram * windowByteSize + packetHeaderSize);

  // Packet header
  datagram.append(protocolType);
  datagram.append(packetNumber);

  if (mode == libmueb::Mode::ROW_WISE) {
    for (std::uint32_t pixelIdx = 0; pixelIdx < pixels; ++pixelIdx) {
      compressColor(datagram, frameData, pixelIdx * 3, pixelIdx);

      if ((pixelIdx + 1) % maxPixelPerDatagram == 0) {
        emit datagramReady(datagram);

        datagram.truncate(0);
        datagram.append(protocolType);
        datagram.append(++packetNumber);
      }
    }
  } else if (mode == libmueb::Mode::WINDOW_WISE) {
    for (std::uint32_t windowIdx = 0; windowIdx < windows; ++windowIdx) {
      auto row = (windowIdx / windowPerRow) * verticalPixelUnit;
      auto col = (windowIdx % windowPerRow) * horizontalPixelUnit;

      for (std::uint32_t y = 0; y < verticalPixelUnit; ++y) {
        for (std::uint32_t x = 0; x < horizontalPixelUnit * 3; x += 3) {
          auto redIdx = (width * 3) * (row + y) + (col * 3 + x);

          compressColor(datagram, frameData, redIdx, x);
        }
      }

      if ((windowIdx + 1) % maxWindowPerDatagram == 0 ||
          ((windowIdx + 1) == windows && windows % maxWindowPerDatagram != 0)) {
        emit datagramReady(datagram);

        datagram.truncate(0);
        datagram.append(protocolType);
        datagram.append(++packetNumber);
      }
    }
  }
}

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
