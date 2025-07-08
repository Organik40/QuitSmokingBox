#ifndef TIMER_H
#define TIMER_H

class Timer {
public:
    Timer();
    void start(unsigned long duration);
    void stop();
    bool isRunning();
    unsigned long getTimeRemaining();
    void update();

private:
    unsigned long startTime;
    unsigned long duration;
    bool running;
};

#endif // TIMER_H