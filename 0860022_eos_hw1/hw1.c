#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"

void led_set(char *binary, int fd);
void seg_set(int n, int fd);
int input_number(char input[]);
int get(int fbb);

int main()
{
    unsigned short key;
    unsigned short led;
    int fd, ret;

    int number[3][8] = {0}; // 0 = no parking
    int reserve[3][8] = {0};// 0 = no reserve
    char input[4] = "0000";     // keyin_number
    unsigned char index = 0;
    int value;                  // keyin_number value
    char status;                // car_status
    lcd_write_info_t display;
    char binary[8] = {0};       // led
    int car_in_grid = 0;
    int park_idel[3] = {0};
    int led_grid_state[8] = {0};

    int i, j, aaa;
    int k, l;
    bool keyin_number = true;
    bool select_manu = false;
    bool choose_lot = false;
    bool choose_grid = false;
    int lot,grid;
    int tmp;
    bool wait = true;

    /* Open device /dev/lcd */
    if ((fd = open("/dev/lcd", O_RDWR)) == -1)
    {
        printf("Open /dev/lcd faild.\n");
        exit(-1);
    }
    /*Clear device*/
    ioctl(fd, LED_IOCTL_SET, &led);
    ioctl(fd, KEY_IOCTL_CLEAR, key);
    ioctl(fd, LCD_IOCTL_CLEAR, NULL);
    printf("start\n");

    while (1)
    {
        display.Count = sprintf((char *)display.Msg, "");
        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
        display.Count = sprintf((char *)display.Msg, "Input plate num:");
        ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/

        while (keyin_number)
        {
            ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
            if (ret < 0)
            {
                sleep(1);
                continue;
            }
            ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);

            switch (key & 0xff)
            {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    display.Count = sprintf((char *)display.Msg, "%c", key & 0xff);
                    input[index++] = key & 0xff;
                    break;
                case '#':
                    value = input_number(input);
                    seg_set(value, fd);
                    printf("%d \n", value);
                    index = 0;

                    for (i = 0; i < 3; i++)
                    {
                        for (j = 0; j < 8; j++)
                        {
                            tmp = abs(number[i][j]);
                            if (tmp == value)
                            {
                                if (number[i][j] < 0)
                                {
                                    status = 'S';
                                    goto here;
                                }
                                else
                                {
                                    status = 'P';
                                    goto here;
                                }
                            }
                            else
                            {
                                status = 'N';
                            }
                        }
                    }
                    here:
                    printf("%c \n", status);

                    display.Count = sprintf((char *)display.Msg, "");
                    ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/

                    keyin_number = false;
                    select_manu = true;
                    break;
                default:
                    break;
            }

            ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
        }

        while (select_manu)
        {
            ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
            if (ret < 0)
            {
                sleep(1);
                continue;
            }
            ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);

            if (status == 'N')
            {
                display.Count = sprintf((char *)display.Msg, "");
                ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                display.Count = sprintf((char *)display.Msg, "You haven't reserved grid. \n1. show \n2. reserve \n3. check-in \n4. exit \n");
                ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                switch (key & 0xff)
                {
                    case '1':
                        for(k = 0; k < 3; k++)
                        {
                            car_in_grid = 0;
                            for(l = 0; l < 8; l++)
                            {
                                if(number[k][l] == 0)
                                {
                                    car_in_grid++;
                                }
                            }
                            park_idel[k] = car_in_grid;
                        }
                        display.Count = sprintf((char *)display.Msg, "");
                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                        display.Count = sprintf((char *)display.Msg, "P1 %d \nP2 %d \nP3 %d",park_idel[0],park_idel[1],park_idel[2]);
                        ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                        break;
                    case '2':
                        choose_lot = true;
                        display.Count = sprintf((char *)display.Msg, "");
                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                        while (choose_lot)
                        {
                            ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                            if (ret < 0)
                            {
                                sleep(1);
                                continue;
                            }
                            ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);

                            display.Count = sprintf((char *)display.Msg, "Select parking lot:\n");
                            ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/

                            switch (key & 0xff)
                            {
                                case '1':
                                case '2':
                                case '3':
                                    display.Count = sprintf((char *)display.Msg, "");
                                    ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                                    display.Count = sprintf((char *)display.Msg, "Select parking grid:\n");
                                    ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/

                                    lot = ((key & 0xff)-49);
                                    for(j = 0; j < 8; j++)
                                    {
                                        binary[j] = 0;
                                        if(number[lot][j] == 0)
                                        {
                                            binary[j] = 1;
                                        }
                                    }
                                    led_set(binary, fd);
                                    

                                    choose_grid = true;
                                    while (choose_grid)
                                    {
                                        ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                                        if (ret < 0)
                                        {
                                            sleep(1);
                                            continue;
                                        }
                                        ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);

                                        display.Count = sprintf((char *)display.Msg, "");
                                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/

                                        switch (key & 0xff)
                                        {
                                            case '1':
                                            case '2':
                                            case '3':
                                            case '4':
                                            case '5':
                                            case '6':
                                            case '7':
                                            case '8':
                                                grid = ((key & 0xff)-49);
                                                if (number[lot][grid] == 0)
                                                {
                                                    number[lot][grid] = -value;
                                                    display.Count = sprintf((char *)display.Msg, "Have a nice day!\n");
                                                    ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/

                                                    reserve[lot][grid] = 1;
                                                    choose_lot = false;
                                                    choose_grid = false;
                                                }
                                                else
                                                {
                                                    display.Count = sprintf((char *)display.Msg, "Error! Please select an ideal grid.\nReturn menu.\n");
                                                    ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                                                    choose_lot = false;
                                                    choose_grid = false;
                                                }
                                                break;
                                            default:
                                                break;
                                        }
                                    }
                                    break;
                                default:
                                    break;
                            }
                        }
                        keyin_number = true;
                        select_manu = false;
                        break;
                    case '3':
                        choose_lot = true;
                        display.Count = sprintf((char *)display.Msg, "");
                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                        while (choose_lot)
                        {
                            ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                            if (ret < 0)
                            {
                                sleep(1);
                                continue;
                            }
                            ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);

                            display.Count = sprintf((char *)display.Msg, "Select parking lot:\n");
                            ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/

                            switch (key & 0xff)
                            {
                                case '1':
                                case '2':
                                case '3':
                                    display.Count = sprintf((char *)display.Msg, "");
                                    ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                                    display.Count = sprintf((char *)display.Msg, "Select parking grid:\n");
                                    ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/

                                    lot = ((key & 0xff)-49);
                                    for(j = 0; j < 8; j++)
                                    {
                                        binary[j] = 0;
                                        if(number[lot][j] == 0)
                                        {
                                            binary[j] = 1;
                                        }
                                    }
                                    led_set(binary, fd);
                                    

                                    choose_grid = true;
                                    while (choose_grid)
                                    {
                                        ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                                        if (ret < 0)
                                        {
                                            sleep(1);
                                            continue;
                                        }
                                        ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);

                                        display.Count = sprintf((char *)display.Msg, "");
                                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/

                                        switch (key & 0xff)
                                        {
                                            case '1':
                                            case '2':
                                            case '3':
                                            case '4':
                                            case '5':
                                            case '6':
                                            case '7':
                                            case '8':
                                                grid = ((key & 0xff)-49);
                                                if (number[lot][grid] == 0)
                                                {
                                                    number[lot][grid] = value;
                                                    display.Count = sprintf((char *)display.Msg, "Have a nice day!\n");
                                                    ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                                                    choose_lot = false;
                                                    choose_grid = false;
                                                }
                                                else
                                                {
                                                    display.Count = sprintf((char *)display.Msg, "Error! Please select an ideal grid.\nReturn menu.\n");
                                                    ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                                                    choose_lot = false;
                                                    choose_grid = false;
                                                }
                                                break;
                                            default:
                                                break;
                                        }
                                    }
                                    break;
                                default:
                                    break;
                            }
                        }
                        keyin_number = true;
                        select_manu = false;
                        break;
                    case '4':
                        display.Count = sprintf((char *)display.Msg, "");
                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                        keyin_number = true;
                        select_manu = false;
                        break;
                    default:
                        break;
                }
            }
            else if (status == 'S')
            {
                display.Count = sprintf((char *)display.Msg, "");
                ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                display.Count = sprintf((char *)display.Msg, "You have reserved grid. \n1. show \n2. cancel \n3. check-in \n4. exit \n");
                ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                switch (key & 0xff)
                {
                    case '1':
                        for(k = 0; k < 3; k++)
                        {
                            car_in_grid = 0;
                            for(l = 0; l < 8; l++)
                            {
                                if(number[k][l] == 0)
                                {
                                    car_in_grid++;
                                }
                            }
                            park_idel[k] = car_in_grid;
                        }
                        display.Count = sprintf((char *)display.Msg, "");
                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                        display.Count = sprintf((char *)display.Msg, "P1 %d \nP2 %d \nP3 %d",park_idel[0],park_idel[1],park_idel[2]);
                        ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                        break;
                    case '2':
                        display.Count = sprintf((char *)display.Msg, "");
                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                        display.Count = sprintf((char *)display.Msg, "Reserve fee: $20");
                        ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                        number[i][j] = 0;
                        keyin_number = true;
                        select_manu = false;
                        break;
                    case '3':
                        display.Count = sprintf((char *)display.Msg, "");
                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                        display.Count = sprintf((char *)display.Msg, "Have a nice day!");
                        ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                        number[i][j] = value;
                        keyin_number = true;
                        select_manu = false;
                        break;
                    case '4':
                        display.Count = sprintf((char *)display.Msg, "");
                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                        keyin_number = true;
                        select_manu = false;
                        break;
                    default:
                        break;
                }
            }
            else if (status == 'P')
            {
                display.Count = sprintf((char *)display.Msg, "");
                ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                display.Count = sprintf((char *)display.Msg, "Your grid is at lot p%d grid %d. \n1. show \n2. pick-up \n", (i + 1), (j + 1));
                ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                switch (key & 0xff)
                {
                    case '1':
                        for(k = 0; k < 3; k++)
                        {
                            car_in_grid = 0;
                            for(l = 0; l < 8; l++)
                            {
                                if(number[k][l] == 0)
                                {
                                    car_in_grid++;
                                }
                            }
                            park_idel[k] = car_in_grid;
                        }
                        display.Count = sprintf((char *)display.Msg, "");
                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                        display.Count = sprintf((char *)display.Msg, "P1 %d \nP2 %d \nP3 %d",park_idel[0],park_idel[1],park_idel[2]);
                        ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                        break;
                    case '2':
                        display.Count = sprintf((char *)display.Msg, "");
                        ioctl(fd, LCD_IOCTL_CLEAR, NULL);   /*Clear lcd*/
                        if (reserve[i][j] == 1)
                        {
                            display.Count = sprintf((char *)display.Msg, "Parking fee: $30\n");
                            ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                        }else
                        {
                            display.Count = sprintf((char *)display.Msg, "Parking fee: $40\n");
                            ioctl(fd, LCD_IOCTL_WRITE, &display);   /*lcd display*/
                        }
                        number[i][j] = 0;
                        keyin_number = true;
                        select_manu = false;
                        break;
                    default:
                        break;
                }
            }
        }
        aaa = get(fd);
    }
    close(fd);
}

void seg_set(int n, int fd)
{
    _7seg_info_t data_seg;
    unsigned long seghex = 0;
    int i = 0;

    ioctl(fd, _7SEG_IOCTL_ON, NULL);
    data_seg.Mode = _7SEG_MODE_HEX_VALUE;
    data_seg.Which = _7SEG_ALL;

    printf("output value:%lu \n", n);

    while (n != 0)
    {
        seghex += n % 10 * pow(16, i);
        n /= 10;
        i++;
    }

    data_seg.Value = 0x0000 + seghex;
    ioctl(fd, _7SEG_IOCTL_SET, &data_seg);
}

/* led display */
void led_set(char *binary, int fd)
{
    int i = 0;
    unsigned short led;
    int Led_control = 0;

    /* close all LED */
    led = LED_ALL_OFF;
    ioctl(fd, LED_IOCTL_SET, &led);

    for (i; i < 8; i++)
    {
        led = Led_control + i;
        if (binary[7 - i])
        {
            ioctl(fd, LED_IOCTL_BIT_SET, &led);
        }
    }
}

int input_number(char input[])
{
    int output = 0;
    output = (input[0] - 48) * 1000 + (input[1] - 48) * 100 + (input[2] - 48) * 10 + (input[3] - 48);
    return output;
}

int get(int fbb)
{
    unsigned short key;
    int ret;
    ioctl(fbb, KEY_IOCTL_CLEAR, key);
    while (1)
    {
        ret = ioctl(fbb, KEY_IOCTL_CHECK_EMTPY, &key);

        if (ret < 0)
        {
            sleep(1);
            continue;
        }
        ret = ioctl(fbb, KEY_IOCTL_GET_CHAR, &key);

        switch (key & 0xff)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '#':
                return 1;
                break;
            default:
                return 0;
                break;
        }
    }
}