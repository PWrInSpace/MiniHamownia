#ifndef STATE_MACHINE_HH
#define STATE_MACHINE_HH

#include "FreeRTOS.h"

#define Rx_QUEUE_LENGTH 5
#define Tx_QUEUE_LENGTH 10
#define SD_QUEUE_LENGTH 10

enum State{
    DISCONNECTED = 0,
    IDLE,
    CALIBRATION,
    COUNTDOWN,
    STATIC_FIRE,
    ABORT
};

typedef struct StateMachine{
    State state = DISCONNECTED;

    TaskHandle_t btRxTask = NULL;
    TaskHandle_t btTxTask = NULL;
    TaskHandle_t uiTask = NULL;
    
    TaskHandle_t stateTask = NULL;
    TaskHandle_t sdTask = NULL;
    TaskHandle_t dataTask = NULL;
    TaskHandle_t staticFireTask = NULL;
    
    QueueHandle_t btRxQueue = NULL;
    QueueHandle_t btTxQueue = NULL;
    QueueHandle_t sdQueue = NULL;

    portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

    //state mutex?
} StateMachine;



#endif