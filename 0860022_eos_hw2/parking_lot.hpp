/*
 * Class:  ParkingLot 
 * --------------------
 * Properties: 
 *   - TODO
 */

#ifndef PARKINGLOT_H
#define PARKINGLOT_H
#include <iostream>
#include <vector>
#include <map>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "socket_utils.h"

#ifdef USE_PXA_IO
#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#endif

#define PL_STATE_WAIT_CONNECTION 0
#define PL_STATE_WELCOME 1
#define PL_STATE_MENU_PARKED 2
#define PL_STATE_MENU_NOT_RESERVED 3
#define PL_STATE_MENU_RESERVED 4
#ifdef USE_PXA_IO
#define PL_STATE_SHOW 5
#define PL_STATE_CHOOSE 6
#endif
#define PL_STATE_PAY 7

#define VEH_STATE_INIT 0
#define VEH_STATE_RESERVED 1
#define VEH_STATE_PARKED 2
#define VEH_STATE_PARKED_NO_RESERVED 3

#define MSG_BUFSIZE 128

/*
 * Structure:  VehicleInfo 
 * --------------------
 * Properties: 
 *   - state: 
 *   - lot: range 1 ~ 3 integer related to lot[0] ~ lot[2] respectively
 *   - grid: bitmask for occupied grids(87654321). where 1 means occupied, 0 means available. 
 */

typedef struct VehicleInfo
{
  VehicleInfo() : state(VEH_STATE_INIT), lot(-1), grid(-1){};
  VehicleInfo(int _state) : state(_state), lot(-1), grid(-1){};
  int state;
  int lot;
  int grid;
} VehicleInfo;

class ParkingLot
{
private:
  int conn_fd, sock_fd;
  int sysState;
  int balance;
  std::pair<int, VehicleInfo> currentVehicle;
  std::map<int, VehicleInfo> vehList; // pairs of "ID" and vehicle
  VehicleInfo getVehState(int);
  void setVehState(int, VehicleInfo);
  char *getGridState();
  // For I/O
  int printMessage(const char *msg, bool erase = true);
  int getMessage(char *msg);
#ifdef USE_PXA_IO
  int lcd_fd;
  lcd_write_info_t display;
  int segment_display_decimal(int data);
  int led_display_binary(int data);
#endif
public:
  ParkingLot();
  ParkingLot(int sock_fd);
  // State Nodes
  int showWait();
  int showWelcome();
  int showMenuParked();
  int showMenuReserved();
  int showMenuNotReserved();

#ifdef USE_PXA_IO
  int showChoose();
  int showSpace();
#else
  int selectGrid(int lot, int grid);
  void getSpace(char*);
#endif
  int showPay();
  void showPromptNextState();
  // Actions
  int runSystem();
  int getState();
  void logStatus(const char *log_file);
};

#endif
