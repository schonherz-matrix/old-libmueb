#include "muebreceiver.h"

#include <QNetworkDatagram>
#include <QThread>
#include <QUdpSocket>
#include <cstdint>

#include "datagramprocessor.h"

class MuebReceiverPrivate {
  Q_DISABLE_COPY(MuebReceiverPrivate)
  Q_DECLARE_PUBLIC(MuebReceiver)

  MuebReceiver* q_ptr;

 public:
  explicit MuebReceiverPrivate(MuebReceiver* receiver) : q_ptr(receiver) {
    socket.bind(port);

    QObject::connect(&socket, &QUdpSocket::readyRead, receiver,
                     &MuebReceiver::readPendingDatagrams);
    QObject::connect(&processor, &DatagramProcessor::frameReady, receiver,
                     &MuebReceiver::frameChanged);

    processor.moveToThread(&processorThread);
    processorThread.start();

    qInfo() << "[MuebReceiver] UDP Socket will receive packets on port" << port;
  }

  ~MuebReceiverPrivate() {
    processorThread.quit();
    processorThread.wait();
  }

  QUdpSocket socket;
  std::uint16_t port{libmueb::defaults::port};
  DatagramProcessor processor;
  QThread processorThread;
};

MuebReceiver& MuebReceiver::getInstance() {
  static MuebReceiver instance;
  return instance;
}

QImage MuebReceiver::frame() const { return d_ptr->processor.frame(); }

MuebReceiver::MuebReceiver()
    : d_ptr(std::make_unique<MuebReceiverPrivate>(this)) {}

MuebReceiver::~MuebReceiver() = default;

void MuebReceiver::readPendingDatagrams() {
  Q_D(MuebReceiver);

  using namespace libmueb::defaults;

  while (d->socket.hasPendingDatagrams()) {
    auto datagram{d->socket.receiveDatagram(packetSize)};
    auto data{datagram.data()};
    auto size{data.size()};

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
