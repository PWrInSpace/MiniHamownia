#include "loopTasks.h"
#include "BluetoothSerial.h"
#include "btUI.h"
#include "stateMachine.h"

extern BluetoothUI btUI;
extern StateMachine stateMachine;

//main security task xD
//pamiętać że gdy jest już test, nie wrzucać programu do statu disconnected jeżeli dosżło by do utraty łaczności
void btReceiveTask(void* arg){
    //pinMode(BUZZER, OUTPUT); //move to pinout.h
    String message;

    while(true){
        if(btUI.isConnected()){
            if(btUI.available()){
                message = btUI.readString();
                xQueueSend(stateMachine.btRxQueue, (void*)&message, 10); //TO DO error handling 
            }    
        }else{
            if(stateMachine.state != DISCONNECTED && stateMachine.state < STATIC_FIRE){
                //disconnected sound
                while(!btUI.isConnected()){
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                }
                //connected sound
            }else if(stateMachine.state == COUNTDOWN){
                //spinlock?
                stateMachine.state = ABORT;
                //spinlock?
                btUI.println("ABORT!");
            }else{
                //sd_log, dissconnected in state STATE at: TIME
            }
        }
        
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void btTransmitTask(void *arg){
    String btMsg;

    while(1){
        xQueueReceive(stateMachine.btTxQueue, (void*)&btMsg, portMAX_DELAY); //wait until data appear in queue
        if(btUI.isConnected()){
            btUI.println(btMsg);
        }else{
            //error
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void mainTask(void *arg){
    String btMsg;
    String command = "MH;";
    String uC = "Unknown command";
    
    while(1){
        if(xQueueReceive(stateMachine.btRxQueue, (void*)&btMsg, 10) == pdTRUE){
            if(btMsg.startsWith(command)){
                btMsg.remove(0, command.length()); //remove MH; prefix
                xQueueSend(stateMachine.btTxQueue, (void*)&btMsg, 10);
            }else{             
                xQueueSend(stateMachine.btTxQueue, (void*)&uC, 10);
            }
        }
    }
}