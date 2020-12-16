#include "parking_lot.hpp"
#include <stdlib.h>

#define bit(x) (1ul << (x))

ParkingLot::ParkingLot()
{
  balance = 0;
#ifdef USE_PXA_IO
  lcd_fd = open("/dev/lcd", O_RDWR);
#endif
  sysState = PL_STATE_WAIT_CONNECTION;
  currentVehicle = std::pair<int, VehicleInfo>(0, VehicleInfo(VEH_STATE_INIT));
}

ParkingLot::ParkingLot(int sock_fd)
{
  balance = 0;
#ifdef USE_PXA_IO
  lcd_fd = open("/dev/lcd", O_RDWR);
#endif
  this->sock_fd = sock_fd;
  sysState = PL_STATE_WAIT_CONNECTION;
  currentVehicle = std::pair<int, VehicleInfo>(0, VehicleInfo(VEH_STATE_INIT));
}

int ParkingLot::runSystem()
{
  while (1)
  {
    // printf("state: %d\n", sysState);
    switch (sysState)
    {
    case PL_STATE_WAIT_CONNECTION:
      sysState = showWait();
      break;
    case PL_STATE_WELCOME:
      sysState = showWelcome();
      break;
    case PL_STATE_MENU_NOT_RESERVED:
      sysState = showMenuNotReserved();
      break;
    case PL_STATE_MENU_RESERVED:
      sysState = showMenuReserved();
      break;
    case PL_STATE_MENU_PARKED:
      sysState = showMenuParked();
      break;
#ifdef USE_PXA_IO
    case PL_STATE_SHOW:
      sysState = showSpace();
      break;
    case PL_STATE_CHOOSE:
      sysState = showChoose();
      break;
#endif
    case PL_STATE_PAY:
      sysState = showPay();
      break;
    default:
      perror("Invalid State\n");
    }
  }
}

int ParkingLot::showWait()
{
  struct sockaddr_in cln_addr;
  socklen_t sLen = sizeof(cln_addr);
  conn_fd = accept(sock_fd, (struct sockaddr *)&cln_addr, &sLen);
  if (conn_fd == -1)
  {
    perror("Error: accept()");
    return PL_STATE_WAIT_CONNECTION;
  }
  return PL_STATE_WELCOME;
}

int ParkingLot::showWelcome()
{
  char buf[MSG_BUFSIZE];
  // Welcome message on lcd
#ifdef USE_PXA_IO
  segment_display_decimal(0000);
  led_display_binary(0xff);
#endif
  // printMessage("Input plate num: ");
  // Get Vehicle ID input
  // getMessage()
  if (getMessage(buf) == -1)
  {
    return PL_STATE_WAIT_CONNECTION;
  }

  currentVehicle.first = atoi(buf); //assign input data;
#ifdef USE_PXA_IO
  segment_display_decimal(currentVehicle.first);
#endif
  // search stored vehicle & get vehicle state
  currentVehicle.second = getVehState(currentVehicle.first);
  // vehList[currentVehicle.first] = currentVehicle.second;
  // return new system state
  if (currentVehicle.second.state == VEH_STATE_INIT)
  {
    return PL_STATE_MENU_NOT_RESERVED;
  }
  else if (currentVehicle.second.state == VEH_STATE_RESERVED)
  {
    return PL_STATE_MENU_RESERVED;
  }
  else
  { // PARKED or PARKED_NOT_RESERVED
    return PL_STATE_MENU_PARKED;
  }
}

int ParkingLot::showMenuNotReserved()
{
  char msg_buf[MSG_BUFSIZE];
  char *tok;
  int lot, grid;
#ifdef USE_PXA_IO
  // Show menu options on lcd
  sprintf(msg_buf, "%s\n%s\n%s\n%s\n%s\n",
          "You haven't reserved a grid.",
          "1. show",
          "2. reserve",
          "3. check-in",
          "4. exit");
#else
  sprintf(msg_buf, "%s", "You haven't reserved grid.");
#endif
  printMessage(msg_buf);
  while (1)
  {
    //get input
    if (getMessage(msg_buf) == -1)
    {
      return PL_STATE_WAIT_CONNECTION;
    }

#ifdef USE_PXA_IO
    switch (msg_buf[0])
    {
    case '1':
      return PL_STATE_SHOW;
    case '2':
      return PL_STATE_CHOOSE;
    case '3':
      currentVehicle.second.state = VEH_STATE_PARKED_NO_RESERVED;
      this->setVehState(currentVehicle.first, currentVehicle.second);
      return PL_STATE_CHOOSE;
    case '4':
      return PL_STATE_WELCOME;
    default:
      std::cerr << "invalid input" << sysState << std::endl;
      return PL_STATE_WELCOME;
    }
#else
    // printf("%s", msg_buf);
    tok = strtok(msg_buf, " ");
    // printf("tok = %s\n", tok);
    // printf("msg = %s\n", msg_buf);
    if (strcmp(tok, "show") == 0)
    {
      getSpace(msg_buf);
      printMessage(msg_buf);
      continue;
    }
    else if (strcmp(tok, "reserve") == 0)
    {
      tok = strtok(NULL, " ");
      if (tok == NULL)
      {
        printMessage("Invaild command.");
        continue;
      }
      lot = atoi(tok);

      tok = strtok(NULL, " ");
      if (tok == NULL)
      {
        printMessage("Invaild command.");
        continue;
      }
      grid = atoi(tok);
      if (selectGrid(lot, grid) == 0)
      {
        currentVehicle.second.state = VEH_STATE_RESERVED;
        this->setVehState(currentVehicle.first, currentVehicle.second);
        printMessage("Reserve successful.");
        return PL_STATE_WELCOME;
      }
      // return PL_STATE_CHOOSE;
    }
    else if (strcmp(tok, "check-in") == 0)
    {
      tok = strtok(NULL, " ");
      if (tok == NULL)
      {
        printMessage("Invaild command.");
        continue;
      }
      lot = atoi(tok);

      tok = strtok(NULL, " ");
      if (tok == NULL)
      {
        printMessage("Invaild command.");
        continue;
      }
      grid = atoi(tok);
      if (selectGrid(lot, grid) == 0)
      {
        currentVehicle.second.state = VEH_STATE_PARKED;
        this->setVehState(currentVehicle.first, currentVehicle.second);
        printMessage("Check-in successful.");
        return PL_STATE_WELCOME;
      }
    }
    else if (strcmp(tok, "exit") == 0)
    {
      printMessage("Logout.");
      return PL_STATE_WELCOME;
    }
    else
    {
      // std::cerr << "invalid input" << sysState << std::endl;
      printMessage("Invaild command.");
      continue;
    }
#endif
  }
}

int ParkingLot::showMenuReserved()
{
  char msg_buf[MSG_BUFSIZE];

  // Show menu options on lcd
#ifdef USE_PXA_IO
  sprintf(msg_buf, "%s\n%s\n%s\n%s\n%s\n",
          "You have reserved a grid.",
          "1. show",
          "2. cancel",
          "3. check-in",
          "4. exit");
#else
  sprintf(msg_buf, "%s", "You have reserved grid.");
#endif
  printMessage(msg_buf);
  while (1)
  {
    //get input
    if (getMessage(msg_buf) == -1)
    {
      return PL_STATE_WAIT_CONNECTION;
    }
#ifdef USE_PXA_IO
    switch (msg_buf[0])
    {
    case '1':
      return PL_STATE_SHOW;
    case '2':
      return PL_STATE_PAY;
    case '3':
      currentVehicle.second.state = VEH_STATE_PARKED;
      this->setVehState(currentVehicle.first, currentVehicle.second);
      return PL_STATE_WELCOME;
    case '4':
      return PL_STATE_WELCOME;
    default:
      std::cerr << "invalid input" << sysState << std::endl;
      return PL_STATE_WELCOME;
    }
#else
    // printf("%s", msg_buf);

    if (strcmp(msg_buf, "show") == 0)
    {
      getSpace(msg_buf);
      printMessage(msg_buf);
      continue;
    }
    else if (strcmp(msg_buf, "cancel") == 0)
      return PL_STATE_PAY;
    else if (strcmp(msg_buf, "check-in") == 0)
    {
      currentVehicle.second.state = VEH_STATE_PARKED;
      this->setVehState(currentVehicle.first, currentVehicle.second);
      printMessage("Check-in successful.");
      return PL_STATE_WELCOME;
    }
    else if (strcmp(msg_buf, "exit") == 0)
    {
      return PL_STATE_WELCOME;
      printMessage("Logout.");
    }
    else
    {
      // std::cerr << "invalid input" << sysState << std::endl;
      printMessage("Invaild command.");
      continue;
    }
#endif
  }
}

int ParkingLot::showMenuParked()
{
  char msg_buf[MSG_BUFSIZE];

#ifdef USE_PXA_IO
  // Show menu options on lcd
  sprintf(msg_buf, "%s%d%s%d\n%s\n%s\n",
          "Your car is at lot p", currentVehicle.second.lot, " grid ", currentVehicle.second.grid,
          "1. show",
          "2. pick-up");
#else
  sprintf(msg_buf, "Your grid is at lot P%d grid %d.", currentVehicle.second.lot, currentVehicle.second.grid);
#endif
  printMessage(msg_buf);
  while (1)
  {

    //get input
    if (getMessage(msg_buf) == -1)
    {
      return PL_STATE_WAIT_CONNECTION;
    }
#ifdef USE_PXA_IO
    switch (msg_buf[0])
    {
    case '1':
      return PL_STATE_SHOW;
    case '2':
      return PL_STATE_PAY;
    default:
      std::cerr << "invalid input" << sysState << std::endl;

      return PL_STATE_WELCOME;
    }
#else
    // printf("%s", msg_buf);
    if (strcmp(msg_buf, "show") == 0)
    {
      getSpace(msg_buf);
      printMessage(msg_buf);
      return PL_STATE_MENU_NOT_RESERVED;
    }
    else if (strcmp(msg_buf, "pick-up") == 0)
      return PL_STATE_PAY;
    else if (strcmp(msg_buf, "exit") == 0)
    {
      printMessage("Logout.");
      return PL_STATE_WELCOME;
    }
    else
    {
      // std::cerr << "invalid input" << sysState << std::endl;
      printMessage("Invaild command.");
      continue;
    }
#endif
  }
}
#ifdef USE_PXA_IO
int ParkingLot::showSpace()
{
  // Show all car spaces on lcd
  int sum = 0, n = 0;
  char msg_buf[MSG_BUFSIZE] = {0};
  char *gridState = this->getGridState();

  for (int i = 0; i < 3; i++)
  {
    sum = 0;
    for (int j = 0; j < 8; j++)
    {
      sum += (int)((gridState[i] & bit(j)) != 0);
    }
    // std::cout << "p" << i << ": ";
    // std::cout << 8 - sum << std::endl;
    n = sprintf(msg_buf, "%.*sP%d: %d\n", n, msg_buf, i + 1, 8 - sum);
    n = sprintf(msg_buf, "%.*sgrid ", n, msg_buf);
    for (int j = 0; j < 8; j++)
    {
      if ((gridState[i] & bit(j)) == 0)
      {
        n = sprintf(msg_buf, "%.*s|%d", n, msg_buf, j + 1);
      }
    }
    n = sprintf(msg_buf, "%.*s|\n", n, msg_buf);
  }
#ifdef USE_PXA_IO
  sprintf(msg_buf, "%spress # to return\n", msg_buf);
#endif
  printMessage(msg_buf);
#ifdef USE_PXA_IO
  // wait for any key
  if (getMessage(msg_buf) == -1)
  {
    return PL_STATE_WAIT_CONNECTION;
  }
#endif
  return PL_STATE_WELCOME;
}
#endif
void ParkingLot::getSpace(char *msg_buf)
{
  int sum = 0, n = 0;
  char *gridState = this->getGridState();

  for (int i = 0; i < 3; i++)
  {
    sum = 0;
    for (int j = 0; j < 8; j++)
    {
      sum += (int)((gridState[i] & bit(j)) != 0);
    }
    // printf("P%d: %d\n", i, 8-sum);
    n = sprintf(msg_buf, "%.*sP%d: %d\n", n, msg_buf, i + 1, 8 - sum);
    n = sprintf(msg_buf, "%.*sgrid ", n, msg_buf);
    for (int j = 0; j < 8; j++)
    {
      if ((gridState[i] & bit(j)) == 0)
      {
        n = sprintf(msg_buf, "%.*s|%d", n, msg_buf, j + 1);
      }
    }
    n = sprintf(msg_buf, "%.*s|\n", n, msg_buf);
  }
}

#ifdef USE_PXA_IO
int ParkingLot::showChoose()
{
  // Show prompt for choose
  char msg_buf[MSG_BUFSIZE] = {0};
  char *gridState = getGridState();
  int lot, grid_num;

  // <!-- select grid -->
  printMessage("Select parking lot: ");
  if (getMessage(msg_buf) == -1)
  {
    return PL_STATE_WAIT_CONNECTION;
  }

  lot = atoi(msg_buf);
  if (lot > 3 || lot < 1)
  {
    printMessage("Error!");
    return PL_STATE_WELCOME;
  }
  else
  {
    printf("data = %d\n", gridState[lot - 1]);
#ifdef USE_PXA_IO
    led_display_binary(gridState[lot - 1]);
#endif
  }

  // <!-- select grid && show lot state on "LED" -->
  printMessage("\nSelect parking grid: ", false);
  if (getMessage(msg_buf) == -1)
  {
    return PL_STATE_WAIT_CONNECTION;
  }

  grid_num = atoi(msg_buf);

  if (lot > 3 || lot < 1 || grid_num > 8 || grid_num < 1)
  {
    std::cerr << "invalid input" << sysState << std::endl;
  }
  else if ((gridState[lot - 1] >> grid_num - 1) % 2 == 0)
  {
    // gridState[lot] |= (0x01 << grid_num);
    currentVehicle.second.lot = lot;
    currentVehicle.second.grid = grid_num;
    if (currentVehicle.second.state == VEH_STATE_INIT)
    {
      currentVehicle.second.state = VEH_STATE_RESERVED;
      this->setVehState(currentVehicle.first, currentVehicle.second);
      sprintf(msg_buf, "Reserved, Have a nice day\n");
    }
    else if (currentVehicle.second.state == VEH_STATE_PARKED_NO_RESERVED)
    {
      this->setVehState(currentVehicle.first, currentVehicle.second);
      sprintf(msg_buf, "Parked, Have a nice day\n");
    }
#ifdef USE_PXA_IO
    sprintf(msg_buf, "%spress # to return\n", msg_buf);
#endif
    printMessage(msg_buf);
#ifdef USE_PXA_IO
    // wait for any key
    if (getMessage(msg_buf) == -1)
    {
      return PL_STATE_WAIT_CONNECTION;
    }
#endif
    return PL_STATE_WELCOME;
  }
  else
  {
    printMessage("Error! Selected grid not available\n");
    // std::cerr << "Error! Selected grid not available\n";
    return PL_STATE_WELCOME;
  }
  return PL_STATE_WELCOME;
}
#endif
int ParkingLot::selectGrid(int lot, int grid)
{
  // printf("selecting %d, %d\n", lot, grid);
  char *gridState = this->getGridState();
  if (lot > 3 || lot < 1 || grid > 8 || grid < 1)
  {
    // std::cerr << "invalid input" << sysState << std::endl;
    printMessage("Invaild command.");
    return -1;
  }
  else if ((bit(grid - 1) & gridState[lot - 1]) == 0)
  {
    // gridState[lot] |= (0x01 << grid_num);
    currentVehicle.second.lot = lot;
    currentVehicle.second.grid = grid;
    // if (currentVehicle.second.state == VEH_STATE_INIT)
    // {
    //   currentVehicle.second.state = VEH_STATE_RESERVED;
    //   this->setVehState(currentVehicle.first, currentVehicle.second);
    //   printf("selectGrid: reserved %d\n", currentVehicle.first);
    // }
    // else if (currentVehicle.second.state == VEH_STATE_PARKED_NO_RESERVED)
    // {
    //   currentVehicle.second.state = VEH_STATE_PARKED;
    //   this->setVehState(currentVehicle.first, currentVehicle.second);
    //   printf("selectGrid: checked in %d\n", currentVehicle.first);

    //   // printMessage("Check-in successful.");
    // }
    return 0;
  }
  else
  {
    printMessage("Error! Please select an ideal grid.");
    return -1;
  }
}

int ParkingLot::showPay()
{
  // <!-- if action == cancel -->
  // Reserve fee: $20
  char msg_buf[MSG_BUFSIZE] = {0};
  if (currentVehicle.second.state == VEH_STATE_PARKED)
  {
    sprintf(msg_buf, "Parking fee: $40.");
    balance += 30;
  }
  else if (currentVehicle.second.state == VEH_STATE_PARKED_NO_RESERVED)
  {
    sprintf(msg_buf, "Parking fee: $30.");
    balance += 40;
  }
  else if (currentVehicle.second.state == VEH_STATE_RESERVED)
  {
    sprintf(msg_buf, "Reserve fee: $20.");
    balance += 20;
  }

  currentVehicle.second.state = VEH_STATE_INIT;
  this->setVehState(currentVehicle.first, currentVehicle.second);
#ifdef USE_PXA_IO
  sprintf(msg_buf, "%spress # to return\n", msg_buf);
#endif
  printMessage(msg_buf);
#ifdef USE_PXA_IO
  // wait for any key
  if (getMessage(msg_buf) == -1)
  {
    return PL_STATE_WAIT_CONNECTION;
  }
#endif
  return PL_STATE_WELCOME;
}

VehicleInfo ParkingLot::getVehState(int id)
{
  std::map<int, VehicleInfo>::iterator veh = vehList.find(id);
  if (veh != vehList.end())
  {
    return veh->second;
  }
  else
  {
    std::pair<int, VehicleInfo> _veh(id, VehicleInfo(VEH_STATE_INIT));
    vehList.insert(_veh);
    return _veh.second;
  }
}

void ParkingLot::setVehState(int id, VehicleInfo state)
{
  std::map<int, VehicleInfo>::iterator veh = vehList.find(id);
  if (veh != vehList.end())
  {
    veh->second = state;
  }
  else
  {
    vehList.insert(std::pair<int, VehicleInfo>(id, state));
  }
}

char *ParkingLot::getGridState()
{
  char *gridState = new char[3];
  // std::cout << "getting\n";
  for (std::map<int, VehicleInfo>::iterator it = vehList.begin(); it != vehList.end(); it++)
  {
    // std::cout << "getting\n";
    // printf("%d,%d,%d,%d\n", it->first, it->second.lot, it->second.grid, it->second.state);
    if (it->second.state != VEH_STATE_INIT)
    {
      // printf("%d, %d = 1\n", it->second.lot, bit(it->second.grid-1));
      gridState[it->second.lot - 1] |= bit(it->second.grid - 1);
      // printf("lot:%d\n", (int)gridState[it->second.lot -1]);
    }
  }

  return gridState;
}

int ParkingLot::printMessage(const char *msg, bool erase)
{
#ifdef USE_PXA_IO
  int ret = 0;
  if (erase)
  {
    ret = ioctl(this->lcd_fd, LCD_IOCTL_CLEAR, NULL);
  }
  display.Count = sprintf((char *)display.Msg, "%s", msg);
  return ioctl(this->lcd_fd, LCD_IOCTL_WRITE, &display);
#else
  return write(conn_fd, msg, strlen(msg));
#endif
}

int ParkingLot::getMessage(char *msg)
{
#ifdef USE_PXA_IO
  int ret, cnt = 0;
  unsigned short key;
  char c = 0;
  while (1)
  {
    ret = ioctl(lcd_fd, KEY_IOCTL_WAIT_CHAR, &key);
    c = key & 0xff;

    if (c == '#')
      break;
    else
    {
      msg[cnt++] = c;
      display.Count = sprintf((char *)display.Msg, "%c", c);
      ret = ioctl(lcd_fd, LCD_IOCTL_WRITE, &display);
    }
  };
#else
  int n;
  if ((n = read(conn_fd, msg, MSG_BUFSIZE)) == 0)
  {
    // printf("Connection closed\n");
    close(conn_fd);
    // sysState = PL_STATE_WAIT_CONNECTION;
    return -1;
  }
  if (msg[n - 1] == '\n')
  {
    msg[n - 1] = '\0';
  }
  else
  {
    msg[n] = '\0';
  }
  printf("cmd> %s\n", msg);
  return n - 1;
#endif
}
#ifdef USE_PXA_IO
int ParkingLot::segment_display_decimal(int data)
{
  int i;
  _7seg_info_t seg_data;
  unsigned long result_num = 0;
  for (i = 1; i <= 4096; i *= 16)
  {
    // printf("%d^%d, ", data%10, (i));
    result_num += (data % 10) * (i);
    data /= 10;
  }
  // printf("%04lx\n", result_num);

  seg_data.Mode = _7SEG_MODE_HEX_VALUE;
  seg_data.Which = _7SEG_ALL;
  seg_data.Value = result_num;
  return ioctl(lcd_fd, _7SEG_IOCTL_SET, &seg_data);
}

int ParkingLot::led_display_binary(int data)
{
  int i, ret = 0;
  printf("displaying:%d \n", data);
  for (i = 0; i < 8; i++)
  {
    if (data % 2)
      ret |= ioctl(lcd_fd, LED_IOCTL_BIT_CLEAR, &i);
    else
      ret |= ioctl(lcd_fd, LED_IOCTL_BIT_SET, &i);
    data /= 2;
  }
  return ret;
}
#endif

void ParkingLot::logStatus(const char *log_file)
{
  int log_fd, n;
  char msg_buf[MSG_BUFSIZE];
  log_fd = open(log_file, O_CREAT | O_RDWR);
  printf("logging to file\n");
  for (std::map<int, VehicleInfo>::iterator it = vehList.begin(); it != vehList.end(); it++)
  {
    if (it->second.state != VEH_STATE_INIT)
    {
      n = sprintf(msg_buf, "%d,%d,%d\n", it->second.lot, it->second.grid, it->first);
      n = write(log_fd, msg_buf, n);
      printf("%s", msg_buf);
    }
  }
  n = sprintf(msg_buf, "Total income : $%d\n", balance);
  n = write(log_fd, msg_buf, n);
  printf("%s", msg_buf);
  close(log_fd);
}
