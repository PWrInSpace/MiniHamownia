#ifndef LOOP_TASKS_HH
#define LOOP_TASKS_HH

void btReceiveTask(void *arg);
void btTransmitTask(void *arg);
void uiTask(void *arg);

void stateTask(void *arg);
void dataTask(void *arg);
void sdTask(void *arg);
void staticFireTask(void *arg);
void calibrationTask(void *arg);

#endif