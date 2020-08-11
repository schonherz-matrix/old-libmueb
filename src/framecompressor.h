#ifndef FRAMECOMPRESSOR_H
#define FRAMECOMPRESSOR_H

#include <QObject>

class FrameCompressor : public QObject {
  Q_OBJECT

 public slots:
  void compressFrame(QImage frame);

 signals:
  void datagramReady(QByteArray datagram);
};

#endif  // FRAMECOMPRESSOR_H
