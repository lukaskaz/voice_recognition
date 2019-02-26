#include <iostream>
#include <string>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> 

#include "protocol.hpp"
#include "voice_messages.hpp"
#include "voice_commands.hpp"
#include "vr.hpp"

static const int INFINITE_WAIT = (-1);
static const int STANDARD_TIMEOUT_MS = 100;
static const int EXTENDED_TIMEOUT_MS = 500;

const bool easyvr::SHOW_INFO_TRACES  = true;
const bool easyvr::SHOW_DEBUG_TRACES = true;
const bool easyvr::SHOW_ERROR_TRACES = true;


#define VOICE_RECOG_MAIN_MENU	0xFF
			
int easyvr::volume_gain = 6;
std::unordered_map<int, menu_data_t> easyvr::menus
({
	{
		VOICE_RECOG_MAIN_MENU,
                {
	        	.intro = VOICE_MSG_MENU_MAIN,
			.help_infos =
			{
				VOICE_MSG_MENU_SESSION, VOICE_MSG_MENU_INTERFACE, 
				VOICE_MSG_MENU_SYSTEM, VOICE_MSG_MENU_VOLUME,
				VOICE_MSG_MENU_SERVOS, VOICE_MSG_MENU_LED,
				VOICE_MSG_MENU_SIGNAL	
			},
			.callback = &easyvr::submenus_dispatcher
		}
	},
	{
		VOICE_CMD_MENU_SESSION,
                {
	        	.intro = VOICE_MSG_MENU_SESSION,
			.help_infos = { VOICE_MSG_LOGOUT },
			.callback = &easyvr::submenu_session
		}
	},
	{
		VOICE_CMD_MENU_INTERFACE, 
		{
			.intro = VOICE_MSG_MENU_INTERFACE,
			.help_infos = { VOICE_MSG_CLOSE },
			.callback = &easyvr::submenu_interface
		}
	},
	{
		VOICE_CMD_MENU_SERVOS, 
		{
			.intro = VOICE_MSG_MENU_SERVOS,
			.help_infos = 
			{ 
				VOICE_MSG_ENERGISE, VOICE_MSG_DISENGAGE, 
				VOICE_MSG_MANUAL_CONTROL  
			},
			.callback = &easyvr::submenu_servos
		}
	},
	{
		VOICE_CMD_MENU_SYSTEM, 
		{
			.intro = VOICE_MSG_MENU_SYSTEM,
			.help_infos = { VOICE_MSG_CLOSE },
			.callback = &easyvr::submenu_system
		}
	},
	{
		VOICE_CMD_MENU_VOLUME, 
		{	
			.intro = VOICE_MSG_MENU_VOLUME,
			.help_infos = { VOICE_MSG_DECREASE, VOICE_MSG_INCREASE },
			.callback = &easyvr::submenu_volume
		}
	},
	{
		VOICE_CMD_MENU_LED, 
		{
			.intro = VOICE_MSG_MENU_LED,
			.help_infos = { VOICE_MSG_SEL_RED, VOICE_MSG_SEL_GREEN, VOICE_MSG_SEL_BLUE },
			.callback = &easyvr::submenu_led
		}
	},
	{
		VOICE_CMD_MENU_SIGNAL, 
		{
			.intro = VOICE_MSG_MENU_SIGNAL,
			.help_infos = { VOICE_MSG_ACTIVATE_SIGNAL },
			.callback = &easyvr::submenu_signal
		}
	}
});

void easyvr::print_info(const std::string& source, const std::string& txt)
{
    if(SHOW_INFO_TRACES == true) {
        std::cerr<<"[INFO]["<<source<<"] "<<txt<<std::endl;
    }
}

void easyvr::print_debug(const std::string& source, const std::string& txt)
{
    if(SHOW_DEBUG_TRACES == true) {
        std::cerr<<"[DEBUG]["<<source<<"] "<<txt<<std::endl;
    }
}

void easyvr::print_error(const std::string& source, const std::string& txt, int error_code = 0)
{
    if(SHOW_ERROR_TRACES == true) {
        if(error_code) {
            std::cerr<<"[ERROR]["<<source<<"] "<<txt<<", code: " \
                <<std::showbase<<std::hex<<error_code<<std::dec<<std::endl;
        }
        else {
            std::cerr<<"[ERROR]["<<source<<"] "<<txt<<std::endl;
        }
    }
}

void easyvr::reset(void)
{
	static const std::string vr_reset_ctrl("/home/lukasz/Desktop/work/os_started_signal/easyvr_reset");

	if(access(vr_reset_ctrl.c_str(), F_OK) == (-1)) {
		print_error(__func__, "Cannot reset easyvr, reset control not exist!");
	}
	else {
		static unsigned int delay_ms = 250;
		std::string command;

		print_debug(__func__, "Resetting easyvr");
		
		command = vr_reset_ctrl + " --state on";
		system(command.c_str());

		usleep(delay_ms * 1000);
		
		command = vr_reset_ctrl + " --state off";
		system(command.c_str());
		
		usleep(delay_ms * 1000);
		print_debug(__func__, "Easyvr reset completed");
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

    resp = transfer_data(CMD_ID, EXTENDED_TIMEOUT_MS);
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

void easyvr::release_vr(void)
{
    print_debug(__func__, "VR is being released, so restoring default settings!");
    set_baudrate(bddefault);

    reset();
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
    static const char req[] =
    {
        CMD_RECOG_SD, 
        (char)(ARG_ZERO + VOICE_CMD_GROUP_USERS)
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
    static const char req[] =
    {
        CMD_RECOG_SD,
        (char)(ARG_ZERO + VOICE_CMD_GROUP_PASSWORDS)
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
    //static const int volume_gain = 7;
    int idxH = type/32, idxL = type%32;
    const char req[] =
    {
        CMD_PLAY_SX,
        (char)(ARG_ZERO + idxH),
        (char)(ARG_ZERO + idxL),
        (char)(ARG_ZERO + volume_gain)
    };
    char resp = '\0';

    resp = transfer_sequence(req, sizeof(req), INFINITE_WAIT);
    print_debug(__func__, std::string("Resp: ") + resp);
}

void easyvr::play_voice_info(int type, int vol)
{
    vol = (vol > 31) ? 31 : vol;
    int idxH = type/32, idxL = type%32;
    const char req[] =
    {
        CMD_PLAY_SX,
        (char)(ARG_ZERO + idxH),
        (char)(ARG_ZERO + idxL),
        (char)(ARG_ZERO + vol)
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
        if(recognize_trigger() == VOICE_CMD_TRIGGER) {
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

    play_voice_info(VOICE_MSG_GIVE_NAME);
    while(1) {
        user_idx = recognize_user();
        if(user_idx >= 0) {
            ret = 0;
            break;
        }

        if(--retry_question) {
            play_voice_info(VOICE_MSG_NOT_KNOW_YOU);
            play_voice_info(VOICE_MSG_REPEAT);
        }
        else {
            play_voice_info(VOICE_MSG_CANNOT_RECOGNIZE_YOU);
            break;
        }
    }

    return ret;
}

void easyvr::greet_user(void)
{
    int user_name_msg = (-1);

    switch(user_idx) {
        case VOICE_CMD_USER_LUKASZ:
            user_name_msg = VOICE_MSG_LUKASZ;
            break;
        default:
            break;
    }

    if(user_name_msg >= 0) {
    	play_voice_info(VOICE_MSG_NICE_TO_SEE_YOU);
        play_voice_info(user_name_msg);
    }
}

void easyvr::bye_user(void)
{
    int user_name_msg = (-1);

    switch(user_idx) {
        case VOICE_CMD_USER_LUKASZ:
            user_name_msg = VOICE_MSG_LUKASZ;
            break;
        default:
            break;
    }

    if(user_name_msg >= 0) {
    	play_voice_info(VOICE_MSG_BYE_USER);
        play_voice_info(user_name_msg);
    }
}

int easyvr::get_password(void)
{
    int ret = (-1);
    int retry_question = 3;

    set_timeout(10);
    set_sd_sensitive(3);

    play_voice_info(VOICE_MSG_GIVE_PASS);
    while(1) {
        pass_idx = recognize_password();
        if(pass_idx >= 0) {
            ret = 0;
            break;
        }

        play_voice_info(VOICE_MSG_WRONG_PASS);
        if(--retry_question) {
            play_voice_info(VOICE_MSG_REPEAT);
        }
        else {
            play_voice_info(VOICE_MSG_ABORTING_ACCESS);
            break;
        }
    }

    return ret;
}

int easyvr::get_menu_sel(void)
{
    int sel = (-1);
    static const char req[] =
    {
        CMD_RECOG_SD, 
        (char)(ARG_ZERO + VOICE_CMD_GROUP_MENUS)
    };
    char resp = '\0';

    resp = transfer_sequence(req, sizeof(req), INFINITE_WAIT);
    print_debug(__func__, std::string("Resp: ") + resp);

    if(resp == STS_RESULT) {
        sel = get_argument();
    }
    else {
        error_handler(resp);
    }

    print_debug(__func__, "Menu: " + std::to_string(sel));

    return sel;
}

int easyvr::get_submenu_sel(void)
{
    int sel = (-1);
    static const char req[] =
    {
        CMD_RECOG_SD, 
        (char)(ARG_ZERO + VOICE_CMD_GROUP_MENU_SEL)
    };
    char resp = '\0';

    resp = transfer_sequence(req, sizeof(req), INFINITE_WAIT);
    print_debug(__func__, std::string("Resp: ") + resp);

    if(resp == STS_RESULT) {
        sel = get_argument();
    }
    else {
        error_handler(resp);
    }

    print_debug(__func__, "Submenu: " + std::to_string(sel));

    return sel;
}

int easyvr::submenu_session(int sel)
{
	int ret = (-1);

	if(sel == VOICE_CMD_MENU_SEL_LOGOUT) {
		play_voice_info(VOICE_MSG_LOGGING_OUT);

		usleep(250*1000);
		bye_user();

        	print_info(__func__, "Logging out this session");
		ret = 3;	
	}

	return ret;
}

int easyvr::submenu_interface(int sel)
{
	int ret = (-1);

	if(sel == VOICE_CMD_MENU_SEL_CLOSE) {
		play_voice_info(VOICE_MSG_CLOSING);
		play_voice_info(VOICE_MSG_MENU_INTERFACE);

		usleep(250*1000);
		bye_user();

        	print_info(__func__, "Exitting VR interface");
		ret = 1;	
	}

	return ret;
}

int easyvr::submenu_servos(int sel)
{
	int ret = 0;

	if(sel == VOICE_CMD_MENU_SEL_ENERGISE) {
		play_voice_info(VOICE_MSG_ENERGISING);

		print_info(__func__, "Command to supply servos!!");
		system("/home/lukasz/Desktop/work/os_started_signal/gpio_ctrl --state on");
	}
	else if(sel == VOICE_CMD_MENU_SEL_DISENGAGE) {
		play_voice_info(VOICE_MSG_DISENGAGING);

		print_info(__func__, "Command to disable servos supply!!");
		system("/home/lukasz/Desktop/work/os_started_signal/gpio_ctrl --state off");
	}
	else if(sel == VOICE_CMD_MENU_SEL_MANUAL_CTRL) {
		play_voice_info(VOICE_MSG_GIVING_MANUAL_CONTROL);

		print_info(__func__, "Activating manual servos control!!");
		system("/home/lukasz/Desktop/servo_test/prog/out/servos_ctrl");
	}
	else {
		ret = (-1);
	}

	return ret;
}

int easyvr::submenu_system(int sel)
{
	int ret = (-1);

	if(sel == VOICE_CMD_MENU_SEL_CLOSE) {
		play_voice_info(VOICE_MSG_CLOSING);
		play_voice_info(VOICE_MSG_MENU_SYSTEM);

		usleep(250*1000);
		bye_user();

        	print_info(__func__, "Shutting down the system");
		ret = 1;	
	}

	return ret;
}

int easyvr::submenu_volume(int sel)
{
	static const int volume_min = 2;
	static const int volume_max = 31;
	static const int step = 6;
	int ret = 0;

	if(sel == VOICE_CMD_MENU_SEL_DECREASE) {
		if(volume_gain == volume_min) {
			play_voice_info(VOICE_MSG_VOLUME_GAIN);
			play_voice_info(VOICE_MSG_MINIMAL);
		}
		else {
			volume_gain = ((volume_gain - step) < volume_min) ? volume_min : (volume_gain - step);
			play_voice_info(VOICE_MSG_DECREASING);
			play_voice_info(VOICE_MSG_BY_20_PERC);
		}
	}
	else if(sel == VOICE_CMD_MENU_SEL_INCREASE) {
		if(volume_gain == volume_max) {
			play_voice_info(VOICE_MSG_VOLUME_GAIN);
			play_voice_info(VOICE_MSG_MAXIMAL);
		}
		else {
			volume_gain = ((volume_gain + step) > volume_max) ? volume_max : (volume_gain + step);
			play_voice_info(VOICE_MSG_INCREASING);
			play_voice_info(VOICE_MSG_BY_20_PERC);
		}
	}
	else {
		ret = (-1);
	}

	print_info(__func__, "Volume is now: " + std::to_string(volume_gain));

	return ret;
}

int easyvr::submenu_led(int sel)
{
	static const std::string rgb_ctrl("/home/lukasz/Desktop/work/os_started_signal/rgb_led");
	int ret = 0;

	if(sel == VOICE_CMD_MENU_SEL_RED   ||
           sel == VOICE_CMD_MENU_SEL_GREEN ||
	   sel == VOICE_CMD_MENU_SEL_BLUE)
	{
		static const char* color[] = { "blue", "green", "red" };
		const char* sel_color = color[10-sel];
					
		play_voice_info(VOICE_MSG_ENABLING_COLOR);
		play_voice_info(VOICE_MSG_ENA_BLUE-(10-sel));
		
		std::string command = rgb_ctrl + " --state " + sel_color;
		system(command.c_str());
		
		print_info(__func__, "Setting rgb in " + std::string(sel_color));
	}
	else if(sel == VOICE_CMD_MENU_SEL_EXIT_MENU) {
        	std::string command = rgb_ctrl + " --state os_default";
		system(command.c_str());
	}
	else {
		ret = (-1);
	}

	return ret;
}

int easyvr::submenu_signal(int sel)
{
	int ret = 0;

	if(sel == VOICE_CMD_MENU_SEL_ACTIVATE) {
		play_voice_info(VOICE_MSG_ACTIVATING_SIGNAL);

        	print_info(__func__, "Activating signal for 10 seconds");
		system("/home/lukasz/Desktop/work/os_started_signal/buzzer_ctrl --state on");
		sleep(10);
		system("/home/lukasz/Desktop/work/os_started_signal/buzzer_ctrl --state off");
	}
	else {
		ret = (-1);
	}

	return ret;
}

int easyvr::submenus_dispatcher(int idx)
{
	int ret = (-1);

	std::unordered_map<int, menu_data_t>::const_iterator submenu = menus.find(idx);
	if(submenu != menus.end()) {
		play_voice_info(VOICE_MSG_PRESENT_MENU);
		play_voice_info(submenu->second.intro);
		
		while(1) {
			play_voice_info(VOICE_MSG_GIVE_CMD_OR_HELP);
			int sel = get_submenu_sel();
		
			if(sel == VOICE_CMD_MENU_SEL_HELP) {
				play_voice_info(VOICE_MSG_AVAIL_COMMANDS);

				for(const voice_messages_t &help_info : submenu->second.help_infos) {
					play_voice_info(help_info);
				}

				play_voice_info(VOICE_MSG_EXIT_MENU);
			}
			else if(sel == VOICE_CMD_MENU_SEL_EXIT_MENU) {
				(this->*submenu->second.callback)(sel);
				break;
			}
			else {
				ret = (this->*submenu->second.callback)(sel);
				if(ret > 0) {
					break;
				}
			}

			usleep(250*1000);
		}
	}

	return ret;
}

int easyvr::menu(void)
{
	int ret = (-1);

	std::unordered_map<int, menu_data_t>::const_iterator mainmenu = menus.find(VOICE_RECOG_MAIN_MENU);
	if(mainmenu != menus.end()) {
	    	set_timeout(25);
    		set_sd_sensitive(3);
	
		while(1) {
			play_voice_info(VOICE_MSG_PRESENT_MENU);
			play_voice_info(mainmenu->second.intro);
			play_voice_info(VOICE_MSG_GIVE_CMD_OR_HELP);

			int sel = get_menu_sel();
			if(sel == VOICE_CMD_MENU_HELP) {
				play_voice_info(VOICE_MSG_AVAIL_COMMANDS);

				for(const voice_messages_t &help_info : mainmenu->second.help_infos) {
					play_voice_info(help_info);
				}
			}
			else {
				ret = (this->*mainmenu->second.callback)(sel);
				if(ret > 0) {
					break;
				}
			}
			
			usleep(250*1000);
		}
	}

	return ret;
}

int easyvr::authenticate(void)
{
    int ret = (-1);

    wait_for_trigger();
    
    play_voice_info(VOICE_MSG_HELLO_USER);
    if(!get_user()) {
        greet_user();

        if(!get_password()) {
            if(user_idx == pass_idx) {
                auth_sess_nb++;
                print_info(__func__, std::string("Successfull access no.: ") + \
                                        std::to_string(auth_sess_nb) + " !!");
                play_voice_info(VOICE_MSG_ACCESS_GRANTED);
                
                ret = 0;
            }
        }
    }

    if(ret != 0) {
        non_auth_sess_nb++;
        print_info(__func__, std::string("Invalid access no.: ") + \
                                    std::to_string(non_auth_sess_nb));
        play_voice_info(VOICE_MSG_AUTH_ENDED);
    }

    return ret;
} 

#include <fstream>

int easyvr::handle_commands()
{
    int ret = (-1);

    wait_for_trigger();

    set_timeout(5);
    set_sd_sensitive(5);

    play_voice_info(VOICE_MSG_BEEP);

    selected_cmd = recognize_exit();
    if(selected_cmd == 0) {
        print_info(__func__, "Closing VR application!!");
        ret = 0;
    }
    else if(selected_cmd == 1) {
        print_info(__func__, "RPI system shutdown!!");
        release_vr();

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
    else if(selected_cmd == 4){
        print_info(__func__, "Command to to activate servo!!");
        system("/home/lukasz/Desktop/work/os_started_signal/gpio_ctrl --state on");
	
        ret = 0;
    }
    else if(selected_cmd == 5){
        print_info(__func__, "Command to deactivate servo!!");
        system("/home/lukasz/Desktop/work/os_started_signal/gpio_ctrl --state off");
    
    	ret = 0;
    }
    else {
        print_info(__func__, "No command given, continuing!!");
    }

    return ret;
} 

char easyvr::transfer_data(const char req, int timeout_ms)
{
    return transfer_sequence(&req, 1, timeout_ms);
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

