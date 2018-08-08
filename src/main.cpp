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
	vr.set_baudrate(12);

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

static int auth_user_process(int user)
{
    std::string user_name;
    
    get_user_name(user, user_name);
    std::string py_script("python -u /home/lukasz/Desktop/work/rainbow-hat/examples/led_text.py -u " + user_name + " >/dev/null &");

    //std::cerr<<py_script.c_str()<<std::endl;
    //system("python -u /home/lukasz/Desktop/work/rainbow-hat/examples/led_text.py -u lukasz &>/dev/null &");
    system(py_script.c_str());
    sleep(1);
    int ret = system("ps aux|grep -i led_text.py | grep -iv grep >/dev/null");

    if(ret == 0) {
        int delay = 15;
        std::cerr<<"Demo login duration left: "<<delay<<" s ";
        while(1) {
            sleep(1);

            delay--;
            if(delay) {
                if(delay < 9) {
                    std::cerr<<"\b\b\b\b"<<delay<< " s ";
                }
                else {
                    std::cerr<<"\b\b\b\b\b"<<delay<< " s ";
                }
            }
            else {
                std::cerr<<"\b\b\b\b"<<delay<< " s "<<std::endl;;
                std::cerr<<"Completed session number: "<</*vr.get_session()*/5<<" !!"<<std::endl;
                break;
            }
        }

        system("pkill -2 -f led_text.py");
    }

    return ret;
}





int main(int argc, char* argv[])
{
    signal(SIGINT, signal_handler);
    std::cout<<"Starting EasyVR control program!"<<std::endl;

    easyvr vr(auth_user_process);

    std::cout<<"EasyVR FW version is: "<<vr.get_fw_version()<<std::endl;
    vr.set_baudrate(3);

    while(1) {
        vr.wait_for_trigger();

        if(!vr.get_user()) {
            vr.greet_user();

            if(!vr.get_password()) {
                if(!vr.process_authentication()) {
                    if(vr.process_system_commands() == 1) {
                        break;
                    }
                }
            }
        }
        else {
            vr.play_voice_info(9);
        }
    }

    return 0;
}
