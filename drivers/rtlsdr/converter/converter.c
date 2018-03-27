/*
 * Copyright (C) 2018 by IMDEA Networks Institute
 *
 * This file is part of Electrosense.
 *
 * Electrosense is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Electrosense is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RTL-Spec.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 	Authors:
 * 	    Bertold
 *
 */
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "converter.h"


static ssize_t converterWrite(converter* driver, char* buffer, ssize_t len){
	ssize_t index = 0;
	while(index<len){
		ssize_t inc = write(driver->fd, buffer+index, len-index);
		if(inc <= 0){
			return inc;
		}
		index += inc;
	}
	return len;
}

static int converterOpenPort(converter* driver){
	if(driver->open == true){
		driver->open = false;
		close(driver->fd);
	}

	driver->fd = open(driver->portPath,  O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
	int retVal = 0;
	if(driver->fd < 0){
		return -1;
	}

	/* Most settings are ignored by the device */
	struct termios termios;
    if(tcgetattr(driver->fd, &termios) < 0){
    	retVal = -1;
    	goto error;
	}
    termios.c_cflag &= ~CRTSCTS;
    cfmakeraw(&termios);
    termios.c_lflag |= ICANON;
    termios.c_cc[VEOL] = '\n';
    termios.c_cc[VEOL2] = '\n';
	if(tcsetattr(driver->fd, TCSANOW, &termios) < 0){
		retVal = -1;
		goto error;
	}

	/* Start pressing enter */
	for(int i=0; i<5; i++){
		converterWrite(driver, "\r\n",2);
	}

	struct pollfd fds[1];
	fds[0].fd = driver->fd;
	fds[0].events = POLLIN;

	bool deviceReply = false;
	do{
		retVal = poll(fds, 1, 250);
		if(retVal && (fds[0].revents & POLLIN)){
			/* We received something */
			deviceReply = true;
			char buf[8];
			if((retVal = read(driver->fd, buf, sizeof(buf))) <= 0){
				goto error;
			}
		}
	}while(retVal);

	if(retVal < 0) goto error;

	if(!deviceReply){
		retVal = -1;
		errno = EIO;
		goto error;
	}

error:
	if(retVal < 0){
		close(driver->fd);
	}else{
		driver->open = true;
	}

	return retVal;
}

static bool converterBlindCommand(converter* driver, char* command){
	bool writeOk = true;
	writeOk &= (converterWrite(driver, command, strlen(command)) > 0);
	writeOk &= (converterWrite(driver, "\r\n", 2) > 0);
	return writeOk;
}

static bool converterDoCommand(converter* driver, char* command, char* responseStart, char* responseBuffer, unsigned int responseBufferSize){
	bool writeOk = true;
	writeOk &= (converterWrite(driver, command, strlen(command)) > 0);
	writeOk &= (converterWrite(driver, "\r\n", 2) > 0);

	if(!writeOk){
		return false;
	}

	/*
	 * Read replies. Keep reading until we find the desired response (or timeout)
	 */
	struct pollfd fds[1];
	fds[0].fd = driver->fd;
	fds[0].events = POLLIN;

	for(;;){
		int retVal = poll(fds, 1, 250);
		if(retVal && (fds[0].revents & POLLIN)){
			/* Read response */
			if((retVal = read(driver->fd, responseBuffer, responseBufferSize-1)) <= 0){
				return false;
			}
			responseBuffer[retVal] = 0;

			/* Suited? */
			if(!strncmp(responseBuffer, responseStart, strlen(responseStart))){
				return true;
			}
		}
		if(retVal <= 0){
			return false;
		}
	}

	return false;
}

bool converterTune(converter* driver, uint64_t inputFrequency, uint64_t* ifFrequency, bool* invert){
	char replyBuffer[100];
	snprintf(replyBuffer, sizeof(replyBuffer), "convert setup %llu", inputFrequency);
	if(!converterDoCommand(driver, replyBuffer, "IF Frequency:", replyBuffer, sizeof(replyBuffer))){
		return false;
	}

	char* nextPart;
	uint32_t tmp = strtoul(replyBuffer+14, &nextPart, 10);
	if(ifFrequency) *ifFrequency = tmp;

	tmp = strtoul(nextPart+16, NULL, 10);
	if(invert) *invert = tmp;

	return true;
}

bool converterInit(converter* driver){
	if(converterOpenPort(driver) != 0){
		return false;
	}

	char replyBuffer[100];
	/* Enable SoF killing */
	if(!converterDoCommand(driver, "sof 1", "SoF Count", replyBuffer, sizeof(replyBuffer))){
		return false;
	}

	if(!converterBlindCommand(driver,"convert hs5g 1")){
		return false;
	}

	/* Set to bypass mode */
	return converterTune(driver, 100000, NULL, NULL);
}

