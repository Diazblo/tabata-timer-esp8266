#ifndef __DISPLAY__H__
#define __DISPLAY__H__

#include "LedController.hpp"
#include "preprocessor_helper.h"
#include "board_defs.h"

LedController<1, 1> lc;

#define UNIT_SECONDS 0
#define UNIT_TIME 1
#define UNIT_CHARS 2

#define BAR_BLACK B000
#define BAR_RED B100
#define BAR_YELLOW B110
#define BAR_GREEN B010
#define BAR_CYAN B011
#define BAR_BLUE B001
#define BAR_MAGENTA B101
#define BAR_WHITE B111

#define HEX_RED(HEX_VAL) ((HEX_VAL >> 16) & 0xFF)
#define HEX_GREEN(HEX_VAL) ((HEX_VAL >> 8) & 0xFF)
#define HEX_BLUE(HEX_VAL) ((HEX_VAL)&0xFF)
#define HEX_THRESH 127

uint8_t loading_counter = 0;
byte loading_anim[] = {
    B00000001,
    B00000100,
    B00001000,
    B00010000,
};
void display_loading()
{
    lc.setRow(0, 2, loading_anim[loading_counter]);
    loading_counter++;
    if(loading_counter == 4)loading_counter=0;
}

void display_bar_clear()
{
    lc.setRow(0, 7, 0);
    lc.setRow(0, 5, 0);
    lc.setRow(0, 6, 0);
}

void display_clear()
{
    lc.setRow(0, 0, 0);
    lc.setRow(0, 1, 0);
    lc.setRow(0, 2, 0);
    lc.setRow(0, 3, 0);
    lc.setRow(0, 4, 0);
}

byte bootanim_chars[]={B00001000, B00000001, B01000000};
void display_bootanim()
{   
    for(uint8_t k = 0; k < 5; k++) 
    for(uint8_t j  = 0; j < 3; j++)
    for (uint8_t i = 5; i > k; i--)
    {
        lc.setRow(0, i, B00000000);
        byte output = lc.getRow(0,i-1) | bootanim_chars[j];
        lc.setRow(0,i-1, output);
        delay(50);
    }

    lc.setChar(0,1,'U',false);
    lc.setChar(0,2,'A',false);
    delay(2000);
}

void display_bar_set(byte t_led_register, byte t_color = B111, bool t_clear = false)
{
    byte led_register_corr = (((t_led_register & 1) << 7) | (t_led_register >> 1)) & 0XFF;

    if (t_clear)
        display_bar_clear();

    if ((t_color & BAR_RED) >> 2)
        lc.setRow(0, 7, led_register_corr);
    if ((t_color & BAR_GREEN) >> 1)
        lc.setRow(0, 5, led_register_corr);
    if ((t_color & BAR_BLUE))
        lc.setRow(0, 6, led_register_corr);
}

void display_bar(uint16_t t_progress, byte t_color, bool t_progress_reverse = false, uint16_t t_progress_total = 8)
{
    uint8_t m_progress = (t_progress * 8.0) / t_progress_total;
    uint8_t m_led_register = (B11111111 << (8 - m_progress));
    if (t_progress_reverse)
        m_led_register = (B11111111 >> (8 - m_progress));

    display_bar_set(m_led_register, t_color, true);
}

void display_init()
{
    lc = LedController<1, 1>(DIN, CLK, CS);
    lc.setIntensity(15);
    lc.clearMatrix();
}
void display_sleep()
{
    // lc.clearMatrix();
    lc.shutdownAllSegments();
}

void display_text(String t_text, bool t_dp = 0)
{
    uint8_t m_max = t_text.length();
    if (m_max > 5)
        m_max = 5;

    display_clear();
    for (uint8_t i = 0; i < m_max; i++)
    {
        lc.setChar(0, i, t_text[i], t_dp);
    }
}

uint8_t display_blink=0;
void display_time(int t_input, uint8_t t_blink = false)
{
    uint8_t m_seconds = t_input % 60;
    uint8_t m_minutes = t_input % (60 * 60) / 60;
    uint8_t m_hours = t_input / 3600;

    lc.setDigit(0, 0, (m_hours % 10), true);

    lc.setDigit(0, 1, (m_minutes % 100) / 10, false);
    lc.setDigit(0, 2, (m_minutes % 10 / 1), true);

    lc.setDigit(0, 3, (m_seconds % 100) / 10, false);
    lc.setDigit(0, 4, (m_seconds % 10 / 1), false);

    if(t_blink){
        display_blink++;
        if(display_blink > 4){
            display_blink = 0;
            if(t_blink == 1){
                display_clear();
            }
            if(t_blink == 2){
                lc.setChar(0, 3, '_', false);
                lc.setChar(0, 4, '_', false);
            }
            if(t_blink == 3){
                lc.setChar(0, 1, '_', false);
                lc.setChar(0, 2, '_', false);
            }
            if(t_blink == 4){
                lc.setChar(0, 0, '_', false);
            }
        }
    }
}
void display_seconds(int32_t t_input)
{   
    uint8_t nn = 1;
    uint8_t offset = 0;
    if(t_input<0){ 
        lc.setChar(0,0,'-', false);
        t_input = -t_input;
        offset = 1;
        }
    for (uint8_t i = 5; i > offset; i--)
    {
        lc.setDigit(0, i - 1 , (t_input % (nn * 10)) / nn, false);
        nn = nn * 10;
    }
}

#endif //!__DISPLAY__H__