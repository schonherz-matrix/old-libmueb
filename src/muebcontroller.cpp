#include "muebcontroller.h"

#include <QDebug>
#include <QNetworkDatagram>
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
};

MuebController& MuebController::getInstance() {
  static MuebController controller;
  return controller;
}

void MuebController::sendCommand(MuebController::Commands command,
                                 QHostAddress target, bool broadcastCommand,
                                 QByteArray macAddress) {
  using namespace libmueb::defaults;
  Q_D(MuebController);

  QByteArray packet;
  packet.append(commandMagic).append(command);

  if (broadcastCommand) {
    target = QHostAddress(broadcastAddress);
    packet.append(1);
    packet.append(macAddress);
  }

  d->udpSocket.writeDatagram(packet, target, commandPort);
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
