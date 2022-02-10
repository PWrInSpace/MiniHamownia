#include "loopTasks.h"
#include "BluetoothSerial.h"
#include "btUI.h"
#include "stateMachine.h"

extern BluetoothUI btui;
StateMachine state = DISCONNECTED;

//main security task xD
//pamiętać że gdy jest już test, nie wrzucać programu do statu disconnected jeżeli dosżło by do utraty łaczności
void bluetoothTask(void* arg){
    //pinMode(BUZZER, OUTPUT); //move to pinout.h

    while(true){
        if(btui.isConnected()){
            if(btui.available()){

            }    
        
        }else{
            if(state != DISCONNECTED && state < STATIC_FIRE){
                
                //disconnected sound
                while(!btui.isConnected()){
                    delay(250);
                }

                //connected sound
            }else if(state == COUNTDOWN){
                state = ABORT;
            }else{
                //sd_log, dissconnected in state STATE at: TIME
            }
        }
        
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}