#include "Arduino.h"
#include "btUI.h"
#include "BluetoothSerial.h"
#include "stateMachine.h"
#include "loopTasks.h"

BluetoothUI btUI;
StateMachine stateMachine;

void setup()
{
    Serial.begin(115200);  //debug only
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //pinMode(2, OUTPUT);
    
    btUI.begin();
    while(!btUI.isConnected()){
        vTaskDelay(250 / portTICK_PERIOD_MS); //wait until connect
    }
    stateMachine.state = IDLE;
    btUI.println("MiniHamownia v1.0 aka Rozkurwiacz");
    btUI.printTimers();

    stateMachine.btRxQueue = xQueueCreate(Rx_QUEUE_LENGTH, sizeof(String));
    stateMachine.btTxQueue = xQueueCreate(Tx_QUEUE_LENGTH, sizeof(String));
    stateMachine.sdQueue   = xQueueCreate(SD_QUEUE_LENGTH, sizeof(String));
    
    xTaskCreatePinnedToCore(btReceiveTask,  "Bt rx task", 16384, NULL, 2, &stateMachine.btRxTask, PRO_CPU_NUM);
    xTaskCreatePinnedToCore(btTransmitTask, "Bt tx task", 16384, NULL, 1, &stateMachine.btTxTask, PRO_CPU_NUM);
    xTaskCreatePinnedToCore(uiTask,         "Ui task",    16384, NULL, 2, &stateMachine.uiTask,   PRO_CPU_NUM);

    xTaskCreatePinnedToCore(stateTask,      "State task", 4096,  NULL, 3, &stateMachine.stateTask, APP_CPU_NUM);
    xTaskCreatePinnedToCore(dataTask,       "data task",  16384, NULL, 2, &stateMachine.dataTask,  APP_CPU_NUM);
    xTaskCreatePinnedToCore(sdTask,         "SD task",    16384, NULL, 1, &stateMachine.sdTask,    APP_CPU_NUM);

    xTaskNotifyGive(stateMachine.stateTask); //first run, set state

    vTaskDelete(NULL);
}


void loop(){
    
}
