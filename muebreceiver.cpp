#include "muebreceiver.h"

#include <QNetworkDatagram>

inline static int UpdateFrame(char *in, QImage &f) {
  if (in[0] != 1) {
    return -1;
  }
  int PN = in[1] - 1;
  if (PN > 4) {
    return -1;
  }

  QColor leftTop, rightTop, leftBottom, rigthBottom;
  for (int wn = PN * 52, i = 0; wn < (PN + 1) * 52; ++wn, ++i) {
    leftTop.setRed((in[i * 6 + 0 + 2] & 0xf0) << 1);
    leftTop.setGreen((in[i * 6 + 0 + 2] & 0x0f) << 5);

    leftTop.setBlue((in[i * 6 + 1 + 2] & 0xf0) << 1);
    rightTop.setRed((in[i * 6 + 1 + 2] & 0x0f) << 5);

    rightTop.setGreen((in[i * 6 + 2 + 2] & 0xf0) << 1);
    rightTop.setBlue((in[i * 6 + 2 + 2] & 0x0f) << 5);

    leftBottom.setRed((in[i * 6 + 3 + 2] & 0xf0) << 1);
    leftBottom.setGreen((in[i * 6 + 3 + 2] & 0x0f) << 5);

    leftBottom.setBlue((in[i * 6 + 4 + 2] & 0xf0) << 1);
    rigthBottom.setRed((in[i * 6 + 4 + 2] & 0x0f) << 5);

    rigthBottom.setGreen((in[i * 6 + 5 + 2] & 0xf0) << 1);
    rigthBottom.setBlue((in[i * 6 + 5 + 2] & 0x0f) << 5);

    f.setPixelColor((wn % 16) * 2 + 0, (wn / 16) * 2 + 0, leftTop);
    f.setPixelColor((wn % 16) * 2 + 1, (wn / 16) * 2 + 0, rightTop);
    f.setPixelColor((wn % 16) * 2 + 0, (wn / 16) * 2 + 1, leftBottom);
    f.setPixelColor((wn % 16) * 2 + 1, (wn / 16) * 2 + 1, rigthBottom);
  }

  return 0;
}

MUEBReceiver::MUEBReceiver(QObject *parent, uint16_t port)
    : QObject(parent),
      socket_(this),
      port_(port),
      frame_(32, 26, QImage::Format_RGB888) {
  qInfo() << "UDP Socket will receive packets on port" << port_;

  frame_.fill(Qt::black);

  socket_.bind(port_);

  connect(&socket_, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

void MUEBReceiver::readPendingDatagrams() {
  char data[314];
  while (socket_.hasPendingDatagrams()) {
    qint64 len = socket_.readDatagram(data, 314);

    if (len != 314) {
      qWarning() << "[MUEBReceiver] Ez a csomag rossz meretu FOS!";
      break;
    }

    if (UpdateFrame(data, frame_) != 0) {
      qWarning() << "[MUEBReceiver] Ez a csomag invalid FOS!";
      break;
    }

    emit frameChanged(frame_);
  }
}
