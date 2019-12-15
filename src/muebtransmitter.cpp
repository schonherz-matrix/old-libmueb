#include "muebtransmitter.h"

MUEBTransmitter::MUEBTransmitter(QObject *parent, QString addr, uint16_t port)
    : QObject(parent), socket_(this), address_(addr), port_(port) {
  qInfo() << "UDP Socket will send frame to " << address_.toString();
}

// PN = package number
// wn = window number
inline static void CreatePackage(QImage f, uint8_t PN, uint8_t out[]) {
  out[0] = 1;
  out[1] = PN + 1;  // 1, 2, 3, 4

  // PN * 52: wn = 0, 52, 104, 156
  // (PN + 1)*52: wn = 52, 104, 156, 208
  // (PN + 1)*52
  for (uint8_t wn = PN * 52, i = 0; wn < (PN + 1) * 52; ++wn, ++i) {
    // wn = 0, i = 0

    QColor leftTop, rightTop, leftBottom, rigthBottom;  // one side

    // read frame
    //                              x                   y
    leftTop = f.pixelColor((wn % 16) * 2 + 0, (wn / 16) * 2 + 0);
    rightTop = f.pixelColor((wn % 16) * 2 + 1, (wn / 16) * 2 + 0);
    leftBottom = f.pixelColor((wn % 16) * 2 + 0, (wn / 16) * 2 + 1);
    rigthBottom = f.pixelColor((wn % 16) * 2 + 1, (wn / 16) * 2 + 1);

    // start from the 2 index see out[0], out[1]
    // 6 item one window
    // 0-5 offset from the start of the window
    out[i * 6 + 0 + 2] = ((leftTop.red() >> 1) & 0x70) | (leftTop.green() >> 5);
    out[i * 6 + 1 + 2] = ((leftTop.blue() >> 1) & 0x70) | (rightTop.red() >> 5);
    out[i * 6 + 2 + 2] = ((rightTop.green() >> 1) & 0x70) | (rightTop.blue() >> 5);

    out[i * 6 + 3 + 2] = ((leftBottom.red() >> 1) & 0x70) | (leftBottom.green() >> 5);
    out[i * 6 + 4 + 2] = ((leftBottom.blue() >> 1) & 0x70) | (rigthBottom.red() >> 5);
    out[i * 6 + 5 + 2] = ((rigthBottom.green() >> 1) & 0x70) | (rigthBottom.blue() >> 5);
  }
}

void MUEBTransmitter::sendFrame(QImage frame) {
  if (frame.width() != 32 || frame.height() != 26) {
    qWarning() << "[MUEBTransmitter] Ez a csomag FOS!";
    return;
  }

  uint8_t data[314] = {};

  for (uint8_t i = 0; i < 4; i += 1) {
    CreatePackage(frame, i, data);

    socket_.writeDatagram(reinterpret_cast<char *>(data), 314, address_, 10000);
  }
}
