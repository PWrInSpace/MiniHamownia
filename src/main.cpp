#include "Arduino.h"
#include "btUI.h"
#include "BluetoothSerial.h"
#include "stateMachine.h"
#include "loopTasks.h"
#include "DCValve.h"
#include "pinout.h"

BluetoothUI btUI;
StateMachine sm;
extern DCValve firstValve;
extern DCValve secondValve;

void setup()
{
    Serial.begin(115200);  //debug only
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //pinMode(2, OUTPUT);
    
    btUI.begin();
    while(!btUI.isConnected()){
        //Serial.println("Nie ma połączenia");
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
    
    Serial.println("Połączono");

    firstValve.init();
    secondValve.init();

    sm.btRxQueue = xQueueCreate(Rx_QUEUE_LENGTH, sizeof(String));
    sm.btTxQueue = xQueueCreate(Tx_QUEUE_LENGTH, sizeof(String));
    sm.sdQueue   = xQueueCreate(SD_QUEUE_LENGTH, sizeof(String));
    
    xTaskCreatePinnedToCore(btReceiveTask,  "Bt rx task", 4096, NULL, 2, &sm.btRxTask, PRO_CPU_NUM);
    xTaskCreatePinnedToCore(btTransmitTask, "Bt tx task", 4096, NULL, 1, &sm.btTxTask, PRO_CPU_NUM);
    xTaskCreatePinnedToCore(uiTask,         "Ui task",    4096, NULL, 2, &sm.uiTask,   PRO_CPU_NUM);

    xTaskCreatePinnedToCore(stateTask,      "State task", 4096,  NULL, 3, &sm.stateTask, APP_CPU_NUM);
    xTaskCreatePinnedToCore(dataTask,       "data task",  8192, NULL, 2, &sm.dataTask,  APP_CPU_NUM);
    xTaskCreatePinnedToCore(sdTask,         "SD task",    4096, NULL, 1, &sm.sdTask,    APP_CPU_NUM);

    //TO DO: check tasks

    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    if(sm.btRxTask == NULL || sm.btTxTask == NULL || sm.uiTask == NULL){
        btUI.println("Pro_cpu task error");
        ESP.restart();
    }

    if(sm.stateTask == NULL || sm.dataTask == NULL || sm.sdTask == NULL){
        btUI.println("Pro_cpu task error");
        ESP.restart();
    }

    if(sm.btRxQueue == NULL || sm.btTxQueue == NULL || sm.sdQueue == NULL){
        btUI.println("Queue create error");
        ESP.restart();
    }

    //welcome screen
    btUI.println("MiniHamownia v1.0 aka Rozkurwiacz");
    btUI.println(btUI.timersDescription());
    sm.changeState(IDLE);

    vTaskDelete(NULL);
}


void loop(){
    
}
