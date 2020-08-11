#include "muebreceiver.h"

#include <QNetworkDatagram>
#include <QThread>
#include <QUdpSocket>
#include <cstdint>

class MuebReceiverPrivate {
  Q_DISABLE_COPY(MuebReceiverPrivate)
  Q_DECLARE_PUBLIC(MuebReceiver)

  MuebReceiver* q_ptr;

 public:
  explicit MuebReceiverPrivate(MuebReceiver* receiver) : q_ptr(receiver) {
    Q_Q(MuebReceiver);

    socket.bind(port);

    QObject::connect(&socket, &QUdpSocket::readyRead, q,
                     &MuebReceiver::readPendingDatagrams);
    QObject::connect(&processor, &DatagramProcessor::frameReady, receiver,
                     &MuebReceiver::frameChanged);

    qInfo() << "[MuebReceiver] UDP Socket will receive packets on port" << port;
  }

  ~MuebReceiverPrivate() {
    thread.quit();
    thread.wait();
  }

  QUdpSocket socket;
  std::uint16_t port{libmueb::defaults::port};
  DatagramProcessor processor;
  QThread thread;
};

MuebReceiver& MuebReceiver::getInstance() {
  static MuebReceiver instance;
  return instance;
}

MuebReceiver::MuebReceiver()
    : d_ptr(std::make_unique<MuebReceiverPrivate>(this)) {}

MuebReceiver::~MuebReceiver() = default;

static void uncompressColor(const QByteArray& data,
                            std::uint8_t* const& frameData,
                            const std::uint32_t& frameIdx,
                            std::uint32_t& dataIdx, const std::uint32_t& x) {
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

DatagramProcessor::DatagramProcessor() { m_frame.fill(Qt::black); }

static inline void datagramUncompressError() {
  qWarning() << "[MuebReceiver] Processed packet is invalid! Check the header "
                "or packet contents(size)";
}

void DatagramProcessor::processDatagram(const QByteArray datagram) {
  using namespace libmueb::defaults;

  // Packet header check
  auto protocol = datagram[0];
  if (protocol != 1 && protocol != 2) {
    datagramUncompressError();
    return;
  }

  const std::uint32_t packetNumber = datagram[1];
  if (packetNumber >= maxPacketNumber || packetNumber < 0) {
    datagramUncompressError();
    return;
  }

  const auto frameData = m_frame.bits();
  auto dataIdx = packetHeaderSize;

  if (protocol == 2) {
    for (std::uint32_t pixelIdx = packetNumber * maxPixelPerDatagram;
         pixelIdx < (packetNumber + 1) * maxPixelPerDatagram; ++pixelIdx) {
      // Check datagram index
      // Drop invalid packet
      if (dataIdx >= datagram.size()) {
        datagramUncompressError();
        return;
      }

      auto frameIdx = pixelIdx * 3;
      uncompressColor(datagram, frameData, frameIdx, dataIdx, pixelIdx);
    }
  } else if (protocol == 1) {
    for (std::uint32_t windowIdx = packetNumber * maxWindowPerDatagram;
         windowIdx < (packetNumber + 1) * maxWindowPerDatagram &&
         windowIdx < windows;
         ++windowIdx) {
      auto row = (windowIdx / windowPerRow) * verticalPixelUnit;
      auto col = (windowIdx % windowPerRow) * horizontalPixelUnit;

      for (std::uint32_t y = 0; y < verticalPixelUnit; ++y) {
        for (std::uint32_t x = 0; x < horizontalPixelUnit * 3; x += 3) {
          // Check datagram index
          // Drop invalid packet
          if (dataIdx >= datagram.size()) {
            datagramUncompressError();
            return;
          }

          auto frameIdx = (m_frame.width() * 3) * (row + y) + (col * 3 + x);

          uncompressColor(datagram, frameData, frameIdx, dataIdx, x);
        }
      }
    }
  }

  emit frameReady(m_frame);
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

    QMetaObject::invokeMethod(&d->processor, "processDatagram",
                              Qt::QueuedConnection,
                              Q_ARG(const QByteArray, data));
  }
}
