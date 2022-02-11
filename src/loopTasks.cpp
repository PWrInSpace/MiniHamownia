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
    String msg;

    while(true){
        if(btUI.isConnected()){
            if(btUI.available()){
                msg = btUI.readString();
                xQueueSend(stateMachine.btRxQueue, (void*)&msg, 10); //TO DO error handling 
            }    
        }else{
            if(stateMachine.state != DISCONNECTED && stateMachine.state < COUNTDOWN){
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
                //String log = "Disconnected in state: " + String(stateMachine.state);
                //sd_log, dissconnected in state STATE at: TIME
            }
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void btTransmitTask(void *arg){
    String btMsg;

    while(1){
        xQueueReceive(stateMachine.btTxQueue, (void*)&btMsg, portMAX_DELAY); //wait until data appear in queue
        if(btUI.isConnected()){
            if(btMsg == "SCS"){
                btUI.printTimers();
            }else{
                btUI.println(btMsg);
            }
        }else{
            //error
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void stateTask(void *arg){
    State previousState = DISCONNECTED;
    
    while(1){
        if(ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE){
            //critical section begin
            //portENTER_CRITICAL(&spinlock);
            if(stateMachine.state == DISCONNECTED){
                //uiTask suspend
                //dataTask suspend
                //sdTask suspend ??
            }else if(stateMachine.state == IDLE){
                //
            }else if(stateMachine.state == CALIBRATION){
                
            }else if(stateMachine.state == COUNTDOWN){

            }else if(stateMachine.state == STATIC_FIRE){

            }else if(stateMachine.state == ABORT){
                //staticFireTask delete 
                //
            }
            //portEXIT_CRITICAL(&spinlock);
            //critical section end

            previousState = stateMachine.state;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void uiTask(void *arg){
    String btMsg;
    String prefix = "MH;";  //prefix
    String command;
    String time;
    String btTx; 

    while(1){
        if(xQueueReceive(stateMachine.btRxQueue, (void*)&btMsg, 10) == pdTRUE){
            if(stateMachine.state == COUNTDOWN){ //message in countdown state

                stateMachine.state = ABORT;
                xTaskNotifyGive(stateMachine.stateTask); 

            //frame check
            }else if(checkCommand(btMsg, prefix, ';', 2)){
                btMsg.remove(0, prefix.length()); //remove MH; prefix
                command = btMsg.substring(0, 4); //get command
                time = btMsg.substring(4);  //get timer

                //valve open timer
                if(command == "VO1;" || command == "VO2;"){
                    if(btUI.setValveOpenTimer(time.toInt(), command[2] - 48)){
                        btTx = "Valve " + String(command[2] - 48) + " New open time: " + String(btUI.getValveOpenTimer(command[2] - 48));
                    }else{
                        btTx = "Failed to save";    
                    }

                //valve close timer    
                }else if(command == "VC1;" || command == "VC2;"){
                    if(btUI.setValveCloseTimer(time.toInt(), command[2] - 48)){
                        btTx = "Valve " + String(command[2] - 48) + " New close time: " + String(btUI.getValveCloseTimer(command[2] - 48));
                    }else{
                        btTx = "Failed to save";
                    }

                //valve enable
                }else if(command == "VE1;" || command == "VE2;"){
                    if(btUI.setValveState(time.toInt(), command[2] - 48)){
                        btTx = "Valve " + String(command[2] - 48) + " enable";
                    }else{
                        btTx = "Failed to save";
                    }

                //count down timer
                }else if(command == "CDT;"){
                    if(btUI.setCountDownTime((uint16_t)time.toInt())){
                        btTx = "New count down time:  " + String(btUI.getCountDownTime());
                    }else{
                        btTx = "Failed to save";
                    }

                //calibration
                }else if(command == "CAL;"){
                    btTx = "Calibration begin";
                    //start calibration

                //show current setings 
                }else if(command == "WCS;"){
                    btUI.saveToFlash();
                    btTx = "Saved to flash";
                }else if(command == "SCS;"){
                    btTx = "SCS";

                //enable/disable data frame to user
                }else if(command == "SDF;"){
                    btTx = "Data frame "; //enable / disable
                    //set flag

                //start static fire task
                }else if(command == "SFS;"){
                    btTx = "Static fire task created";
                    //check timers 
                    //change state to countdown

                //error handling
                }else{
                    btTx = "Unknow command";
                    
                }

            }else{ 
                btTx = "Unknow command";  
            }

            xQueueSend(stateMachine.btTxQueue, (void*)&btTx, 10);

        }else{
            //calibration??
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

//sama sekwencja testowa na przerwaniu?
//abort to przerwanie