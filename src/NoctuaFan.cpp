#include "NoctuaFan.h"

#include <Arduino.h>

#define TARGET_PWM_FREQUENCY 25000
#define PULSE_PER_ROTATION 2
#define UPDATE_CYCLE 1000

static volatile int rpm_counter;

NoctuaFan::NoctuaFan(int pwm_index, int pin_out, int pin_in) : PWMIndex(pwm_index), PinOut(pin_out), PinIn(pin_in), mFanSpeed(255) {

}

NoctuaFan::~NoctuaFan(){

}

void NoctuaFan::begin(void (*interrupt)()){
    mInterrupt = interrupt;

    ledcAttachPin(PinOut, PWMIndex);
    ledcSetup(PWMIndex, TARGET_PWM_FREQUENCY, 8);
    ledcWrite(PWMIndex, mFanSpeed);

    pinMode(PinIn, INPUT);
    digitalWrite(PinIn, HIGH);
    attachInterrupt(digitalPinToInterrupt(PinIn), mInterrupt, FALLING);
}

void NoctuaFan::set_speed(float percent){
    if( percent > 1.0 ) mFanSpeed = 255;
    if( percent < 0.0 ) mFanSpeed = 0;
    mFanSpeed = (uint32_t)(255.0f * percent);

    ledcWrite(PWMIndex, mFanSpeed);
}

float NoctuaFan::get_speed() {
    return mFanSpeed / 255.0f;
}

void NoctuaFan::update(){
    if ((unsigned long)(millis() - mLastUpdate) >= UPDATE_CYCLE) {
        // detach interrupt while calculating rpm
        detachInterrupt(digitalPinToInterrupt(PinIn));

        // calculate rpm
        mRotationsPerMinute = rpm_counter * (60.0f / (float)PULSE_PER_ROTATION) * (1000.0f / (float)UPDATE_CYCLE);

        // reset counter
        rpm_counter = 0;

        // store milliseconds when tacho was measured the last time
        mLastUpdate = millis();

        // attach interrupt again
        attachInterrupt(digitalPinToInterrupt(PinIn), mInterrupt, FALLING);
    }
}

void NoctuaFan::increment(){
    rpm_counter++;
}

int NoctuaFan::RotationsPerMinute() {
    return mRotationsPerMinute;
}