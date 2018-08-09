#include <iostream>
#include <string>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> 

#include <signal.h>

#include "vr.hpp"


void signal_handler(int sig)
{
    std::cout<<"Restoring default baudrate!"<<std::endl;

    easyvr vr;
    vr.~easyvr();

    std::cout<<"Exitting program!"<<std::endl;
    exit(0);
}



static void get_user_name(int user, std::string& name)
{
    switch(user) {
        case 0:
            name = std::string("Lukasz");
            break;
        case 1:
            name = std::string("Renata");
            break;
        case 2:
            name = std::string("Adela");
            break;
        case 3:
            name = std::string("Jerzy");
            break;
        default:
            name = std::string("N/A");
            break;
    }
}

typedef enum {
    LOGIN_PROCESS_UNDEF = 0,
    LOGIN_PROCESS_START,
    LOGIN_PROCESS_KILL
} Login_Process_Op_t;

static int auth_user_process(int user, Login_Process_Op_t op)
{
    int ret = (-1);

    switch(op) {
        case LOGIN_PROCESS_START: {
            std::string user_name;
            
            get_user_name(user, user_name);
            std::string py_script("python -u /home/lukasz/Desktop/work/rainbow-hat/examples/led_text.py -u " + user_name + " >/dev/null &");

            //std::cerr<<py_script.c_str()<<std::endl;
            //system("python -u /home/lukasz/Desktop/work/rainbow-hat/examples/led_text.py -u lukasz &>/dev/null &");
            system(py_script.c_str());
            sleep(1);
            ret = system("ps aux|grep -i led_text.py | grep -iv grep >/dev/null");

            //~ if(ret == 0) {
                //~ int delay = 15;
                //~ std::cerr<<"Demo login duration left: "<<delay<<" s ";
                //~ while(1) {
                    //~ sleep(1);

                    //~ delay--;
                    //~ if(delay) {
                        //~ if(delay < 9) {
                            //~ std::cerr<<"\b\b\b\b"<<delay<< " s ";
                        //~ }
                        //~ else {
                            //~ std::cerr<<"\b\b\b\b\b"<<delay<< " s ";
                        //~ }
                    //~ }
                    //~ else {
                        //~ std::cerr<<"\b\b\b\b"<<delay<< " s "<<std::endl;;
                        //~ break;
                    //~ }
                //~ }

                //~ system("pkill -2 -f led_text.py");
            //~ }

            break;
        }
        case LOGIN_PROCESS_KILL:
            ret = system("pkill -2 -f led_text.py");
            break;
        default:
            // unsupported operation, ignore
            break;
    }

    return ret;
}

#define IS_EXIT(state)        ((state == 0) || (state == 1))
#define IS_LOGOUT(state)        (state == 3)

int main(int argc, char* argv[])
{
    signal(SIGINT, signal_handler);
    std::cout<<"Starting EasyVR control program!"<<std::endl;

    easyvr vr;
    int vr_fw_ver = (-1);

    if(!vr.get_fw_version(vr_fw_ver)) {
        std::cout<<"EasyVR firmware version is: "<<vr_fw_ver<<std::endl;
    }
    else {
        std::cout<<"Cannot receive EasyVR firmaware!"<<std::endl;
    }

    int state = (-1);
    while(IS_EXIT(state) == 0) {
        state = (-1);

        if(!vr.authenticate()) {
            auth_user_process(vr.get_user_idx(), LOGIN_PROCESS_START);

            while(IS_EXIT(state) == 0 && IS_LOGOUT(state) == 0) {
                if(!vr.handle_commands()) {
                    state = vr.get_selected_cmd();
                }
            }

            auth_user_process((-1), LOGIN_PROCESS_KILL);
        }
    }

    return 0;
}
