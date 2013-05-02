#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "rs232.h"

bool SerialPort::init(void)
{
	int error;

	/* Open UART port */
	port = open("/dev/ttyO1", O_RDWR | O_NOCTTY);
	if (port < 0) {
		perror("unable to open serial port ");
		return false;
	}

	/* Save old port settings */
	error = tcgetattr(port, &old_port_settings);
	if(error < 0) {
		close(port);
		perror("unable to read portsettings ");
		return false;
	}
	
	/* Set new port settings */
	memset(&new_port_settings, 0, sizeof(new_port_settings));  // clear the new struct

	new_port_settings.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
	new_port_settings.c_iflag = IGNPAR;
	new_port_settings.c_oflag = 0;
	new_port_settings.c_lflag = 0;
	new_port_settings.c_cc[VMIN] = 0;      // block untill n bytes are received
	new_port_settings.c_cc[VTIME] = 0;     // block untill a timer expires (n * 100 mSec.)
	error = tcsetattr(port, TCSANOW, &new_port_settings);
	if(error < 0) {
		close(port);
		perror("unable to adjust portsettings ");
		return false;
	}
#ifdef SERIAL_DEBUG
    printf("Serialport initialized \n");
#endif

	return true;
}

bool SerialPort::deinit(void)
{
	/* Close UART port */
	close(port);
	
	/* Restore old settings */
	tcsetattr(port, TCSANOW, &old_port_settings);

#ifdef SERIAL_DEBUG
    printf("Serialport de-initialized \n");
#endif
	
    return true;
}

int SerialPort::send(char *buf, int len)
{
#ifdef SERIAL_DEBUG
    printf("Write started, port: %#x, length: %d, first char: %#x \n", port, len, buf[0]);
#endif
	return write(port, buf, len);
}

int SerialPort::receive(char *buf, int len)
{
    int result;
#ifndef __STRICT_ANSI__ 	// __STRICT_ANSI__ is defined when the -ansi option is used for gcc
	if(len > SSIZE_MAX)
		len = (int) SSIZE_MAX;  // SSIZE_MAX is defined in limits.h
#else
	if(len > 4096)
		len = 4096;
#endif

#ifdef SERIAL_DEBUG
    printf("Read started, port: %#x, length: %d \n", port, len);
#endif
    
	result = read(port, buf, len);
    
#ifdef SERIAL_DEBUG
    printf("Read finished, port: %#x, length: %d, first char: %#x, result: %d \n", port, len, buf[0], result);
#endif
    
    return result;

}
