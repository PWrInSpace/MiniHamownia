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
    btUI.printTimers();

    stateMachine.btRxQueue = xQueueCreate(Rx_QUEUE_LENGTH, sizeof(String));
    stateMachine.btTxQueue = xQueueCreate(Tx_QUEUE_LENGTH, sizeof(String));
    
    xTaskCreatePinnedToCore(btReceiveTask, "Bt rx task", 16384, NULL, 2, &stateMachine.btRxTask, PRO_CPU_NUM);
    xTaskCreatePinnedToCore(btTransmitTask, "Bt tx task", 16384, NULL, 1, &stateMachine.btTxTask, PRO_CPU_NUM);
    xTaskCreatePinnedToCore(uiTask, "User interface task", 16384, NULL, 2, &stateMachine.uiTask, PRO_CPU_NUM);
    
    vTaskDelete(NULL);
}


void loop(){
    
}
