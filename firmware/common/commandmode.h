#ifndef COMMANDMODE_H
#define COMMANDMODE_H
#define MODE_NORMAL         ((Mode)0)
#define MODE_ENTER_COMMAND  ((Mode)1)
#define MODE_COMMAND        ((Mode)2)
#define MODE_LEAVE_COMMAND  ((Mode)3)

#define CMDMODE_ENTER_KEY   KEY_scrlck
#define CMDMODE_ABORT_KEY   KEY_esc

typedef uint8_t Mode;
#endif /* !COMMANDMODE_H */
