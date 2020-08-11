#pragma once

#include <QObject>

class FrameCompressor : public QObject {
  Q_OBJECT

 public slots:
  void compressFrame(QImage frame);

 signals:
  void datagramReady(QByteArray datagram);
};
