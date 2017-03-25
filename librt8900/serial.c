//
// Created by cormac on 24/02/17.
//

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "serial.h"
#include "display_packet.h"
#include "log.h"

/// Set our serial port attributes. Radio expects these constants
void set_serial_attributes(int fd)
{
        struct termios tty;
        memset(&tty, 0, sizeof(tty));

        //read in existing settings
        if (tcgetattr(fd, &tty) != 0) {
                log_msg(RT8900_ERROR, "Failed reading terminal settings");
                exit(EXIT_FAILURE);
        }

        //set send and receive baud rates
        cfsetospeed (&tty, B19200);
        cfsetispeed (&tty, B19200);

        //sets out 8n1 without any other fancy options
        //Settings inspired from http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
        tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;         /* 8-bit characters */
        tty.c_cflag &= ~PARENB;     /* no parity bit */
        tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
        tty.c_cflag &= ~CRTSCTS;    /* no hardware flow control */


        /* setup for non-canonical mode */
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
        tty.c_oflag &= ~OPOST;


        // block read calls until we read a whole packet fetch of bytes until they become available
        tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        tty.c_cc[VMIN] = DISPLAY_PACKET_SIZE;
        tty.c_cc[VTIME] = 1;
        // fcntl(fd, F_SETFL, 0); this will make reads blocking

        //write our new settings
        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
                log_msg(RT8900_ERROR, "Failed to set terminal settings");
                exit(EXIT_FAILURE);
        }

}

/// Open and configure a serial port for sending and receiving from radio
void open_serial(int *fd, char * *serial_path)
{
        *fd = 0;
        *fd = open(*serial_path, O_RDWR | O_NOCTTY ); //| O_NDELAY
        if (fd < 0) {
                log_msg(RT8900_ERROR, "Error while opening serial_path %s\n", *serial_path); // Just if you want user interface error control
                exit(EXIT_FAILURE);
        }

        tcflush(*fd, TCIOFLUSH); //flush in/out buffers
        set_serial_attributes(*fd);
}