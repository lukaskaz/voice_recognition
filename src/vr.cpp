#include <iostream>
#include <string>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> 

#include "vr.hpp"

int easyvr::session_nb = 0;
int easyvr::baudrate = B9600;

void easyvr::initialize_terminal(void)
{
	std::string init_serial_tty("stty clocal -F " + serial);

	std::cerr<<"Init: "<<init_serial_tty<<std::endl;
	system(init_serial_tty.c_str());
}

void easyvr::adjust_baudrate(void)
{
	char resp = '\0';

	resp = transfer_data('x');
	std::cerr<<"Resp: "<<resp<<std::endl;
	
	if(resp == 'x' || resp == 'w') {
		std::cerr<<"Keeping default baudrate 9600bd"<<std::endl;
		baudrate = B9600;
	}
	else {
		std::cerr<<"Switching baudrate to 115200bd"<<std::endl;
		baudrate = B115200;
	}
}

void easyvr::initialize_vr(void)
{
	char resp = '\0';

	while(resp != 'o' && resp != 'i') {
		resp = transfer_data('b');
	}

	std::cerr<<"Resp: "<<resp<<std::endl;
}

void easyvr::error_handler(char resp)
{
	int err_type = (-1);

	if(resp == 'e') {
		err_type = get_argument();
		if(err_type) {
			err_type <<= 4;
			err_type |= get_argument();
		}

		std::cerr<<"Error: "<<std::showbase<<std::hex<<err_type<<std::dec<<std::endl;
	}

	initialize_vr();
}

void easyvr::set_baudrate(int baud_id)
{
	char req[2] = { 'a', 'A'};
	char resp = '\0';

	req[1] += baud_id;
	resp = transfer_sequence(req, sizeof(req));

	std::cerr<<"Resp: "<<resp<<std::endl;
	int tmp = B9600;

	switch(baud_id) {
		case 1:
			tmp = B115200;
			break;
		default:
			break;	
	}

	baudrate = tmp;
}

void easyvr::set_timeout(int timeout)
{
	char req[2] = { 'o', 'A'};
	char resp = '\0';

	req[1] += timeout;
	resp = transfer_sequence(req, sizeof(req));

	std::cerr<<"Resp: "<<resp<<std::endl;
}

void easyvr::set_sd_sensitive(int level)
{
	char req[2] = { 'v', 'A'};
	char resp = '\0';

	req[1] += level;
	resp = transfer_sequence(req, sizeof(req));

	std::cerr<<"Resp: "<<resp<<std::endl;
}

int easyvr::recognize_trigger(void)
{
	int trig = (-1);
	char req[2] = { 'd', 'A' };
	char resp = '\0';

	resp = transfer_sequence(req, sizeof(req));

	std::cerr<<"Resp: "<<resp<<std::endl;

	if(resp == 'r') {
		trig = get_argument();
	}
	else {
		error_handler(resp);
	}

	std::cerr<<"Trigger: "<<trig<<std::endl;

	return trig;
}

int easyvr::recognize_user(void)
{
	int user = (-1);
	char req[2] = { 'd', 'B' };
	char resp = '\0';

	resp = transfer_sequence(req, sizeof(req));

	std::cerr<<"Resp: "<<resp<<std::endl;

	if(resp == 'r') {
		user = get_argument();
	}
	else {
		error_handler(resp);
	}

	std::cerr<<"User: "<<user<<std::endl;

	return user;
}

int easyvr::recognize_password(void)
{
	int pass = (-1);
	char req[2] = { 'd', 'Q' };
	char resp = '\0';

	resp = transfer_sequence(req, sizeof(req));

	std::cerr<<"Resp: "<<resp<<std::endl;

	if(resp == 'r') {
		pass = get_argument();
	}
	else {
		error_handler(resp);
	}

	std::cerr<<"Password: "<<pass<<std::endl;

	return pass;
}

int easyvr::recognize_exit(void)
{
	int cmd = (-1);
	char req[2] = { 'd', 'C' };
	char resp = '\0';

	resp = transfer_sequence(req, sizeof(req));

	std::cerr<<"Resp: "<<resp<<std::endl;

	if(resp == 'r') {
		cmd = get_argument();
	}
	else {
		error_handler(resp);
	}

	std::cerr<<"Cmd: "<<cmd<<std::endl;

	return cmd;
}

void easyvr::play_voice_info(int type)
{
	char req[4] = { 'w', 'A', 'A', '`' };
	char resp = '\0';

	req[2] += type;
	resp = transfer_sequence(req, sizeof(req));

	std::cerr<<"Resp: "<<resp<<std::endl;
}


int easyvr::get_fw_version(void)
{
	int version = (-1);

	if(transfer_data('x') == 'x') {
		char tmp = '\0';

		tmp = get_argument();
		if(tmp != '\0') {
			version = (int)tmp;	
		}
	}

	return version;
}

char easyvr::get_argument(void)
{
	int data = '\0', byte = '\0';

	byte = transfer_data(' ');
	if(byte != '\0') {
		data = byte - 'A';
	}

	return data;
}

char easyvr::transfer_data(char req)
{
	char resp = '\0';

	int fd = open(serial.c_str(), O_RDWR | O_NOCTTY );
	if(fd >= 0) {
		struct termios options;
		tcgetattr(fd, &options);
		options.c_cflag = baudrate | CS8 | CLOCAL | CREAD;		//<Set baud rate
		//options.c_cflag = B9600 | CS8 | CREAD;		//<Set baud rate
		options.c_iflag = IGNPAR;
		options.c_oflag = 0;
		options.c_lflag = 0;
		tcflush(fd, TCIFLUSH);
		tcsetattr(fd, TCSANOW, &options);	

		while(1) {
			//usleep(1000);
			if(write(fd, &req, 1) == 1) {
				int timoutMs = 500;
				struct pollfd pollInfo = { .fd = fd, .events = POLLIN, .revents = 0 };
				int ret = poll(&pollInfo, 1, timoutMs);

				if(ret > 0 && (pollInfo.revents & POLLIN) != 0) {
					char byte = '\0';
				
					if(read(fd, &byte, 1) == 1) {
						if(byte != 'v') {
							resp = byte;
							break;
						}

						std::cerr<<"Unrecognized data transfer, repeating..."<<std::endl;
						usleep(5000);
					}
				}
				else {
					break;
				}
			}
		}
	
		close(fd);	
	}

	return resp;
}

char easyvr::transfer_sequence(char* req, size_t size)
{
	char resp = '\0';

	int fd = open(serial.c_str(), O_RDWR | O_NOCTTY );
	if(fd >= 0) {
		struct termios options;
		tcgetattr(fd, &options);
		options.c_cflag = baudrate | CS8 | CLOCAL | CREAD;		//<Set baud rate
		//options.c_cflag = B9600 | CS8 | CREAD;		//<Set baud rate
		options.c_iflag = IGNPAR;
		options.c_oflag = 0;
		options.c_lflag = 0;
		tcflush(fd, TCIFLUSH);
		tcsetattr(fd, TCSANOW, &options);	

		while(1) {
		//	if(write(fd, req, size) == size) {
			if(1) {
				for(unsigned int i=0; i<size; i++) {
					usleep(100);
					if(write(fd, &req[i], 1) != 1) {
						return resp;
					}
				}

				int timoutMs = (-1);
				struct pollfd pollInfo = { .fd = fd, .events = POLLIN, .revents = 0 };
				int ret = poll(&pollInfo, 1, timoutMs);

				if(ret > 0 && (pollInfo.revents & POLLIN) != 0) {
					char byte = '\0';
				
					if(read(fd, &byte, 1) == 1) {
						if(byte != 'v') {
							resp = byte;
							break;
						}

						std::cerr<<"Unrecognized sequence transfer, repeating..."<<std::endl;
						usleep(5000);
					}
				}
			}
		}
	
		close(fd);	
	}

	return resp;
}

