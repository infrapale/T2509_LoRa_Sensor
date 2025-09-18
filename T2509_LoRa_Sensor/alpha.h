#ifndef __ALPHA_H__ 
#define __ALPHA_H__

#define SHOW_MAX_STR_LEN    40

typedef enum
{
    ALPHA_SHOW_UNDEFINED    = 0b0000000000000000,
    ALPHA_SHOW_FLOAT        = 0b0000000000000001,
    ALPHA_SHOW_INTEGER      = 0b0000000000000010,
    ALPHA_SHOW_STR4         = 0b0000000000000100,
} alpha_event_et;

typedef struct
{
    uint16_t        event;
    uint16_t        default_event;
    float           fval;
    uint16_t        ival;
    char            str[SHOW_MAX_STR_LEN];
    uint32_t        timeout;
    uint32_t        new_timeout;
} alpha_ctrl_st;


void alpha_initialize(void);
void alpha_show_str(char *str);
void alpha_show_float(float value);

void alpha_show_float_event(float fval, uint32_t timeout, bool set_default);
void alpha_show_integer_event(uint16_t ival, uint32_t timeout, bool set_default);
void alpha_show_str4_event(const char *str4, uint32_t timeout, bool set_default);

#endif