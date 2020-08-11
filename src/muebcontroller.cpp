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
#ifdef Q_OS_WINDOWS
    QObject::connect(&tcpSocket, &QTcpSocket::bytesWritten, q,
                     &MuebController::closeTcpSocket);
  }
#endif

  QUdpSocket udpSocket;
  QTcpSocket tcpSocket;
  QByteArray whiteBalanceData;
  QByteArray macAddress;
  bool broadcastCommandMode{false};
};

MuebController& MuebController::getInstance() {
  static MuebController controller;
  return controller;
}

bool MuebController::sendCommand(MuebController::Commands command,
                                 QString target) {
  using namespace libmueb::defaults;
  Q_D(MuebController);

  QHostAddress targetAddress{target};

  QByteArray packet;
  packet.reserve(32);
  packet.append(commandMagic).append(command);

  if (d->broadcastCommandMode) {
    targetAddress = QHostAddress(broadcastAddress);
    packet.append(1);
    packet.append(d->macAddress);
  } else if (targetAddress.isNull()) {
    qWarning() << "[MuebController] Invalid IP address";
    return false;
  }

  if (command == Commands::set_whitebalance) {
    packet.insert(11, d->whiteBalanceData);
  }

  d->udpSocket.writeDatagram(packet, targetAddress, commandPort);

  return true;
}

bool MuebController::sendFirmware(QFile firmware, QString target) {
  using namespace libmueb::defaults;
  Q_D(MuebController);

  QHostAddress targetAddress{target};
  if (targetAddress.isNull()) {
    qWarning() << "[MuebController] Invalid IP address";
    return false;
  }

  d->tcpSocket.connectToHost(targetAddress, firmwarePort,
                             QTcpSocket::WriteOnly);

  if (!d->tcpSocket.waitForConnected()) {
    qWarning() << "[MuebController]" << d->tcpSocket.error();
    return false;
  };

  if (!firmware.open(QFile::ReadOnly)) return false;

  d->tcpSocket.write(firmware.readAll());

  // https://doc.qt.io/qt-5/qabstractsocket.html#waitForBytesWritten Note
#ifndef Q_OS_WINDOWS
  if (!d->tcpSocket.waitForBytesWritten()) return false;

  d->tcpSocket.disconnectFromHost();
  if (!d->tcpSocket.waitForDisconnected()) {
    qWarning() << "[MuebController]:" << d->tcpSocket.error();
    return false;
  }
#endif

  return true;
}

QByteArray MuebController::whiteBalanceData() const {
  return d_ptr->whiteBalanceData;
}

bool MuebController::setWhiteBalanceData(const QByteArray value) {
  if (value.size() != 21) return false;

  Q_D(MuebController);
  d->whiteBalanceData.reserve(21);
  d->whiteBalanceData = value;

  return true;
}

bool MuebController::broadcastCommandMode() const {
  return d_ptr->broadcastCommandMode;
}

bool MuebController::setBroadcastCommandMode(bool on,
                                             const QByteArray macAddress) {
  Q_D(MuebController);

  if (on) {
    if (macAddress.size() != 6) return false;
    d->macAddress.reserve(6);
  }

  d->broadcastCommandMode = on;

  return true;
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

#ifdef Q_OS_WINDOWS
void MuebController::closeTcpSocket() {
  Q_D(MuebController);

  d->tcpSocket.disconnectFromHost();
  if (!d->tcpSocket.waitForDisconnected()) {
    qWarning() << "[MuebController]" << d->tcpSocket.error();
  }
}
#endif
