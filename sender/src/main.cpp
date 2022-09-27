/*!
   @file main.cpp
   @author Wellington Camargo (wellington.camargo@agres.com.br)
   @brief 
   @date 2022-09-27
   
   @copyright Copyright (c) 2022
   
 */

/*********************************************************************************************************************
 * Dependencies
 *********************************************************************************************************************/

#include <Arduino.h>

/*********************************************************************************************************************
 * Constants and macros
 *********************************************************************************************************************/

#define BTN_1           10
#define BTN_2           11
#define BTN_3           12
#define LED             13
#define DEBOUNCE_TIME   5

/*********************************************************************************************************************
 * Struct and types definitions
 *********************************************************************************************************************/

typedef struct Button_s Button_s;

struct Button_s
{
    bool pressed;
    bool last_state;
    bool state;
    bool run_once;
    bool run_state;

    uint8_t pin;
    uint16_t debounce_time;
    uint32_t press_time;

    uint32_t (*getMs)(void);
    bool (*debounce)(Button_s *);
    bool (*btnRead)(Button_s *);
    void (*onClick)(Button_s *);
};

/*********************************************************************************************************************
 * Private function prototypes
 *********************************************************************************************************************/

/*!
   @brief Debounce function
   
   @param self Pointer to a Button "object"
   @return true if debounced
   @return false if not debounced
 */
static bool debounceButton(Button_s *self);

/*!
   @brief Function to read the button status. If configured run_once, the function will return true only
   when the button is pressed (momentarily), otherwise it will keep returning true while the button is pressed
   
   @param self Pointer to a Button "object"
   @return true If the button read status is true
   @return false If the button read status is false
 */
static bool btnRead(Button_s *self);

/*!
    @brief Button "object" constructor
    
    @param self Pointer to a Button "object"
    @param pin Pin number to which the button is to be connected
    @param run_once Configures if the callback function is to be called only when click (true) or if it wil be called
                    while the button is pressed (false)
    @param fn callback function to be called when button pressed
 */
static void Button(Button_s *self, uint8_t pin, bool run_once, void (*fn)(Button_s *));

/*!
   @brief Button 1 callback function
   
   @param self Pointer to a Button "object"
 */
static void button_1_Callback(Button_s *self);

/*!
   @brief Button 2 callback function
   
   @param self Pointer to a Button "object"
 */
static void button_2_Callback(Button_s *self);

/*********************************************************************************************************************
 * Private function definitions
 *********************************************************************************************************************/

static void Button(Button_s *self, uint8_t pin, bool run_once, void (*fn)(Button_s *))
{
    self->pressed = false;
    self->last_state = false;
    self->state = false;
    self->run_once = run_once;
    self->run_state = false;
    self->pin = pin;
    self->debounce_time = DEBOUNCE_TIME;
    self->press_time = 0;

    self->getMs = millis;
    self->debounce = debounceButton;
    self->btnRead = btnRead;
    self->onClick = fn;

    pinMode(self->pin, OUTPUT);
}

static bool debounceButton(Button_s *self)
{
    bool debounce = false;

    if (self->state != self->last_state)
    {
        self->last_state = self->state;
        self->press_time = self->getMs();
    }

    if ((self->state == self->last_state) && ((self->getMs() - self->press_time) >= self->debounce_time))
    {
        debounce = true;
    }

    return debounce;
}

static bool btnRead(Button_s *self)
{
    bool read = false;

    self->state = digitalRead(self->pin);

    if (self->debounce(self))
    {
        self->pressed = self->state;

        if (!self->run_once && self->pressed)
            read = true;
    }

    if (self->run_once)
    {
        if (self->pressed && !self->run_state)
        {
            self->run_state = true;
            read = true;
        }

        if (!self->pressed)
            self->run_state = false;
    }

    return read;
}

static void button_1_Callback(Button_s *self)
{
    static bool state = false;

    if (self->btnRead(self))
    {
        state ^= 1;
        digitalWrite(LED, state);
    }
}

static void button_2_Callback(Button_s *self)
{
    if (self->btnRead(self))
    {
        Serial.println("Oi, eu sou o Goku");
    }
}

/*********************************************************************************************************************
 * Aplication run
 *********************************************************************************************************************/

void setup()
{
    pinMode(LED, OUTPUT);
    Serial.begin(9600);
}

void loop()
{
    Button_s led_btn;
    Button_s serial_btn;

    Button(&led_btn, BTN_1, true, button_1_Callback);
    Button(&serial_btn, BTN_2, true, button_2_Callback);

    while (true)
    {
        led_btn.onClick(&led_btn);
        serial_btn.onClick(&serial_btn);
    }
}