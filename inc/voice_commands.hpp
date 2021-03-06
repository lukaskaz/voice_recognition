#ifndef VOICE_COMMANDS_H
#define VOICE_COMMANDS_H

typedef enum {
    VOICE_CMD_TRIGGER               = 0,

    VOICE_CMD_USER_LUKASZ           = 0,

    VOICE_CMD_MENU_HELP             = 0,
    VOICE_CMD_MENU_SESSION          = 1,
    VOICE_CMD_MENU_INTERFACE        = 2,
    VOICE_CMD_MENU_SERVOS           = 3,
    VOICE_CMD_MENU_SYSTEM           = 4,
    VOICE_CMD_MENU_VOLUME           = 5,
    VOICE_CMD_MENU_LED              = 6,
    VOICE_CMD_MENU_SIGNAL           = 7,

    VOICE_CMD_MENU_SEL_HELP         = 0,
    VOICE_CMD_MENU_SEL_LOGOUT       = 1,
    VOICE_CMD_MENU_SEL_CLOSE        = 2,
    VOICE_CMD_MENU_SEL_ENERGISE     = 3,
    VOICE_CMD_MENU_SEL_DISENGAGE    = 4,
    VOICE_CMD_MENU_SEL_MANUAL_CTRL  = 5,
    VOICE_CMD_MENU_SEL_DECREASE     = 6,
    VOICE_CMD_MENU_SEL_INCREASE     = 7,
    VOICE_CMD_MENU_SEL_RED          = 8,
    VOICE_CMD_MENU_SEL_GREEN        = 9,
    VOICE_CMD_MENU_SEL_BLUE         = 10,
    VOICE_CMD_MENU_SEL_ACTIVATE     = 11,
    VOICE_CMD_MENU_SEL_EXIT_MENU    = 12,
} voice_commands_t;

typedef enum {
    VOICE_CMD_GROUP_TRIGGER         = 0,
    VOICE_CMD_GROUP_USERS           = 1,
    VOICE_CMD_GROUP_MENUS           = 3,
    VOICE_CMD_GROUP_MENU_SEL        = 4,
    VOICE_CMD_GROUP_PASSWORDS       = 16
} voice_commands_groups_t;

#endif //VOICE_COMMANDS_H

