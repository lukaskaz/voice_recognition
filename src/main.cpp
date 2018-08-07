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

static int get_user(easyvr& vr)
{
    int user = (-1);
    int retry_question = 3;

    vr.set_timeout(10);
    vr.set_sd_sensitive(3);

    vr.play_voice_info(2);
    while(1) {
        user = vr.recognize_user();
        if(user >= 0) {
            break;
        }

        if(retry_question) {
            retry_question--;
            vr.play_voice_info(7);
        }
        else {
            vr.play_voice_info(10);
            break;
        }
    }

    return user;
}

static int get_password(easyvr& vr)
{
    int pass = (-1);
    int retry_question = 3;

    vr.set_timeout(10);
    vr.set_sd_sensitive(3);

    vr.play_voice_info(4);
    while(1) {
        pass = vr.recognize_password();
        if(pass >= 0) {
            break;
        }

        if(retry_question) {
            retry_question--;
            vr.play_voice_info(8);
        }
        else {
            vr.play_voice_info(10);
            break;
        }
    }

    return pass;
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

static int process_authentication(easyvr& vr, int user, int pass)
{
    int access = 0;

    if(user == pass) {
        vr.play_voice_info(6);

        std::string user_name;
        
        get_user_name(user, user_name);
        std::string py_script("python -u /home/lukasz/Desktop/work/rainbow-hat/examples/led_text.py -u " + user_name + " >/dev/null &");

        //std::cerr<<py_script.c_str()<<std::endl;
        //system("python -u /home/lukasz/Desktop/work/rainbow-hat/examples/led_text.py -u lukasz &>/dev/null &");
        system(py_script.c_str());
        

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
                vr.increment_session();
                std::cerr<<"\b\b\b\b"<<delay<< " s "<<std::endl;;
                std::cerr<<"Completed session number: "<<vr.get_session()<<" !!"<<std::endl;
                break;
            }
        }

        system("pkill -2 -f led_text.py");
        access = 1;
    }
    else {
        vr.play_voice_info(9);
    }

    return access;
}   

static void greet_user(easyvr& vr, int user)
{
    int user_name_record = (-1);

    vr.play_voice_info(3);
    switch(user) {
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
        vr.play_voice_info(user_name_record);
    }
}

static int process_system_commands(easyvr& vr)
{
    int ret = 0;

    vr.set_timeout(5);
    vr.set_sd_sensitive(5);

    vr.play_voice_info(0);
    int exit_cmd = vr.recognize_exit();
    if(exit_cmd == 0) {
            std::cerr<<"Closing VR application!!"<<std::endl;
            ret = 1;
    }
    else if(exit_cmd == 1) {
            std::cerr<<"RPI system shutdown!!"<<std::endl;
            vr.~easyvr();
            sleep(2);
            system("halt -p");
    }
    else if(exit_cmd == 2){
        std::cerr<<"No exit command received, continuing!!"<<std::endl;
    }
    else {
        std::cerr<<"No command given, continuing!!"<<std::endl;
    }

    return ret;
}

static void wait_for_trigger(easyvr& vr)
{
    vr.set_timeout(0);
    vr.set_sd_sensitive(5);

    while(1) {
        std::cout<<std::endl<<"START >> Listening for trigger word to activate!!"<<std::endl;
        if(vr.recognize_trigger() == 0) {
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    signal(SIGINT, signal_handler);
    std::cout<<"Starting EasyVR control program!"<<std::endl;

    easyvr vr;

    std::cout<<"EasyVR FW version is: "<<vr.get_fw_version()<<std::endl;
    vr.set_baudrate(1);

    while(1) {
        wait_for_trigger(vr);

        vr.play_voice_info(1);
        int user = get_user(vr);
        if(user >= 0) {
            greet_user(vr, user);
            
            int pass = get_password(vr);
            if(process_authentication(vr, user, pass) == 1) {
                if(process_system_commands(vr) == 1) {
                    break;
                }
            }
        }
        else {
            vr.play_voice_info(9);
        }
    }

    return 0;
}
