#ifndef MUEBCONTROLLER_H
#define MUEBCONTROLLER_H

#include <QHostAddress>
#include <QObject>
#include <memory>

#include "libmueb_global.h"

class MuebControllerPrivate;

class LIBMUEB_EXPORT MuebController final : public QObject {
  Q_OBJECT
  Q_DECLARE_PRIVATE(MuebController)
  Q_DISABLE_COPY(MuebController)

 public:
  enum Commands {
    turn_12v_off_left = 0x00,       //!< Turn left panel's 12v off
    turn_12v_off_right = 0x01,      //!< Turn right panel's 12v off
    reset_left_panel = 0x02,        //!< Reset left panel state
    reset_right_panel = 0x03,       //!< Reset right panel state
    reboot = 0x04,                  //!< Reboot device
    get_status = 0x05,              //!< Get device's status
    get_mac = 0x07,                 //!< Get device's MAC address
    use_internal_anim = 0x10,       //!< Use internal animation
    use_external_anim = 0x20,       //!< Use external animation
    blank = 0x30,                   //!< Blank both panels
    delete_network_buffers = 0x06,  //!< Flush network buffers
    ping = 0x40,                    //!< Send back 'pong' response
    start_firmware_update = 0x50,   //!< Start firmware update process
    get_firmware_checksum = 0x51,   //!< Return main program checksum
    swap_windows = 0x70,            //!< Swap left and right windows
    set_whitebalance = 0x80         //!< Set white balance
  };

  static MuebController& getInstance();

  void sendCommand(Commands command, QHostAddress target, bool broadcastCommand,
                   QByteArray macAddress = "");

 signals:
  void commandResponse(QString val);

 private:
  std::unique_ptr<MuebControllerPrivate> const d_ptr;

  MuebController();
  ~MuebController();
  void readPendingDatagrams();
};

#endif  // MUEBCONTROLLER_H
