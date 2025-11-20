#ifndef __ALPHA_H__ 
#define __ALPHA_H__

#define LONG_STR_LEN        80
#define SHORT_STR_LEN       10 

typedef enum
{
    ALPHA_CH_TEMPERATURE = 0,
    ALPHA_CH_HUMIDITY,
    ALPHA_CH_MSG_CNTR,
    ALPHA_CH_ERR,
    ALPHA_CH_SENDING,
    ALPHA_CH_ADDR,
    ALPHA_CH_NUMBER,
    ALPHA_CH_LONG_STR,
    ALPHA_CH_NBR_OF,
} alpha_channel_et;

typedef enum
{
    ALPHA_FUNCTION_ONE_TIME = 0,
    ALPHA_FUNCTION_PERMANENT,
    ALPHA_FUNCTION_SCROLLING,
    ALPHA_FUNCTION_NBR_OF,
} alpha_function_et;


typedef struct
{
    bool     active;
    uint32_t duration;
    uint32_t timeout;
    char     str[SHORT_STR_LEN];
    alpha_function_et function;  
} alpha_channel_st;


typedef struct
{
    uint8_t         channel;
    char            long_str[LONG_STR_LEN];
    uint8_t         cursor;
    uint8_t         len;
    uint32_t        timeout;
    bool            ready;
} alpha_ctrl_st;


void alpha_initialize(void);
void alpha_add_float(alpha_channel_et ch, float fval);
void alpha_add_integer(alpha_channel_et ch, uint16_t ival);
void alpha_add_short_str(alpha_channel_et ch, char *buff);
void alpha_add_long_str(alpha_channel_et ch, char *buff);
void alpha_set_brightness(uint8_t br);



// void alpha_show_str(char *str);
// void alpha_show_float(float value);

// void alpha_show_float_event(float fval, uint32_t timeout, bool set_default);
// void alpha_show_integer_event(uint16_t ival, uint32_t timeout, bool set_default);
// void alpha_show_str4_event(const char *str4, uint32_t timeout, bool set_default);
// void alpha_show_long_str_event(char *str, uint32_t timeout);


#endif