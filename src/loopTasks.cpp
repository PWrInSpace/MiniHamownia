#include "loopTasks.h"
#include "BluetoothSerial.h"
#include "btUI.h"
#include "stateMachine.h"
#include "SDcard.h"

extern BluetoothUI btUI;
extern StateMachine stateMachine;

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
                xTaskNotifyGive(stateMachine.stateTask);
            }else{
                msg = "LOG: Disconnected in state: " + String(stateMachine.state);
                xQueueSend(stateMachine.sdQueue, (void*)&msg, 0);
            }
        }
        
        msg.clear();

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

void uiTask(void *arg){
    String btMsg;
    String prefix = "MH;";  //prefix
    String command;
    String time;
    String btTx;
    TickType_t askTime = xTaskGetTickCount() * portTICK_PERIOD_MS; 
    TickType_t askTimeOut = 15000;

    while(1){
        if(xQueueReceive(stateMachine.btRxQueue, (void*)&btMsg, 10) == pdTRUE){
            if(stateMachine.state == COUNTDOWN){ //message in countdown state

                stateMachine.state = ABORT;
                xTaskNotifyGive(stateMachine.stateTask); 

            //frame check
            }else if(checkCommand(btMsg, prefix, ';', 2) && (stateMachine.state == IDLE)){
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
                        btTx = "Valve " + String(command[2] - 48) + (btUI.getValveState(command[2] - 48) > 0 ? " enable" : " disable");
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
                    stateMachine.state = CALIBRATION;
                    xTaskNotifyGive(stateMachine.stateTask);

                    btTx = "Calibration begin";
                    
                //save to flash 
                }else if(command == "WCS;"){
                    btUI.saveToFlash();

                    btTx = "Saved to flash";
                
                //show current setings
                }else if(command == "SCS;"){
                    btTx = "SCS";

                //enable/disable data frame to user
                }else if(command == "SDF;"){
                    btTx = "Data frame "; //enable / disable
                    //set flag

                //start static fire task
                }else if(command == "SFS;"){
                    askTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    //check timers
                    //show timers
                    //ask
                    btTx = "Do you want to start test with this settings? Write MH;SFY;";

                //
                }else if(command == "SFY;"){
                    if((xTaskGetTickCount() * portTICK_PERIOD_MS) - askTime < askTimeOut){
                        btTx = "create static fire task";
                        stateMachine.state = COUNTDOWN;
                        xTaskNotifyGive(stateMachine.stateTask);
                    }else{
                        btTx = "Static fire ask time out!";
                    }
                
                //turn off esp 
                }else if(command == "OFF;"){
                    btTx = "Esp is going to sleep";
                    
                //error handling
                }else{
                    btTx = "Unknown command";
                    
                }

                xQueueSend(stateMachine.btTxQueue, (void*)&btTx, 10);
            
            }else{ 
                btTx = "Unknown command or not IDLE state";  
                xQueueSend(stateMachine.btTxQueue, (void*)&btTx, 10);
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

//sama sekwencja testowa na przerwaniu?
//abort to przerwanie

//***********************************//
//             APP_CPU               //
//***********************************//

void stateTask(void *arg){
    String stateMsg;
    while(1){
        if(ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE){
            //critical section begin
            portENTER_CRITICAL(&stateMachine.spinlock);
            
            if(stateMachine.state == DISCONNECTED){
                //suspend tasks
                vTaskSuspend(stateMachine.uiTask);
                vTaskSuspend(stateMachine.dataTask);
                
                //security check
                if(stateMachine.staticFireTask != NULL){
                    vTaskDelete(stateMachine.staticFireTask);
                    stateMachine.staticFireTask = NULL;
                }

            }else if(stateMachine.state == IDLE){
                vTaskResume(stateMachine.uiTask);
                vTaskResume(stateMachine.dataTask);
                
                stateMsg = "State: IDLE"; 

            }else if(stateMachine.state == CALIBRATION){
                //xTaskCreatePinnedToCore(calibrationTask, "calibration task", 16384, )
                
                stateMsg = "State: CALIBRATION";
            }else if(stateMachine.state == COUNTDOWN){
                xTaskCreatePinnedToCore(staticFireTask, "static fire task", 16384, NULL, 3, &stateMachine.staticFireTask, APP_CPU_NUM);
                //check that this task will work only one time

                stateMsg = "State: COUNTDOWN";

            }else if(stateMachine.state == STATIC_FIRE){
                stateMsg = "State: STATIC_FIRE";
                
            }else if(stateMachine.state == ABORT){
                if(stateMachine.staticFireTask != NULL){
                    vTaskDelete(stateMachine.staticFireTask);
                    stateMachine.staticFireTask = NULL;
                }
                stateMsg = "State: ABORT";
            }

            if(stateMachine.state != DISCONNECTED){
                xQueueSend(stateMachine.btTxQueue, (void*)&stateMsg, 0);
            }

            //critical section end
            portEXIT_CRITICAL(&stateMachine.spinlock);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void dataTask(void *arg){
    String dataFrame;

    while(1){
        dataFrame = "";

        //dataFrame += String(analogRead(A0)); example

        ///..... 


        if(stateMachine.state == COUNTDOWN || stateMachine.state == STATIC_FIRE){
            xQueueSend(stateMachine.sdQueue, (void*)&dataFrame, 10);
        }
        /*
        if(btDataFlag){
            xQueueSend(stateMachine.btTxQueue, (void*)&dataFrame, 10);    
        }
        */
       vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void sdTask(void *arg){
    SDCard sd(0, 0, 0, 0); //mmiso mosi a
    String data;
    String logPath = "/logs.txt";
    String dataPath = "/data.txt";

    vTaskDelay(100 / portTICK_RATE_MS);
    /*
    if(!sd.init()){
        data = "Sd init error!";
        while(1){
            xQueueSend(stateMachine.btTxQueue, (void*)&data, 10);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
    */

    while(1){
        xQueueReceive(stateMachine.sdQueue, (void*)&data, portMAX_DELAY);

        if(data.startsWith("LOG: ")){
            if(!sd.write(logPath, data)){
                //error handling
            }
        }else{
            if(!sd.write(dataPath, data)){
                //error handling
            }
        }
    }
}


void staticFireTask(void *arg){
    TickType_t countDownTime = btUI.getCountDownTime();
    uint32_t firstValveOpenTime = btUI.getValveOpenTimer(FIRST_VALVE);
    uint32_t firstValveCloseTime = btUI.getValveCloseTimer(FIRST_VALVE);
    uint8_t firstValveEnable = btUI.getValveState(FIRST_VALVE);
    uint32_t secondValveOpenTime = btUI.getValveOpenTimer(SECOND_VALVE);
    uint32_t secondValveCloseTime = btUI.getValveCloseTimer(SECOND_VALVE);
    uint8_t secondValveEnable = btUI.getValveState(SECOND_VALVE);
    TickType_t startTime;
    TickType_t stopTime;
    String msg;

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    startTime = xTaskGetTickCount();
    stopTime = (countDownTime + secondValveCloseTime) * 2 + startTime;
    countDownTime += startTime;


    while((countDownTime - (xTaskGetTickCount() * portTICK_PERIOD_MS)) > 0){
        int timeInSec = (countDownTime - (xTaskGetTickCount() * portTICK_PERIOD_MS)) / 1000;
        
        if((timeInSec > 10) && ((timeInSec % 5) == 0)){ //print every 5sec
            msg = String(timeInSec);
            xQueueSend(stateMachine.btTxQueue, (void*)&msg, 0);
        }else if(timeInSec <= 10){ //10, 9, 8 ...
            msg = String(timeInSec);
            xQueueSend(stateMachine.btTxQueue, (void*)&msg, 0);    
        }
        /* not neccesery
        if(stateMachine.state == ABORT){
            vTaskDelete(NULL);
        }
        */
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    msg = "static fire fruuuu";
    xQueueSend(stateMachine.btTxQueue, (void*)&msg, 0);

    while(stopTime > xTaskGetTickCount() * portTICK_PERIOD_MS){
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    msg = "usuwanie taska";
    xQueueSend(stateMachine.btTxQueue, (void*)&msg, 0);


    stateMachine.state = IDLE;
    xTaskNotifyGive(stateMachine.stateTask);


    vTaskDelete(NULL);   
}