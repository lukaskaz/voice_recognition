#ifndef VR_H
#define VR_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> 

#define SERIAL_NODE_NAME    "/dev/serial0"

class easyvr {
    public:
        easyvr(): baudrate(B9600), auth_sess_nb(0), non_auth_sess_nb(0), user_idx(-1), 
                    pass_idx(-1), error(0), serial(SERIAL_NODE_NAME) { initialize(); };
        ~easyvr() { std::cerr<<"Destroying VR class!!"<<std::endl; set_baudrate(12); };

        int get_fw_version(int& ver);
        int authenticate(void);
        int handle_commands();

        void incr_auth_session(void) { auth_sess_nb++; }
        void incr_nonauth_session(void) { non_auth_sess_nb++; }
        
        int get_user_idx(void) const { return user_idx; }
        int get_auth_session(void) const { return auth_sess_nb; }
        int get_nonauth_session(void) const { return non_auth_sess_nb; }
        int get_selected_cmd(void) const { return selected_cmd; }

    private:
        int baudrate;
        int auth_sess_nb;
        int non_auth_sess_nb;

        int user_idx;
        int pass_idx;
        int selected_cmd;
        
        int error;
        std::string serial;
        
        void initialize(void) { initialize_terminal(); adjust_baudrate(); initialize_vr(); }
        void initialize_terminal(void);
        void initialize_vr(void);
        char transfer_sequence(char* req, ssize_t size);
        char transfer_data(char req);
        char get_argument(void);
        void error_handler(char resp);

        int get_baudrate(int baud_id);
        void set_baudrate(int baud_id);
        void adjust_baudrate(void);
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
        int get_password(void);
};

#endif
