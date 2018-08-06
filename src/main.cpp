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

int main(int argc, char* argv[])
{
	signal(SIGINT, signal_handler);
	std::cout<<"Starting EasyVR control program!"<<std::endl;

	easyvr vr;

	std::cout<<"EasyVR FW version is: "<<vr.get_fw_version()<<std::endl;
	vr.set_baudrate(1);
	vr.set_timeout(0);
	vr.set_sd_sensitive(5);

	while(1) {
		std::cout<<std::endl<<"START >> Listening for trigger word to activate!!"<<std::endl;
		if(vr.recognize_trigger() != 0) {
			continue;
		}

		vr.set_timeout(10);
		vr.set_sd_sensitive(3);

		vr.play_voice_info(1);
		vr.play_voice_info(2);

		int user = (-1);
		std::string user_name;
		int retry = 3;

		while(1) {
			user = vr.recognize_user();
			if(user >= 0) {
				break;
			}

			if(retry) {
				retry--;
				vr.play_voice_info(7);
			}
			else {
				vr.play_voice_info(10);
				break;
			}
		}

		if(user >= 0) {
			vr.play_voice_info(3);
			switch(user) {
				case 0:
					user_name = "lukasz";
					vr.play_voice_info(11);
					break;
				case 1:
					user_name = "renata";
					vr.play_voice_info(12);
					break;
				case 2:
					user_name = "adela";
					vr.play_voice_info(13);
					break;
				case 3:
					user_name = "jerzy";
					vr.play_voice_info(14);
					break;
				default:
					break;
			}

			int pass = (-1);	
			vr.play_voice_info(4);

			retry = 3;
			while(1) {
				pass = vr.recognize_password();
				if(pass < 0) {
					if(retry) {
						retry--;
						vr.play_voice_info(8);
					}
					else {
						vr.play_voice_info(10);
						break;
						//exit(1);
					}
				}
				else {
					break;
				}
			}

			if(user == pass) {
				std::string py_script("python -u /home/lukasz/Desktop/work/rainbow-hat/examples/led_text.py -u " + user_name + " >/dev/null &");

				//std::cerr<<py_script.c_str()<<std::endl;
				//system("python -u /home/lukasz/Desktop/work/rainbow-hat/examples/led_text.py -u lukasz &>/dev/null &");
				system(py_script.c_str());
				vr.play_voice_info(6);

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
			}
			else {
				vr.play_voice_info(9);
			}
		}
		else {
			vr.play_voice_info(9);
		}


		vr.set_timeout(5);
		vr.set_sd_sensitive(5);

		vr.play_voice_info(0);
		int exit_cmd = vr.recognize_exit();
		if(exit_cmd == 0) {
				std::cerr<<"Closing VR application!!"<<std::endl;
				break;
		}
		else if(exit_cmd == 1) {
				std::cerr<<"RPI system shutdown!!"<<std::endl;
				vr.~easyvr();
				sleep(2);
				system("halt -p");
		}
		else {
			vr.set_timeout(0);
		}
	}

	return 0;
}
