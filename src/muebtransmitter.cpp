#include "muebtransmitter.h"

#include <QByteArray>

MuebTransmitter::MuebTransmitter(QObject *parent) : QObject(parent) {
  qInfo() << "UDP Socket will send frame to" << m_targetAddress.toString();
}

void MuebTransmitter::sendFrame(QImage frame) {
  using namespace libmueb::defaults;

  if (frame.width() != width || frame.height() != height) {
    qWarning() << "[MuebTransmitter] Frame has invalid size" << frame.size()
               << "must be" << libmueb::defaults::frame.size();
    return;
  }

  const auto frameData = frame.bits();
  char packageNumber = 1;
  auto prevBlueIdx = 0;

  QByteArray datagram;
  datagram.reserve(maxWindowPerDatagram * windowByteSize + packetHeaderSize);

  // Packet header
  datagram.append(1);
  datagram.append(packageNumber);

  for (int windowIdx = 0; windowIdx < windows; ++windowIdx) {
    auto row = (windowIdx / windowPerRow) * verticalPixelUnit;
    auto col = (windowIdx % windowPerRow) * horizontalPixelUnit;

    for (int y = 0; y < verticalPixelUnit; ++y) {
      for (int x = 0; x < horizontalPixelUnit * 3; x += 3) {
        auto redIdx = (width * 3) * (row + y) + (col * 3 + x);

        if (colorDepth == 3 || colorDepth == 4) {  // 3 bit, 4 bit compression
          if (x % 2 == 0) {
            datagram.append(frameData[redIdx] >> factor << factor |
                            frameData[redIdx + 1] >> factor);            // RG
            datagram.append(frameData[redIdx + 2] >> factor << factor);  // B

            prevBlueIdx = datagram.size() - 1;
          } else {
            datagram[prevBlueIdx] =
                datagram[prevBlueIdx] | frameData[redIdx] >> factor;  // (B)R
            datagram.append(frameData[redIdx + 1] >> factor << factor |
                            frameData[redIdx + 2] >> factor);  // GB
          }
        } else {                                   // 8 bit not compressed
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
      datagram.append(++packageNumber);
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
