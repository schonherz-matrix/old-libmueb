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

static void uncompressColor(const QByteArray& data, quint8* const& frameData,
                            const quint32& frameIdx, quint32& dataIdx,
                            const quint32& x) {
  using namespace libmueb::defaults;

  if (colorDepth < 5) {
    if (x % 2 == 0) {
      frameData[frameIdx] = data[dataIdx] & 0xf0;                  // R(G)
      frameData[frameIdx + 1] = (data[dataIdx] & 0x0f) << factor;  // (R)G
      frameData[frameIdx + 2] = data[dataIdx + 1] & 0xf0;          // B(R)

      dataIdx++;
    } else {
      frameData[frameIdx] = (data[dataIdx] & 0x0f) << factor;          // (B)R
      frameData[frameIdx + 1] = data[dataIdx + 1] & 0xf0;              // G(B)
      frameData[frameIdx + 2] = (data[dataIdx + 1] & 0x0f) << factor;  // (G)B

      dataIdx += 2;
    }
  } else {
    frameData[frameIdx] = data[dataIdx];          // R
    frameData[frameIdx + 1] = data[dataIdx + 1];  // G
    frameData[frameIdx + 2] = data[dataIdx + 2];  // B

    dataIdx += 3;
  }
}

bool MuebReceiver::updateFrame(const QByteArray data) {
  Q_D(MuebReceiver);

  using namespace libmueb::defaults;

  // Packet header check
  auto protocol = data[0];
  if (protocol != 1 && protocol != 2) return false;

  const quint32 packetNumber = data[1];
  if (packetNumber >= maxPacketNumber || packetNumber < 0) return false;

  const auto frameData = d->frame.bits();
  auto dataIdx = packetHeaderSize;

  if (protocol == 2) {
    for (quint32 pixelIdx = packetNumber * maxPixelPerDatagram;
         pixelIdx < (packetNumber + 1) * maxPixelPerDatagram; ++pixelIdx) {
      // Check datagram index
      // Drop invalid packet
      if (dataIdx >= data.size()) return false;

      auto frameIdx = pixelIdx * 3;
      uncompressColor(data, frameData, frameIdx, dataIdx, pixelIdx);
    }
  } else if (protocol == 1) {
    for (quint32 windowIdx = packetNumber * maxWindowPerDatagram;
         windowIdx < (packetNumber + 1) * maxWindowPerDatagram &&
         windowIdx < windows;
         ++windowIdx) {
      auto row = (windowIdx / windowPerRow) * verticalPixelUnit;
      auto col = (windowIdx % windowPerRow) * horizontalPixelUnit;

      for (quint32 y = 0; y < verticalPixelUnit; ++y) {
        for (quint32 x = 0; x < horizontalPixelUnit * 3; x += 3) {
          // Check datagram index
          // Drop invalid packet
          if (dataIdx >= data.size()) return false;

          auto frameIdx = (d->frame.width() * 3) * (row + y) + (col * 3 + x);

          uncompressColor(data, frameData, frameIdx, dataIdx, x);
        }
      }
    }
  }

  return true;
}

void MuebReceiver::readPendingDatagrams() {
  Q_D(MuebReceiver);

  using namespace libmueb::defaults;

  while (d->socket.hasPendingDatagrams()) {
    auto datagram = d->socket.receiveDatagram(packetSize);
    auto data = datagram.data();
    auto size = data.size();

    if (size != packetSize && size != remainderPacketSize) {
      qWarning() << "[MuebReceiver] Packet has invalid size!" << size
                 << "bytes";
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
