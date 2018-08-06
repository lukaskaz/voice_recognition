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
		easyvr(): error(0), serial("/dev/serial0") { initialize(); };
		~easyvr() { std::cerr<<"Destroying VR class!!"<<std::endl; set_baudrate(12); };

		void set_timeout(int timeout);
		void set_sd_sensitive(int level);
		void set_baudrate(int baud_id);
		int get_fw_version(void);
		void play_voice_info(int type);

		int recognize_trigger(void);
		int recognize_user(void);
		int recognize_password(void);
		int recognize_exit(void);

		void increment_session(void) { session_nb++; }
		int get_session(void) const { return session_nb; }

	private:
		static int baudrate;
		static int session_nb;
		int error;
		std::string serial;

		void initialize(void) { initialize_terminal(); adjust_baudrate(); initialize_vr(); }
		void initialize_terminal(void);
		void initialize_vr(void);
		void adjust_baudrate(void);
		char transfer_sequence(char* req, size_t size);
		char transfer_data(char req);
		char get_argument(void);
		void error_handler(char resp);
};

#endif

