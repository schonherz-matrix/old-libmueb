#include "muebcontroller.h"

#include <QDebug>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QTcpSocket>
#include <QUdpSocket>

class MuebControllerPrivate {
  Q_DISABLE_COPY(MuebControllerPrivate)
  Q_DECLARE_PUBLIC(MuebController)

  MuebController* q_ptr;

 public:
  explicit MuebControllerPrivate(MuebController* controller)
      : q_ptr(controller) {
    Q_Q(MuebController);

    udpSocket.bind();

    QObject::connect(&udpSocket, &QUdpSocket::readyRead, q,
                     &MuebController::readPendingDatagrams);
  }

  QUdpSocket udpSocket;
  QTcpSocket tcpSocket;
};

MuebController& MuebController::getInstance() {
  static MuebController controller;
  return controller;
}

void MuebController::sendCommand(MuebController::Commands command,
                                 QString target, bool broadcastCommand,
                                 QByteArray macAddress) {
  using namespace libmueb::defaults;
  Q_D(MuebController);

  QHostAddress targetAddress{target};

  QByteArray packet;
  packet.append(commandMagic).append(command);

  if (broadcastCommand) {
    targetAddress = QHostAddress(broadcastAddress);
    packet.append(1);
    packet.append(macAddress);
  }

  d->udpSocket.writeDatagram(packet, targetAddress, commandPort);
}

bool MuebController::sendFirmware(QFile firmware, QString target) {
  using namespace libmueb::defaults;
  Q_D(MuebController);

  d->tcpSocket.connectToHost(target, firmwarePort, QTcpSocket::WriteOnly);

  if (!d->tcpSocket.waitForConnected()) {
    qWarning() << "[MuebController]: Unable to connect to"
               << targetAddress.toString();
    return false;
  };

  if (!firmware.open(QFile::ReadOnly)) return false;

  d->tcpSocket.write(firmware.readAll());

  return d->tcpSocket.waitForBytesWritten();
}

MuebController::MuebController()
    : d_ptr(std::make_unique<MuebControllerPrivate>(this)) {}

MuebController::~MuebController() = default;

void MuebController::readPendingDatagrams() {
  Q_D(MuebController);

  while (d->udpSocket.hasPendingDatagrams()) {
    emit commandResponse(d->udpSocket.receiveDatagram().data());
  }
}
