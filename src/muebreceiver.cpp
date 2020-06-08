#include "muebreceiver.h"

#include <QNetworkDatagram>
#include <QUdpSocket>

class MuebReceiverPrivate {
  Q_DISABLE_COPY(MuebReceiverPrivate)
  Q_DECLARE_PUBLIC(MuebReceiver)

  MuebReceiver* q_ptr;

 public:
  explicit MuebReceiverPrivate(MuebReceiver* receiver) : q_ptr(receiver) {
    Q_Q(MuebReceiver);

    socket.bind(port);
    frame.fill(Qt::black);

    QObject::connect(&socket, &QUdpSocket::readyRead, q,
                     &MuebReceiver::readPendingDatagrams);

    qInfo() << "[MuebReceiver] UDP Socket will receive packets on port" << port;
  }

  QUdpSocket socket;
  quint16 port{libmueb::defaults::port};
  QImage frame{libmueb::defaults::frame};
};

MuebReceiver& MuebReceiver::getInstance() {
  static MuebReceiver instance;
  return instance;
}

MuebReceiver::MuebReceiver()
    : d_ptr(std::make_unique<MuebReceiverPrivate>(this)) {}

MuebReceiver::~MuebReceiver() = default;

bool MuebReceiver::updateFrame(const QByteArray data) {
  Q_D(MuebReceiver);

  using namespace libmueb::defaults;

  // Packet header check
  if (data[0] != 1 && data[0] != 2) return false;

  const auto packetNumber = data[1];
  if (packetNumber >= maxPacketNumber || packetNumber < 0) return false;

  auto frameData = d->frame.bits();
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

        auto frameIdx = (d->frame.width() * 3) * (row + y) + (col * 3 + x);

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
  Q_D(MuebReceiver);

  while (d->socket.hasPendingDatagrams()) {
    auto datagram = d->socket.receiveDatagram(libmueb::defaults::packetSize);
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

    emit frameChanged(d->frame);
  }
}
