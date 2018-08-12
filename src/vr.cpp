#include <iostream>
#include <string>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> 

#include "protocol.hpp"
#include "vr.hpp"

static const int INFINITE_WAIT = (-1);
static const int STANDARD_TIMEOUT_MS = 100;

const bool easyvr::PRINT_INFO_TRACES  = true;
const bool easyvr::PRINT_DEBUG_TRACES = true;
const bool easyvr::PRINT_ERROR_TRACES = true;

void easyvr::print_info(const std::string& source, const std::string& txt)
{
    if(PRINT_INFO_TRACES == true) {
        std::cerr<<"[INFO]["<<source<<"] "<<txt<<std::endl;
    }
}

void easyvr::print_debug(const std::string& source, const std::string& txt)
{
    if(PRINT_DEBUG_TRACES == true) {
        std::cerr<<"[DEBUG]["<<source<<"] "<<txt<<std::endl;
    }
}

void easyvr::print_error(const std::string& source, const std::string& txt, int error_code = 0)
{
    if(PRINT_ERROR_TRACES == true) {
        if(error_code) {
            std::cerr<<"[ERROR]["<<source<<"] "<<txt<<", code: " \
                <<std::showbase<<std::hex<<error_code<<std::dec<<std::endl;
        }
        else {
            std::cerr<<"[ERROR]["<<source<<"] "<<txt<<std::endl;
        }
    }
}

void easyvr::initialize_serial(void)
{
    std::string init_serial_cmd("stty clocal -F " + serial);

    print_debug(__func__, init_serial_cmd);
    system(init_serial_cmd.c_str());
}

void easyvr::initialize_baudrate(void)
{
    char resp = '\0';
    static const int alt_baudrate = bd38400;

    resp = transfer_data(CMD_ID, STANDARD_TIMEOUT_MS);
    print_debug(__func__, std::string("Resp: ") + resp);

    if(resp == STS_ID || resp == STS_AWAKEN) {
        print_info(__func__, std::string("Default baudrate is set (9600bd), ") + \
            "changing to alternative baud(" + std::to_string(alt_baudrate) + ")");
        set_baudrate(alt_baudrate);
    }
    else {
        print_info(__func__, std::string("Alternative baudrade is already set(") + \
            std::to_string(alt_baudrate) + "), updating environment variables!");
        baudrate = get_baudrate(alt_baudrate);
    }
}

void easyvr::initialize_vr(void)
{
    char resp = '\0';

    do {
        resp = transfer_data(CMD_BREAK, STANDARD_TIMEOUT_MS);
    } while(resp != STS_SUCCESS && resp != STS_INTERR);

    print_debug(__func__, std::string("Resp: ") + resp);
}

void easyvr::relase_vr(void)
{
    print_debug(__func__, "VR is being released, so restoring default settings!");
    set_baudrate(bddefault);
}

void easyvr::error_handler(char resp)
{
    int err_code = (-1);

    if(resp == STS_ERROR) {
        err_code = get_argument();
        if(err_code) {
            err_code <<= 4;
            err_code |= get_argument();
        }

        print_error(__func__, "Error detected", err_code);
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
        const char req[] =
        {
            CMD_BAUDRATE,
            (char)(ARG_ZERO + baud_id)
        };
        char resp = '\0';

        resp = transfer_sequence(req, sizeof(req), STANDARD_TIMEOUT_MS);
        print_debug(__func__, std::string("Resp: ") + resp);

        baudrate = tmp;
    }
}

void easyvr::set_timeout(int timeout)
{
    const char req[] =
    {
        CMD_TIMEOUT,
        (char)(ARG_ZERO + timeout)
    };
    char resp = '\0';

    resp = transfer_sequence(req, sizeof(req), STANDARD_TIMEOUT_MS);
    print_debug(__func__, std::string("Resp: ") + resp);
}

void easyvr::set_sd_sensitive(int level)
{
    char req[] =
    {
        CMD_LEVEL,
        (char)(ARG_ZERO + level)
    };
    char resp = '\0';

    resp = transfer_sequence(req, sizeof(req), STANDARD_TIMEOUT_MS);
    print_debug(__func__, std::string("Resp: ")  + resp);
}

int easyvr::recognize_trigger(void)
{
    int trig = (-1);
    static const int trigger_group = 0;
    char req[] = 
    {
        CMD_RECOG_SD,
        (char)(ARG_ZERO + trigger_group)
    };
    char resp = '\0';

    req[1] += trigger_group;
    resp = transfer_sequence(req, sizeof(req), INFINITE_WAIT);

    print_debug(__func__, std::string("Resp: ")  + resp);

    if(resp == STS_RESULT) {
        trig = get_argument();
    }
    else {
        error_handler(resp);
    }

    print_debug(__func__, "Trigger: " + std::to_string(trig));

    return trig;
}

int easyvr::recognize_user(void)
{
    int user = (-1);
    static const int users_group = 1;
    static const char req[] =
    {
        CMD_RECOG_SD, 
        (char)(ARG_ZERO + users_group)
    };
    char resp = '\0';

    resp = transfer_sequence(req, sizeof(req), INFINITE_WAIT);
    print_debug(__func__, std::string("Resp: ") + resp);

    if(resp == STS_RESULT) {
        user = get_argument();
    }
    else {
        error_handler(resp);
    }

    print_debug(__func__, "User: " + std::to_string(user));

    return user;
}

int easyvr::recognize_password(void)
{
    int pass = (-1);
    static const int pass_group = 16;
    static const char req[] =
    {
        CMD_RECOG_SD,
        (char)(ARG_ZERO + pass_group)
    };
    char resp = '\0';

    resp = transfer_sequence(req, sizeof(req), INFINITE_WAIT);
    print_debug(__func__, std::string("Resp: ") + resp);

    if(resp == STS_RESULT) {
        pass = get_argument();
    }
    else {
        error_handler(resp);
    }

    print_debug(__func__, "Password: " + std::to_string(pass));

    return pass;
}

int easyvr::recognize_exit(void)
{
    int cmd = (-1);
    static const int exit_cmds_group = 2;
    static const char req[] =
    {
        CMD_RECOG_SD,
        (char)(ARG_ZERO + exit_cmds_group)
    };
    char resp = '\0';

    resp = transfer_sequence(req, sizeof(req), INFINITE_WAIT);
    print_debug(__func__, std::string("Resp: ") + resp);

    if(resp == STS_RESULT) {
        cmd = get_argument();
    }
    else {
        error_handler(resp);
    }

    print_debug(__func__, "Command: " + std::to_string(cmd));

    return cmd;
}

void easyvr::play_voice_info(int type)
{
    static const int volume_gain = 31;
    const char req[] =
    {
        CMD_PLAY_SX,
        ARG_ZERO,
        (char)(ARG_ZERO + type),
        (char)(ARG_ZERO + volume_gain)
    };
    char resp = '\0';

    resp = transfer_sequence(req, sizeof(req), INFINITE_WAIT);
    print_debug(__func__, std::string("Resp: ") + resp);
}

int easyvr::get_fw_version(int& fw_ver)
{
    int ret = (-1);
    char resp = '\0';

    resp = transfer_data(CMD_ID, STANDARD_TIMEOUT_MS);
    if(resp == STS_ID) {
        char tmp = '\0';

        tmp = get_argument();
        if(tmp != '\0') {
            fw_ver = (int)tmp;
            ret = 0;
        }
    }
    print_debug(__func__, std::string("Resp: ") + resp);

    return ret;
}

char easyvr::get_argument(void)
{
    int data = '\0', byte = '\0';

    byte = transfer_data(ARG_ACK, STANDARD_TIMEOUT_MS);
    if(byte != '\0') {
        data = byte - ARG_ZERO;
    }

    return data;
}

void easyvr::wait_for_trigger(void)
{
    set_timeout(0);
    set_sd_sensitive(5);

    while(1) {
        print_info(__func__, "START >> Listening for trigger word to activate!!");
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
                auth_sess_nb++;
                print_info(__func__, std::string("Successfull access no.: ") + \
                                        std::to_string(auth_sess_nb) + " !!");
                play_voice_info(6);
                
                ret = 0;
            }
        }
    }

    if(ret != 0) {
        non_auth_sess_nb++;
        print_info(__func__, std::string("Invalid access no.: ") + \
                                    std::to_string(non_auth_sess_nb));
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
        print_info(__func__, "Closing VR application!!");
        ret = 0;
    }
    else if(selected_cmd == 1) {
        print_info(__func__, "RPI system shutdown!!");
        relase_vr();

        sleep(2);
        system("halt -p");
        ret = 0;
    }
    else if(selected_cmd == 2){
        print_info(__func__, "Command to keep going received, continuing!!");
        ret = 0;
    }
    else if(selected_cmd == 3){
        print_info(__func__, "Command to logout received, logging out!!");
        ret = 0;
    }
    else {
        print_info(__func__, "No command given, continuing!!");
    }

    return ret;
} 

char easyvr::transfer_data(const char req, int timeout_ms)
{
    return transfer_sequence(&req, 1, 100);
}

char easyvr::transfer_sequence(const char* req, ssize_t size, int timeout_ms)
{
    char resp = '\0';

    int fd = open(serial.c_str(), O_RDWR | O_NOCTTY );
    if(fd >= 0) {
        struct termios options;
        tcgetattr(fd, &options);
        options.c_cflag = baudrate | CS8 | CLOCAL | CREAD;  //<Set baud rate
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd, TCSANOW, &options);

        while(1) {
            if(write(fd, req, size) == size) {
                struct pollfd pollInfo = { .fd = fd, .events = POLLIN, .revents = 0 };
                int ret = poll(&pollInfo, 1, timeout_ms);

                if(ret > 0 && (pollInfo.revents & POLLIN) != 0) {
                    char byte = '\0';

                    if(read(fd, &byte, 1) == 1) {
                        if(byte != STS_INVALID) {
                            resp = byte;
                            break;
                        }

                        print_error(__func__, "Unrecognized sequence transfer, repeating...");
                        usleep(5000);
                    }
                }
                else if(ret == 0) {
                    print_error(__func__, "Timeout of request sending has occured");
                    break;
                }
            }
        }

        close(fd);
    }

    return resp;
}
