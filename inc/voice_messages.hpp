#ifndef VOICE_MESSAGES_H
#define VOICE_MESSAGES_H

typedef enum {
    VOICE_MSG_BEEP                  = 0,
    VOICE_MSG_HELLO_USER            = 1,
    VOICE_MSG_GIVE_NAME             = 2,
    VOICE_MSG_NOT_KNOW_YOU          = 3,
    VOICE_MSG_REPEAT                = 4,
    VOICE_MSG_CANNOT_RECOGNIZE_YOU  = 5,
    VOICE_MSG_NICE_TO_SEE_YOU       = 6,
    VOICE_MSG_LUKASZ                = 7,
    VOICE_MSG_GIVE_PASS             = 8,
    VOICE_MSG_WRONG_PASS            = 9,
    VOICE_MSG_ABORTING_ACCESS       = 10,
    VOICE_MSG_AUTH_ENDED            = 11,
    VOICE_MSG_ACCESS_GRANTED        = 12,

    VOICE_MSG_PRESENT_MENU          = 13,
    VOICE_MSG_MENU_MAIN             = 14,
    VOICE_MSG_MENU_SESSION          = 15,
    VOICE_MSG_MENU_INTERFACE        = 16,
    VOICE_MSG_MENU_SYSTEM           = 17,
    VOICE_MSG_MENU_VOLUME           = 18,
    VOICE_MSG_MENU_SERVOS           = 19,
    VOICE_MSG_MENU_LED              = 20,
    VOICE_MSG_MENU_SIGNAL           = 21,

    VOICE_MSG_GIVE_CMD_OR_HELP      = 22,
    VOICE_MSG_AVAIL_COMMANDS        = 23,
    VOICE_MSG_EXIT_MENU             = 24,

    VOICE_MSG_CLOSE                 = 25,
    VOICE_MSG_CLOSING               = 26,

// MENU MAIN

// MENU SESSION
    VOICE_MSG_LOGOUT                = 27,
    VOICE_MSG_LOGGING_OUT           = 28,

// MENU INTERFACE
    //25 CLOSE
    //26 16 CLOSING INTERFACE

// MENU SYSTEM
    //25 CLOSE
    //26 17 CLOSING SYSTEM

// MENU VOLUME
	VOICE_MSG_DECREASE              = 29,
	VOICE_MSG_INCREASE              = 30,

    VOICE_MSG_DECREASING            = 31,
    VOICE_MSG_INCREASING            = 32,
    VOICE_MSG_BY_20_PERC            = 33,
    VOICE_MSG_VOLUME_GAIN           = 34,
    VOICE_MSG_MINIMAL               = 35,
    VOICE_MSG_MAXIMAL               = 36,

// MENU SERVOS
    VOICE_MSG_ENERGISE              = 37,
    VOICE_MSG_DISENGAGE             = 38,
    VOICE_MSG_MANUAL_CONTROL        = 39,

    VOICE_MSG_ENERGISING            = 40,
    VOICE_MSG_DISENGAGING           = 41,
    VOICE_MSG_GIVING_MANUAL_CONTROL = 42,

// LED	
    VOICE_MSG_SEL_COLOR             = 43,
    VOICE_MSG_SEL_RED               = 44,
    VOICE_MSG_SEL_GREEN             = 45,
    VOICE_MSG_SEL_BLUE              = 46,

    VOICE_MSG_ENABLING_COLOR        = 47,
    VOICE_MSG_ENA_RED               = 48,
    VOICE_MSG_ENA_GREEN             = 49,
    VOICE_MSG_ENA_BLUE              = 50,

// SIGNAL
    VOICE_MSG_ACTIVATE_SIGNAL       = 51,

    VOICE_MSG_ACTIVATING_SIGNAL     = 52,

// MENUS END

    VOICE_MSG_NOT_UNDERSTAND        = 53,
    VOICE_MSG_TIMEOUT               = 54,
    VOICE_MSG_BYE_USER              = 55,
} voice_messages_t;

#endif //VOICE_MESSAGES_H

