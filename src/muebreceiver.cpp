#include "muebreceiver.h"

MuebReceiver::MuebReceiver(QObject *parent) : QObject(parent) {
  m_socket.bind(m_port);
  m_frame.fill(Qt::black);

  connect(&m_socket, &QUdpSocket::readyRead, this,
          &MuebReceiver::readPendingDatagrams);

  qInfo() << "[MuebReceiver] UDP Socket will receive packets on port" << m_port;
}

bool MuebReceiver::updateFrame(const QByteArray data) {
  using namespace libmueb::defaults;

  // Packet header check
  if (data[0] != 1) return false;

  const auto packetNumber = data[1];
  if (packetNumber >= maxPacketNumber || packetNumber < 0) return false;

  auto frameData = m_frame.bits();
  auto redIdx = packetHeaderSize;

  for (int windowIdx = packetNumber * maxWindowPerDatagram;
       windowIdx < (packetNumber + 1) * maxWindowPerDatagram &&
       windowIdx < windows;
       ++windowIdx) {
    auto row = (windowIdx / windowPerRow) * verticalPixelUnit;
    auto col = (windowIdx % windowPerRow) * horizontalPixelUnit;

    for (int y = 0; y < verticalPixelUnit; ++y) {
      for (int x = 0; x < horizontalPixelUnit * 3; x += 3) {
        // Check datagram index
        // Drop invalid packet
        if (redIdx >= data.size()) return false;

        auto frameIdx = (m_frame.width() * 3) * (row + y) + (col * 3 + x);

        if (colorDepth < 5) {
          if (x % 2 == 0) {
            frameData[frameIdx] = data[redIdx] & 0xf0;                  // R(G)
            frameData[frameIdx + 1] = (data[redIdx] & 0x0f) << factor;  // (R)G
            frameData[frameIdx + 2] = data[redIdx + 1] & 0xf0;          // B(R)

            redIdx++;
          } else {
            frameData[frameIdx] = (data[redIdx] & 0x0f) << factor;  // (B)R
            frameData[frameIdx + 1] = data[redIdx + 1] & 0xf0;      // G(B)
            frameData[frameIdx + 2] = (data[redIdx + 1] & 0x0f)
                                      << factor;  // (G)B

            redIdx += 2;
          }
        } else {
          frameData[frameIdx] = data[redIdx];          // R
          frameData[frameIdx + 1] = data[redIdx + 1];  // G
          frameData[frameIdx + 2] = data[redIdx + 2];  // B

          redIdx += 3;
        }
      }
    }
  }

  return true;
}

void MuebReceiver::readPendingDatagrams() {
  while (m_socket.hasPendingDatagrams()) {
    auto datagram = m_socket.receiveDatagram(libmueb::defaults::packetSize);
    auto data = datagram.data();
    auto size = data.size();

    if (size != libmueb::defaults::packetSize) {
      qWarning() << "[MuebReceiver] Packet has invalid size!" << size
                 << "bytes size must be or smaller than"
                 << libmueb::defaults::packetSize << "bytes";
      continue;
    }

    if (!updateFrame(data)) {
      qWarning()
          << "[MuebReceiver] Processed packet is invalid! Check the header "
             "or packet contents(size)";
      continue;
    }

    emit frameChanged(m_frame);
  }
}
