#include "loopTasks.h"
#include "BluetoothSerial.h"
#include "btUI.h"
#include "stateMachine.h"
#include "SDcard.h"
#include "pinout.h"
#include "DCValve.h"

extern BluetoothUI btUI;
extern StateMachine sm;
extern DCValve firstValve;
extern DCValve secondValve;

//***********************************//
//             PRO_CPU               //
//***********************************//

void btReceiveTask(void* arg){
    //pinMode(BUZZER, OUTPUT); //move to pinout.h
    String msg;

    while(true){
        if(btUI.isConnected()){
            if(btUI.available()){
                msg = btUI.readString();
                xQueueSend(sm.btRxQueue, (void*)&msg, 10); //TO DO error handling 
            }    
        }else{
            if(sm.state != DISCONNECTED && sm.state < COUNTDOWN){
                sm.changeState(DISCONNECTED);
                //disconnected sound
                
                while(!btUI.isConnected()){
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                }
                //connected sound

            }else if(sm.state == COUNTDOWN){
                sm.changeState(ABORT);
            }else{
                msg = "LOG: Disconnected in state: " + String(sm.state);
                xQueueSend(sm.sdQueue, (void*)&msg, 0);
            }
        }
        
        //msg.clear();

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void btTransmitTask(void *arg){
    String btMsg;

    while(1){
        xQueueReceive(sm.btTxQueue, (void*)&btMsg, portMAX_DELAY); //wait until data appear in queue
        if(btUI.isConnected()){
          
            btUI.println(btMsg);
        
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
    uint32_t time;
    String btTx;
    TickType_t askTime = xTaskGetTickCount() * portTICK_PERIOD_MS; 
    TickType_t askTimeOut = 30000;

    while(1){
        if(xQueueReceive(sm.btRxQueue, (void*)&btMsg, 1000) == pdTRUE){
            if(sm.state == COUNTDOWN){ //message in countdown state

                sm.changeState(ABORT); 

            //frame check
            }else if(checkCommand(btMsg, prefix, ';', 2) && (sm.state == IDLE)){
                btMsg.remove(0, prefix.length()); //remove MH; prefix
                command = btMsg.substring(0, 4); //get command
                time = btMsg.substring(4).toInt();  //get timer ms

                //valve open timer
                if(command == "MO1;" || command == "MO2;"){
                    if(command[2] == '1'){
                        xTaskCreatePinnedToCore(openFirstValve, "First Val open", 2048, NULL, 2, NULL, APP_CPU_NUM);
                        btTx = "First valve open";
                    }else if(command[2] == '2'){
                        xTaskCreatePinnedToCore(openSecondValve, "second Val open", 2048, NULL, 2, NULL, APP_CPU_NUM);
                        btTx = "Second valve open";
                    }else{
                        btTx = "Unknown valve number";
                    }
                }else if(command == "MC1;" || command == "MC2;"){
                    if(command[2] == '1'){
                        xTaskCreatePinnedToCore(closeFirstValve, "First Val close", 2048, NULL, 2, NULL, APP_CPU_NUM);
                        btTx = "First valve closed";
                    }else if(command[2] == '2'){
                        xTaskCreatePinnedToCore(closeSecondValve, "second Val close", 2048, NULL, 2, NULL, APP_CPU_NUM);
                        btTx = "Second valve closed";
                    }else{
                        btTx = "Unknown valve number";
                    }
                
                }else if(command == "TO1;" || command == "TO2;"){
                    if(command[2] == '1'){
                        //xTaskCreatePinnedToCore(firstValve.timeOpen, "First time open", 2048, (void*)&time.toInt(), 2, NULL, APP_CPU_NUM); //i think it won't work xDD
                        btTx = "First valve open for " + String(time);
                    }else if(command[2] == '2'){
                        //xTaskCreatePinnedToCore(secondValve.timeOpen, "Second Val time open", 2048, (void*)&time.toInt(), 2, NULL, APP_CPU_NUM);
                        btTx = "First valve open for " + String(time);
                    }else{
                        btTx = "Unknown valve number";
                    }

                }else if(command == "VO1;" || command == "VO2;"){
                    if(btUI.setValveOpenTimer(time, command[2] - 48)){
                        btTx = "Valve " + String(command[2] - 48) + " New open time: " + String(btUI.getValveOpenTimer(command[2] - 48));
                    }else{
                        btTx = "Failed to save";    
                    }

                //valve close timer    
                }else if(command == "VC1;" || command == "VC2;"){
                    if(btUI.setValveCloseTimer(time, command[2] - 48)){
                        btTx = "Valve " + String(command[2] - 48) + " New close time: " + String(btUI.getValveCloseTimer(command[2] - 48));
                    }else{
                        btTx = "Failed to save";
                    }

                //valve enable
                }else if(command == "VE1;" || command == "VE2;"){
                    if(btUI.setValveState(time, command[2] - 48)){
                        btTx = "Valve " + String(command[2] - 48) + (btUI.getValveState(command[2] - 48) > 0 ? " enable" : " disable");
                    }else{
                        btTx = "Failed to save";
                    }

                //count down timer
                }else if(command == "CDT;"){
                    if(btUI.setCountDownTime(time)){
                        btTx = "New countdown time:  " + String(btUI.getCountDownTime());
                    }else{
                        btTx = "Failed to save";
                    }

                //calibration
                }else if(command == "CAL;"){
                    sm.changeState(CALIBRATION);
                    vTaskSuspend(NULL); //double suspend check

                    btTx = "Calibration end";
                    
                //save to flash 
                }else if(command == "WCS;"){
                    btUI.saveToFlash();

                    btTx = "Saved to flash";
                
                //show current setings
                }else if(command == "SCS;"){
                    btTx = btUI.timersDescription();

                //enable/disable data frame to user
                }else if(command == "SDF;"){
                    btTx = "Data frame "; //enable / disable
                    //set flag btTxFlag

                //start static fire task
                }else if(command == "SFS;"){
                    //Igniter continuity check
                    //if(digitalRead(CONTINUITY) == HIGH){
                        if(btUI.checkTimers()){//check timers
                            askTime = xTaskGetTickCount() * portTICK_PERIOD_MS; //start timer
                            btTx = btUI.timersDescription(); //show timers
                        
                            btTx += "\n\nDo you want to start test with this settings? Write MH;SFY;"; //ask
                        }else{
                            btTx = "Invalid valve setings";
                        }
                    //}else{
                    //   btTx = "Lack of igniter continuity! :C";
                    // }
                    
                //
                }else if(command == "SFY;"){
                    if((xTaskGetTickCount() * portTICK_PERIOD_MS) - askTime < askTimeOut){
                        btUI.saveToFlash();
                        btTx = "create static fire task";
                        sm.changeState(COUNTDOWN);
                    }else{
                        btTx = "Static fire ask time out!";
                    }
                
                //turn off esp 
                }else if(command == "RST;"){
                    btTx = "Esp is going to sleep";
                
                //error handling
                }else{
                    btTx = "Unknown command";
                    
                }

                xQueueSend(sm.btTxQueue, (void*)&btTx, 10);
            
            }else{ 
                btTx = "Unknown command or not IDLE state";  
                xQueueSend(sm.btTxQueue, (void*)&btTx, 10);
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

//***********************************//
//             APP_CPU               //
//***********************************//

void stateTask(void *arg){
    String stateMsg;
    while(1){
        if(ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE){
            //critical section begin
            portENTER_CRITICAL(&sm.spinlock);
            switch(sm.state){
                case DISCONNECTED:
                    sm.timer.setDefault();
                    vTaskSuspend(sm.uiTask);
                    //vTaskSuspend(sm.dataTask);
                    
                    //security check
                    if(sm.staticFireTask != NULL){
                        vTaskDelete(sm.staticFireTask);
                        sm.staticFireTask = NULL;
                    }

                    break;

                case IDLE:
                    //resume suspended tasks
                    sm.timer.setDefault();
                    vTaskResume(sm.uiTask);
                    vTaskResume(sm.dataTask);

                    if(sm.calibrationTask != NULL){
                        if(eTaskGetState(sm.calibrationTask) != eDeleted){
                            vTaskDelete(sm.calibrationTask);
                        }
                        sm.calibrationTask = NULL;
                    }
                    
                    stateMsg = "State: IDLE"; 
                    break;

                case CALIBRATION:
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                    vTaskSuspend(sm.uiTask); //calibration task will handle ui
                    vTaskSuspend(sm.dataTask); //calibration task need only data from load cell

                    xTaskCreatePinnedToCore(calibrationTask, "calibration task", 4096, NULL, 2, &sm.calibrationTask, APP_CPU_NUM);
                    
                    if(sm.calibrationTask == NULL){
                        stateMsg = "Beep boop error, calibrationTask not created";
                        vTaskResume(sm.uiTask);
                        vTaskResume(sm.dataTask);
                        sm.changeState(IDLE);
                    }else{
                        stateMsg = "State: CALIBRATION";
                    }

                    break;
                
                case COUNTDOWN:
                    xTaskCreatePinnedToCore(staticFireTask, "static fire task", 8192, NULL, 3, &sm.staticFireTask, APP_CPU_NUM);

                    if(sm.staticFireTask == NULL){
                        stateMsg = "Beep boop error, staticFireTask not created";
                        sm.changeState(IDLE);
                    }else{
                        stateMsg = "State: COUNTDOWN";
                    }

                    break;

                case STATIC_FIRE:
                    stateMsg = "State: STATIC_FIRE";
                    break;

                case ABORT:
                    if(sm.staticFireTask != NULL){
                        vTaskDelete(sm.staticFireTask);
                        sm.staticFireTask = NULL;
                    }
                    //close valve or sth
                    digitalWrite(IGNITER, LOW);
                    sm.timer.setDefault();
                    stateMsg = "State: ABORT";
                    break;
            }
    
            //critical section end
            portEXIT_CRITICAL(&sm.spinlock);
            
            //state info for user
            if(sm.state != DISCONNECTED){
                xQueueSend(sm.btTxQueue, (void*)&stateMsg, 0);
            }

        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void dataTask(void *arg){
    String dataFrame;

    while(1){
        //czas testu możesz dostać z sm.timer.getTime();
        dataFrame = "";

        //dataFrame += String(analogRead(A0)); example

        ///..... 

        /*
        if(sm.timer.isEnable()){
            xQueueSend(sm.sdQueue, (void*)&dataFrame, 10);
        }
        */
        /*
        if(btDataFlag){
            xQueueSend(sm.btTxQueue, (void*)&dataFrame, 10);    
        }
        */
       vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void sdTask(void *arg){
    SDCard sd(MOSI, MISO, SCK, SD_CS); //mmiso mosi a
    String data;
    String logPath = "/logs.txt";
    String dataPath = "/data.txt";

    vTaskDelay(100 / portTICK_RATE_MS);
    /*
    while(!sd.init()){
        data = "Sd init error!";
        xQueueSend(sm.btTxQueue, (void*)&data, 10);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    */

    while(1){
        xQueueReceive(sm.sdQueue, (void*)&data, portMAX_DELAY);

        if(data.startsWith("LOG: ")){ //write logs
            if(!sd.write(logPath, data)){
                //error handling
            }
        }else{ //write data
            if(!sd.write(dataPath, data)){
                //error handling
            }
        }
    }
}


void staticFireTask(void *arg){
    uint16_t countDownTime = btUI.getCountDownTime();
    uint32_t firstValveOpenTime = btUI.getValveOpenTimer(FIRST_VALVE);
    uint32_t firstValveCloseTime = btUI.getValveCloseTimer(FIRST_VALVE);
    uint8_t firstValveEnable = btUI.getValveState(FIRST_VALVE);
    uint32_t secondValveOpenTime = btUI.getValveOpenTimer(SECOND_VALVE);
    uint32_t secondValveCloseTime = btUI.getValveCloseTimer(SECOND_VALVE);
    uint8_t secondValveEnable = btUI.getValveState(SECOND_VALVE);
    uint32_t valveOpenTime;
    uint64_t testStartTime;
    uint64_t testStopTime;
    TaskHandle_t firstValveTask = NULL;
    TaskHandle_t secondValveTask = NULL;
    bool igniterState = LOW;
    String msg;

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    //set timers
    testStartTime = millis() + countDownTime; //T0
    testStopTime = (firstValveCloseTime + secondValveCloseTime + 30000) + testStartTime;
    sm.timer.setTimer(testStartTime);


    //countdown
    while((testStartTime - millis()) > 0){
        int timeInSec = (testStartTime - millis()) / 1000;
        
        if((timeInSec > 10) && (timeInSec % 5 == 0)){
        
            msg = String(timeInSec);
            xQueueSend(sm.btTxQueue, (void*)&msg, 0);
        
        }else if(timeInSec <= 10){
            
            msg = String(timeInSec);
            xQueueSend(sm.btTxQueue, (void*)&msg, 0);

        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    if(sm.state == ABORT){
        vTaskDelete(NULL);
    }

    //ignition
    sm.changeState(STATIC_FIRE);
    igniterState = HIGH;
    digitalWrite(IGNITER, igniterState);


    //test start time is T0.00
    //valve control
    while((testStopTime - millis()) > 0){
        //first valve
        if(firstValveEnable && (firstValveTask == NULL)){
            if((millis() - testStartTime) > (firstValveOpenTime)){
                //time open
                if(firstValveCloseTime != 0){
                    valveOpenTime = firstValveCloseTime - firstValveOpenTime;
                    xTaskCreatePinnedToCore(timeOpenFirstValve, "Valve 1", 4096, (void*)&valveOpenTime, 2, &firstValveTask, APP_CPU_NUM);
                    
                    msg = "First valve open for: " + String(valveOpenTime);
                    xQueueSend(sm.btTxQueue, (void*)&msg, 0);
                //open
                }else{
                    msg = "First valve open";
                    xQueueSend(sm.btTxQueue, (void*)&msg, 0);
                    xTaskCreatePinnedToCore(openFirstValve, "Valve 1 open", 4096, NULL, 2, NULL, APP_CPU_NUM);
                }
            }
        }

        //second valve
        if(secondValveEnable && (secondValveTask == NULL)){
            if((millis() - testStartTime) > (secondValveOpenTime)){
                //time open
                if(secondValveCloseTime != 0){
                    valveOpenTime = secondValveCloseTime - secondValveOpenTime;
                    msg = "Second valve open for: " + String(valveOpenTime);
                    xQueueSend(sm.btTxQueue, (void*)&msg, 0);
                    xTaskCreatePinnedToCore(timeOpenSecondValve, "Valve 1", 4096, (void*)&valveOpenTime, 2, &secondValveTask, APP_CPU_NUM);
                //open
                }else{
                    msg = "Second valve open";
                    xQueueSend(sm.btTxQueue, (void*)&msg, 0);
                    xTaskCreatePinnedToCore(openSecondValve, "Valve 1 open", 4096, NULL, 2, NULL, APP_CPU_NUM);
                }
            }
        }

        //igniter
        if(((millis() - testStartTime) > 1000) && (igniterState == HIGH)){
            igniterState = LOW;
            digitalWrite(IGNITER, igniterState);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    sm.timer.setDefault();
    sm.changeState(IDLE);
    sm.staticFireTask = NULL;

    vTaskDelete(NULL);   
}

void calibrationTask(void *arg){
    String msg;
    /*
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    if(eTaskGetState(sm.uiTask) == eSuspended){ 
        msg = "zawieszony";
    }else{
        msg = "dziala";
    }
    
    xQueueSend(sm.btTxQueue, (void*)&msg, 0);

    msg = "Start calibration";
    xQueueSend(sm.btTxQueue, (void*)&msg, 0);
    
    xQueueReceive(sm.btRxQueue, (void*)&msg, portMAX_DELAY);
    xQueueSend(sm.btTxQueue, (void*)&msg, 0);
    */

    //exit
    sm.changeState(IDLE);
    sm.calibrationTask = NULL;
    vTaskDelete(NULL);
}