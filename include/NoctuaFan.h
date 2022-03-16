#ifndef NOCTUA_FAN_H_
#define NOCTUA_FAN_H_

#include <functional>

class NoctuaFan{
private:
    const int PWMIndex, PinOut, PinIn;

    void (*mInterrupt)();

    int mRotationsPerMinute;
    unsigned long mLastUpdate;

    int mFanSpeed;
public:
    NoctuaFan(int pwm_index, int pin_out, int pin_in);
    ~NoctuaFan();

    void begin(void (*interrupt)());
    void update();
    void increment();

    int RotationsPerMinute();

    void set_speed(float percent);
    float get_speed();
};

#endif // NOCTUA_FAN_H_