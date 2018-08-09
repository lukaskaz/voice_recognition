#include <iostream>
#include <string>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> 

#include "vr.hpp"

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
        std::cerr<<"Default baudrate is set (9600bd), changing to alternative baud!"<<std::endl;
        set_baudrate(3);
    }
    else {
        std::cerr<<"Alternative baudrade is already set, updating env variables!"<<std::endl;
        baudrate = get_baudrate(3);
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

int easyvr::get_baudrate(int baud_id)
{
    int baud = B9600;

    switch(baud_id) {
        case 1:
            baud = B115200;
            break;
        case 2:
            baud = B57600;
            break;
        case 3:
            baud = B38400;
            break;
        case 6:
            baud = B19200;
            break;
        case 12:
            baud = B9600;
            break;
        default:
            break;
    }

    return baud;
}

void easyvr::set_baudrate(int baud_id)
{
    int tmp = get_baudrate(baud_id);

    if(tmp != baudrate) {
        char req[2] = { 'a', 'A'};
        char resp = '\0';

        req[1] += baud_id;
        resp = transfer_sequence(req, sizeof(req));

        std::cerr<<"Resp: "<<resp<<std::endl;
        baudrate = tmp;
    }
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

int easyvr::get_fw_version(int& fw_ver)
{
    int ret = (-1);
    char resp = '\0';

    resp = transfer_data('x');
    if(resp == 'x') {
        char tmp = '\0';

        tmp = get_argument();
        if(tmp != '\0') {
            fw_ver = (int)tmp;
            ret = 0;
        }
    }
    std::cerr<<"Resp: "<<resp<<std::endl;

    return ret;
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

void easyvr::wait_for_trigger(void)
{
    set_timeout(0);
    set_sd_sensitive(5);

    while(1) {
        std::cout<<std::endl<<"START >> Listening for trigger word to activate!!"<<std::endl;
        if(recognize_trigger() == 0) {
            break;
        }
    }
}

int easyvr::get_user(void)
{
    int ret = (-1);
    int retry_question = 3;

    set_timeout(10);
    set_sd_sensitive(3);

    play_voice_info(2);
    while(1) {
        user_idx = recognize_user();
        if(user_idx >= 0) {
            ret = 0;
            break;
        }

        if(retry_question--) {
            play_voice_info(7);
        }
        else {
            play_voice_info(10);
            break;
        }
    }

    return ret;
}

void easyvr::greet_user(void)
{
    int user_name_record = (-1);

    play_voice_info(3);
    switch(user_idx) {
        case 0:
            user_name_record = 11;
            break;
        case 1:
            user_name_record = 12;
            break;
        case 2:
            user_name_record = 13;
            break;
        case 3:
            user_name_record = 14;
            break;
        default:
            break;
    }

    if(user_name_record >= 0) {
        play_voice_info(user_name_record);
    }
}

int easyvr::get_password(void)
{
    int ret = (-1);
    int retry_question = 3;

    set_timeout(10);
    set_sd_sensitive(3);

    play_voice_info(4);
    while(1) {
        pass_idx = recognize_password();
        if(pass_idx >= 0) {
            ret = 0;
            break;
        }

        if(retry_question--) {
            play_voice_info(8);
        }
        else {
            play_voice_info(10);
            break;
        }
    }

    return ret;
}

int easyvr::authenticate(void)
{
    int ret = (-1);

    wait_for_trigger();
    
    play_voice_info(1);
    if(!get_user()) {
        greet_user();

        if(!get_password()) {
            if(user_idx == pass_idx) {
                incr_auth_session();
                std::cerr<<"Successfull access no.: "<<get_auth_session()<<" !!"<<std::endl;
                play_voice_info(6);
                
                ret = 0;
            }
        }
    }

    if(ret != 0) {
        incr_nonauth_session();
        std::cerr<<"Invalid access no.: "<<get_nonauth_session()<<std::endl;
        play_voice_info(9);
    }

    return ret;
} 

int easyvr::handle_commands()
{
    int ret = (-1);

    wait_for_trigger();

    set_timeout(5);
    set_sd_sensitive(5);

    play_voice_info(0);

    selected_cmd = recognize_exit();
    if(selected_cmd == 0) {
            std::cerr<<"Closing VR application!!"<<std::endl;
            ret = 0;
    }
    else if(selected_cmd == 1) {
            std::cerr<<"RPI system shutdown!!"<<std::endl;
            set_baudrate(12);

            sleep(2);
            system("halt -p");
            ret = 0;
    }
    else if(selected_cmd == 2){
        std::cerr<<"Command to keep going received, continuing!!"<<std::endl;
        ret = 0;
    }
    else if(selected_cmd == 3){
        std::cerr<<"Command to logout received, logging out!!"<<std::endl;
        ret = 0;
    }
    else {
        std::cerr<<"No command given, continuing!!"<<std::endl;
    }

    return ret;
} 

char easyvr::transfer_data(char req)
{
    char resp = '\0';

    int fd = open(serial.c_str(), O_RDWR | O_NOCTTY );
    if(fd >= 0) {
        struct termios options;
        tcgetattr(fd, &options);
        options.c_cflag = baudrate | CS8 | CLOCAL | CREAD;		//<Set baud rate
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd, TCSANOW, &options);

        while(1) {
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

char easyvr::transfer_sequence(char* req, ssize_t size)
{
    char resp = '\0';

    int fd = open(serial.c_str(), O_RDWR | O_NOCTTY );
    if(fd >= 0) {
        struct termios options;
        tcgetattr(fd, &options);
        options.c_cflag = baudrate | CS8 | CLOCAL | CREAD;		//<Set baud rate
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd, TCSANOW, &options);

        while(1) {
            if(write(fd, req, size) == size) {
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
