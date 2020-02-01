#include "muebtransmitter.h"

#include <QByteArray>

MuebTransmitter::MuebTransmitter(QObject *parent) : QObject(parent) {
  qInfo() << "[MuebTransmitter] UDP Socket will send frame to"
          << m_targetAddress.toString();
}

/* Reference:
 * http://threadlocalmutex.com/?p=48
 * http://threadlocalmutex.com/?page_id=60
 */
inline static quint8 reduceColor(quint8 c) {
  if (libmueb::defaults::colorDepth == 3)
    return (c * 225 + 4096) >> 13;
  else if (libmueb::defaults::colorDepth == 4)
    return (c * 15 + 135) >> 8;

  return c;
}

void MuebTransmitter::sendFrame(QImage frame) {
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

  const auto frameData = frame.constBits();
  qint8 packetNumber = 0;

  QByteArray datagram;
  datagram.reserve(maxWindowPerDatagram * windowByteSize + packetHeaderSize);

  // Packet header
  datagram.append(1);
  datagram.append(packetNumber);

  for (int windowIdx = 0; windowIdx < windows; ++windowIdx) {
    auto row = (windowIdx / windowPerRow) * verticalPixelUnit;
    auto col = (windowIdx % windowPerRow) * horizontalPixelUnit;

    for (int y = 0; y < verticalPixelUnit; ++y) {
      for (int x = 0; x < horizontalPixelUnit * 3; x += 3) {
        auto redIdx = (width * 3) * (row + y) + (col * 3 + x);

        if (colorDepth < 5) {  // < 5 bit color compression
          if (x % 2 == 0) {
            datagram.append(reduceColor(frameData[redIdx]) << factor |
                            reduceColor(frameData[redIdx + 1]));  // RG
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
    }

    if ((windowIdx + 1) % maxWindowPerDatagram == 0 ||
        ((windowIdx + 1) == windows && windows % maxWindowPerDatagram != 0)) {
      m_socket.writeDatagram(datagram, m_targetAddress, m_targetPort);

      datagram.truncate(0);
      datagram.append(1);
      datagram.append(++packetNumber);
    }
  }
}

void MuebTransmitter::sendFrame(QPixmap frame) {
  sendFrame(frame.toImage().convertToFormat(QImage::Format_RGB888));
}

void MuebTransmitter::sendPixel(QRgb pixel, bool windowIdx, quint8 pixelIdx,
                                QHostAddress targetAddress) {
  if (targetAddress.isNull()) return;

  const char data[] = {
      windowIdx, static_cast<char>(pixelIdx), static_cast<char>(qRed(pixel)),
      static_cast<char>(qGreen(pixel)), static_cast<char>(qBlue(pixel))};

  m_socket.writeDatagram(data, sizeof(data), targetAddress,
                         libmueb::defaults::unicastPort);
}
