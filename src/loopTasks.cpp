#include "loopTasks.h"
#include "BluetoothSerial.h"

//main security task xD

//pamiętać że gdy jest już test, nie wrzucać programu do statu disconnected jeżeli dosżło by do utraty łaczności
void bluetoothTask(void* arg){
    BluetoothSerial BTSerial;
    
    BTSerial.begin("rozkurwiacz");
    pinMode(BUZZER, OUTPUT); //move to pinout.h
    
    while(true){
        if(BTSerial.hasClient() < 1){
            /*
            if(state != disconnect && state < staticFire){
                //state = disconnected
                disconnect sound
                for(int i=0; i<2; i-=-1){
                    digitalWrite(BUZZER, HIGH);
                    delay(100*(i+1));
                    digitalWrite(BUZZER, LOW);
                    delay(100*(i+1));
                    digitalWrite(BUZZER, HIGH);
                    delay(200*(i+1));
                    digitalWrite(BUZZER, LOW);
                    delay(200*(i+1));
                }
                
                while(BTSerial.hasClient() < 1){
                    delay(250);
                }

                //connected sound
                digitalWrite(BUZZER, HIGH);
                delay(100);
                digitalWrite(BUZZER, LOW);
                delay(100);
                digitalWrite(BUZZER, HIGH);
                delay(100);
                digitalWrite(BUZZER, LOW);

            }else if(state == COUNTDOWN){
                ABORT, stop test 
            }else{
                //sd_log, dissconnected in state STATE at: TIME
            }
            */
        }else{
            //bt handler
        }
    
        vTaskDelay(100/portTICK_PERIOD_MS)
    }
}