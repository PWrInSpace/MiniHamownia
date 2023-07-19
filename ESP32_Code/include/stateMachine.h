#ifndef STATE_MACHINE_HH
#define STATE_MACHINE_HH

// #include "freertos/FreeRTOS.h"
#include "timer.h"

#define Rx_QUEUE_LENGTH 5
#define Tx_QUEUE_LENGTH 10
#define SD_QUEUE_LENGTH 15

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
    Timer timer;

    TaskHandle_t btRxTask = NULL;
    TaskHandle_t btTxTask = NULL;
    TaskHandle_t uiTask = NULL;
    
    TaskHandle_t stateTask = NULL;
    TaskHandle_t sdTask = NULL;
    TaskHandle_t dataTask = NULL;
    TaskHandle_t calibrationTask = NULL;
    TaskHandle_t staticFireTask = NULL;
    
    QueueHandle_t btRxQueue = NULL;
    QueueHandle_t btTxQueue = NULL;
    QueueHandle_t sdQueue = NULL;

    SemaphoreHandle_t spiMutex = NULL;
    portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
    
    void changeState(State _state){
        //portENTER_CRITICAL(&spinlock);
        state = _state;
        //portEXIT_CRITICAL(&spinlock);
        xTaskNotifyGive(stateTask);
    }
    
} StateMachine;

#endif