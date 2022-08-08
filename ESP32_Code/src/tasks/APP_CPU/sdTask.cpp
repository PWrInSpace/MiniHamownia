#include "../../../include/tasks/loopTasks.h"

void sdTask(void *arg)
{
  SDCard sd(&myspi, SD_CS);
  String header = "TIME; THRUST; PRESSURE; VALVE_1 STATE; VALVE_2 STATE;\n";
  String data = "";
  uint16_t i = 0;

  vTaskDelay(100 / portTICK_RATE_MS);

  xSemaphoreTake(sm.spiMutex, portMAX_DELAY);

  while (!sd.init())
  {
    digitalWrite(BUILTIN_LED, LOW);
    data = "Sd init error!";
    xQueueSend(sm.btTxQueue, (void *)&data, 10);
    digitalWrite(BUILTIN_LED, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  while (sd.fileExists("/data_test_" + String(i) + ".txt"))
  {
    i++;
  }
  String logPath = "/logs_test_" + String(i) + ".txt";
  String dataPath = "/data_test_" + String(i) + ".txt";

  sd.write(dataPath, header);
  xSemaphoreGive(sm.spiMutex);

  while (1)
  {
    if (xQueueReceive(sm.sdQueue, (void *)&data, portMAX_DELAY) == pdTRUE)
    {
      xSemaphoreTake(sm.spiMutex, portMAX_DELAY);

      if (data.startsWith("LOG: "))
      { // write logs
        if (!sd.write(logPath, data))
        {
          // error handling
        }
      }
      else
      { // write data
        if (!sd.write(dataPath, data))
        {
          // error handling
        }
      }

      xSemaphoreGive(sm.spiMutex);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}