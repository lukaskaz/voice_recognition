#ifndef VR_H
#define VR_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> 

class easyvr {
    public:
        easyvr(int (*fn)(int)): easyvr() { user_fn = fn; }
        easyvr(): user_idx(-1), pass_idx(-1), user_fn(NULL), error(0), serial("/dev/serial0") { initialize(); };
        ~easyvr() { std::cerr<<"Destroying VR class!!"<<std::endl; set_baudrate(12); };

        void set_timeout(int timeout);
        void set_sd_sensitive(int level);
        void set_baudrate(int baud_id);
        int get_fw_version(void);
        void play_voice_info(int type);

        void wait_for_trigger(void);
        int get_user(void);
        void greet_user(void);
        int get_password(void);
        int process_authentication(void);
        int process_system_commands(void);

        void increment_session(void) { session_nb++; }
        int get_session(void) const { return session_nb; }

    private:
        static int baudrate;
        static int session_nb;

        int user_idx;
        int pass_idx;
        int (*user_fn)(int);

        int error;
        std::string serial;
        
        void initialize(void) { initialize_terminal(); adjust_baudrate(); initialize_vr(); }
        void initialize_terminal(void);
        void initialize_vr(void);
        char transfer_sequence(char* req, ssize_t size);
        char transfer_data(char req);
        char get_argument(void);
        void error_handler(char resp);

        void adjust_baudrate(void);
        int recognize_trigger(void);
        int recognize_user(void);
        int recognize_password(void);
        int recognize_exit(void);
};

#endif

