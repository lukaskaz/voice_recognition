#ifndef VR_H
#define VR_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> 

#include <unordered_map>
#include <forward_list>

#include "voice_messages.hpp"
#include "voice_commands.hpp"

#define SERIAL_NODE_NAME    "/dev/serial0"

class easyvr;

typedef struct {
	int level;
	int intro;
	bool intro_in_loop;
	std::forward_list<voice_messages_t> help_infos;
	int (easyvr::*callback)(int);
} menu_data_t;

class easyvr {
    public:
        static easyvr& get_instance(void) { static easyvr instance; return instance; }
	    
	int get_fw_version(int& ver);
        int authenticate(void);
        int handle_commands();

        int get_user_idx(void) const { return user_idx; }
        int get_selected_cmd(void) const { return selected_cmd; }

        void play_voice_info(int type, int vol);
        int menu(void);
        int get_menu_sel(void);
        int get_submenu_sel(void);

    private:
        easyvr(): baudrate(B9600), auth_sess_nb(0), non_auth_sess_nb(0), user_idx(-1), 
                    pass_idx(-1), error(0), serial(SERIAL_NODE_NAME) { initialize(); };
        ~easyvr() { release_vr(); };

        enum bauds { bd115200 = 1, bd57600 = 2, bd38400 = 3, bd19200 = 6, bd9600 = 12, bddefault = bd9600 };
        int baudrate;
        int auth_sess_nb;
        int non_auth_sess_nb;

        int user_idx;
        int pass_idx;
        int selected_cmd;
        
        int error;
        std::string serial;

        static int volume_gain;
	static std::unordered_map<int, menu_data_t> menus;
	
	static const bool SHOW_INFO_TRACES;
        static const bool SHOW_DEBUG_TRACES;
        static const bool SHOW_ERROR_TRACES;
	
        void reset(void); 	
        void initialize(void) { reset(); initialize_serial(); initialize_baudrate(); initialize_vr(); }
        void initialize_serial(void);
        void initialize_baudrate(void);
        void initialize_vr(void);
        void release_vr(void);

        char transfer_sequence(const char* req, ssize_t size, int timeout_ms);
        char transfer_data(const char req, int timeout_ms);
        char get_argument(void);
        void error_handler(char resp);

        int get_baudrate(int baud_id);
        void set_baudrate(int baud_id);
        void set_timeout(int timeout);
        void set_sd_sensitive(int level);

        int recognize_trigger(void);
        int recognize_user(void);
        int recognize_password(void);
        int recognize_exit(void);
        void play_voice_info(int type);

        void wait_for_trigger(void);
        int get_user(void);
        void greet_user(void);
	void bye_user(void);
	int get_password(void);

        void print_info(const std::string& source, const std::string& txt);
        void print_debug(const std::string& source, const std::string& txt);
        void print_error(const std::string& source, const std::string& txt, int code);

	int submenu_interface(int);
	int submenus_dispatcher(int);
	int submenu_session(int sel);
	int submenu_servos(int sel);
	int submenu_system(int sel);
	int submenu_volume(int sel);
	int submenu_led(int sel);
	int submenu_signal(int sel);
};

#endif
