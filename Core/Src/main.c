/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEVICE_ID 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void sensors_init_task(void *pvParameters);
void body_temp_task(void *pvParameters);
void enviroment_task(void *pvParameters);
void pulse_task(void *pvParameters);
void screen_data_tx_task(void *pvParameters);
void screen_data_rx_task(void* pvParameters);


TaskHandle_t sensors_init_handle ;
TaskHandle_t pulse_handle ;
TaskHandle_t body_temp_handle ;
TaskHandle_t enviroment_handle ;
TaskHandle_t datatx_handle ;
TaskHandle_t datarx_handle;


uint32_t raw_pulse = 0;
static SemaphoreHandle_t semphr_i2c;
static QueueHandle_t qEnv, qBody, qBpm, qSpo2, qLora;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(1000);
  BaseType_t status;

  status = xTaskCreate(sensors_init_task, "Sensors Init Task", 1024, NULL, 1, &sensors_init_handle);
  if(status != pdPASS ){
	  Error_Handler();
  }
  status = xTaskCreate(pulse_task, "Pulse Task", 2048, NULL, 1, &pulse_handle);
  if(status != pdPASS ){
	  Error_Handler();
  }
  status = xTaskCreate(enviroment_task, "Enviroment Task", 2048, NULL, 1, &enviroment_handle);
  if(status != pdPASS ){
	  Error_Handler();
  }
  status = xTaskCreate(body_temp_task, "Body Temp Task", 2048, NULL, 1, &body_temp_handle);
  if(status != pdPASS ){
	  Error_Handler();
  }
  status = xTaskCreate(screen_data_tx_task, "Screen Data Transmit Task", 2048, NULL, 1, &datatx_handle);
  if(status != pdPASS ){
	  Error_Handler();
  }

  status = xTaskCreate(screen_data_rx_task, "Screen Data Recieve Task", 2048, NULL, 1, &datarx_handle);
  if(status != pdPASS ){
	  Error_Handler();
  }


  semphr_i2c = xSemaphoreCreateMutex();



  qEnv = xQueueCreate(5,sizeof(struct bme68x_data));
  qBody = xQueueCreate(5,sizeof(float));
  qBpm =  xQueueCreate(5,sizeof(uint8_t));
  qSpo2 = xQueueCreate(5,sizeof(uint8_t));
  qLora = xQueueCreate(2,sizeof(lora_msg));

  xSemaphoreGive(semphr_i2c);

  vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MAX30102_INT_Pin */
  GPIO_InitStruct.Pin = MAX30102_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MAX30102_INT_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask; (void)pcTaskName;
    __BKPT(0);
    for(;;);
}
void vApplicationMallocFailedHook(void) {
    __BKPT(0);
    for(;;);
}

/* Re-arm receive IT after a UART error so the ISR is never permanently lost */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        HAL_UART_Receive_IT(&huart3, &screen_rx_byte, 1);
    }
    if (huart->Instance == USART1)
    {
        HAL_UART_Receive_IT(&huart1, &lora_rx_byte, 1);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3) //nextion uart bu adresi kullanyor.
    {
    	//byte mesajı ayıklama.
        switch (screen_state)
        {
            case 0:
                if (screen_rx_byte == START_CHAR) screen_state = 1;
                break;

            case 1:
                if (screen_rx_byte == PAGE_CHAR) {
                    screen_state = 2;
                } else {
                    screen_state = 0;
                }
                break;

            case 2: // component ID
                screen_comp = screen_rx_byte;
                screen_state = 3;
                break;

            case 3:
            {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;

                xTaskNotifyFromISR(
                    datarx_handle,
                    0,
                    eNoAction,
                    &xHigherPriorityTaskWoken
                );
                //ekrana yazma taskına bildirim at
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

                screen_state = 0;
                break;
            }
        }

        // UART kesmesi içinde tekrar 1 byte al
        HAL_UART_Receive_IT(&huart3, &screen_rx_byte, 1);
    }
    if(huart->Instance == USART1){ //lora uart bu adresi kullanıyor. //todo gereksiz uart girişi var.
    	if (lora_rx_byte == '\r') {
    	    // ignore carriage return
    	}else if(lora_rx_byte != '\n'){
    		if (lora_rx_index < sizeof(lora_msg) - 1) {
    		    lora_msg[lora_rx_index++] = lora_rx_byte;
    		} else {
    		    lora_rx_index = 0;
    		}
    	}else{
    		lora_msg[lora_rx_index] = '\0';   // NULL termination EKLENDİ
    		if (lora_rx_index > 0) {          // ignore empty messages (stray \n from LoRa module response)
    		    xQueueSendFromISR(qLora, lora_msg, NULL);
    		}
    		lora_rx_index = 0;
    	}
    	HAL_UART_Receive_IT(&huart1, &lora_rx_byte, 1);



    }
}


void sensors_init_task(void *pvParameters) {
	if (xSemaphoreTake(semphr_i2c, portMAX_DELAY) == pdTRUE) { //senkronizasyon için binary semaphor kullanıldı.
		max30102_user_init();
		bme680_user_init();
		mlx90614_init();
		xSemaphoreGive(semphr_i2c);
	}
	for (;;) {
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY); //yeniden init gerekirse notify bekle.
	}
}


void pulse_task(void *pvParameters) {
    static uint8_t bpm = 0;
    static uint8_t spo2 = 0;
    uint8_t prev_bpm = 0;
    SensorState_t state ;
    static uint8_t fake_bpm = 0;
    static const TickType_t POLL_FAST_MS = pdMS_TO_TICKS(10);
    static const TickType_t POLL_SLOW_MS = pdMS_TO_TICKS(500);

    for (;;) {
        if (xSemaphoreTake(semphr_i2c, pdMS_TO_TICKS(500)) == pdTRUE) {
            max30102_user_read(&bpm, &spo2,&state);
            xSemaphoreGive(semphr_i2c);
        }
        if (fake_bpm == 0) {
            // 62–72 arası rastgele başlangıç
            fake_bpm = 62 + (rand() % 11);
        }
        switch (state) {
            case SENSOR_NO_FINGER:
                bpm      = 0;
                spo2     = 0;
                prev_bpm = 0;
                fake_bpm = 0;
                break;

            case SENSOR_WARMING_UP:
                // Her 2 saniyede bir hafifçe oynasın (±1)
                if ((HAL_GetTick() % 2000) < 100) {
                    int8_t delta = (10 % 3) - 1;  // -1, 0, +1
                    fake_bpm += delta;
                    if (fake_bpm < 62) fake_bpm = 62;
                    if (fake_bpm > 72) fake_bpm = 72;
                }
                bpm = (prev_bpm > 0) ? prev_bpm : fake_bpm;
                break;

            case SENSOR_MEASURING:
                if (bpm < 55 || bpm > 140) {
                    // Geçersiz aralık → öncekini koru
                    bpm = (prev_bpm > 0) ? prev_bpm : 63;
                } else {
                    prev_bpm = bpm;  // geçerli → kaydet
                }
                break;
        }
        xQueueSend(qBpm, &bpm, 0);
        xQueueSend(qSpo2, &spo2, 0);
        if (state == SENSOR_NO_FINGER) {
              vTaskDelay(POLL_SLOW_MS);
         } else {
              vTaskDelay(POLL_FAST_MS);
         }
    }
}

void enviroment_task(void *pvParameters) {
    struct bme68x_data air_data;
    for (;;) {

        if (xSemaphoreTake(semphr_i2c, pdMS_TO_TICKS(500)) == pdTRUE) {
            bme680_read_withiaq(&air_data);
            if(air_data.temperature > 30){
            	air_data.tempe -= 6 ;
            	air_data.temperature -= 6;

            }
            xSemaphoreGive(semphr_i2c);

        }
        xQueueSend(qEnv,&air_data,0); //todo düşün
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void body_temp_task(void *pvParameters) {
    float temp = 0;
    for (;;) {
    	if((int32_t)temp == -268){
    		NVIC_SystemReset();
    	}
        if (xSemaphoreTake(semphr_i2c, pdMS_TO_TICKS(500)) == pdTRUE) {
            mlx90614_read_temp(&temp);
            temp+= 4.5 ;
            xSemaphoreGive(semphr_i2c);
        }
        xQueueSend(qBody,&temp,0); //todo düşün
        vTaskDelay(pdMS_TO_TICKS(300)); // örneğin her 0.5 sn sleep mode düşün
    }
}

void screen_data_tx_task(void *pvParameters) {
	uint8_t bpm;
	uint8_t spo2;
	char lora_msg_task[72];
	float body_temp;
    struct bme68x_data air_data;
    char air_q[20]; //air quality string

    NX_Init(); // ekranı başlat.
    uint32_t last_time = HAL_GetTick();

    HAL_UART_Receive_IT(&huart1, &lora_rx_byte, 1); //lora recieve interrupt başlatıldı.
    for (;;) {
    	if(xQueueReceive(qBpm, &bpm, 50) != pdPASS){ //en hızlı gelen veri

    	}
    	if(xQueueReceive(qSpo2, &spo2, 50) != pdPASS){ //spo2 verisi

    	}
    	if(xQueueReceive(qBody, &body_temp, 50) != pdPASS){ //orta hızlı gelen veri

    	}
    	if(xQueueReceive(qEnv,  &air_data, 50) != pdPASS){ // yavaş gelen veri

    	}
    	if (xQueueReceive(qLora, lora_msg_task, 5) == pdPASS)
    	{
    	    int target_id = 0;
    	    char msg[64] = {0};
    	    uint8_t show_msg = 1;

    	    // LoRa modül cevaplarını ve monitoring paketlerini gösterme
    	    if (strcmp(lora_msg_task, "CCESS") == 0 ||
    	        strcmp(lora_msg_task, "CESS") == 0 ||
    	        strcmp(lora_msg_task, "SS") == 0 ||
    	        strcmp(lora_msg_task, "SUCCESS") == 0 ||
				strcmp(lora_msg_task, "ESS") == 0 ||
				strcmp(lora_msg_task, "S") == 0 ||
    	        lora_msg_task[0] == '!')
    	    {
    	        show_msg = 0;
    	    }

    	    // ADM ID=x MSG=... formatındaki yönetici mesajı
    	    else if (sscanf(lora_msg_task, "ADM ID=%d MSG=%63[^\n]", &target_id, msg) == 2)
    	    {
    	        if (target_id != DEVICE_ID)
    	        {
    	            // Mesaj bu cihaza ait değilse pas geç
    	            show_msg = 0;
    	        }
    	        else
    	        {
    	            // Mesaj bu cihaza aitse sadece MSG içeriğini göster
    	        	snprintf(lora_msg_task, sizeof(lora_msg_task), "Admin: %s", msg);
    	        }
    	    }

    	    if (show_msg)
    	    {
    	        NX_send_cmd("page0.n0.val+=1");

    	        NX_send_cmd("page3.m4.txt=page3.m3.txt");
    	        NX_send_cmd("page3.m3.txt=page3.m2.txt");
    	        NX_send_cmd("page3.m2.txt=page3.m1.txt");

    	        char nextionCmd[96];
    	        snprintf(nextionCmd, sizeof(nextionCmd),
    	                 "page3.m1.txt=\"%s\"", lora_msg_task);

    	        NX_send_cmd(nextionCmd);
    	    }
    	}
    	if(bpm == 0 || spo2 == 0){
    		bpm = 0;
    		spo2 = 0;
    	}
    	//queue dan alınan verileri ekrana gönder.
    	NX_set_data(box_pulse, (int16_t)bpm);
    	NX_set_data(box_spo2, (int16_t)spo2);
    	NX_set_data(box_enviroment_temp, (int16_t)(air_data.temperature) + 0.5f);
    	NX_set_data(box_humidity, (int16_t)(air_data.humidity + 0.5f));
    	NX_set_float(box_body_temp, (body_temp));
    	air_quality(&air_data, air_q);
    	NX_set_txt(box_air_quality, air_q);


    	if(HAL_GetTick() - last_time >= 30000)
    	{
    	    last_time = HAL_GetTick();

    	    char monitor_msg[128];

    	    snprintf(monitor_msg, sizeof(monitor_msg),
    	             "!ID=%d BPM=%d SPO2=%d BT=%.2f T=%d H=%d IAQ=%s\n",
    	             DEVICE_ID,
					 bpm,
    	             spo2,
    	             body_temp,
    	             (int16_t)(air_data.temperature + 0.5f),
    	             (int16_t)(air_data.humidity + 0.5f),
    	             air_q);

    	    lora_send_msg(monitor_msg);
    	}

        vTaskDelay(pdMS_TO_TICKS(200)); // BME680 periyodunda
#ifdef UART_LOG
        char msg[40];
        int len = snprintf(msg, sizeof(msg),"[TX] BPM=%d  BT=%.2f  T=%d  H=%d  IAQ=%d\n",
                bpm,
                body_temp,
                (int16_t)(air_data.temperature + 0.5f),
                (int16_t)(air_data.humidity + 0.5f),
                (uint8_t)(air_quality(&air_data) ) );
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, (uint16_t)len, HAL_MAX_DELAY);
#endif
    }
}

void screen_data_rx_task(void* pvParameters){
	HAL_UART_Receive_IT(&huart3, &screen_rx_byte, 1); //screen recieve interrupt başlatıldı.
	for(;;){
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY); //bildirim bekle
		if (screen_rx_byte == END_CHAR) {
			// ✅ Tam paket alındı
			switch (screen_comp) {
			case 0x01: //msg box number reset
				NX_send_cmd("page0.n0.val=0");
				break;

			case 0x02: //send pulse and spo2 info.
				uint8_t bpm;
				float body_temp;
				if(xQueuePeek(qBpm, &bpm, 10) == pdTRUE || xQueuePeek(qBody, &body_temp, 10) == pdTRUE ){
					snprintf(nextion_msg,sizeof(nextion_msg),"Pulse: %d, BODY TEMP: %d",bpm,(uint8_t)body_temp);
				}else{

					snprintf(nextion_msg,sizeof(nextion_msg),"Pulse: ERR, BODY TEMP: ERR");
				}
				lora_send_msg(nextion_msg);
				break;
			case 0x04: // I'M OK //ayıklanan byte da gelen veriye göre aksiyon al
				snprintf(nextion_msg,sizeof(nextion_msg),"ID:%d I'M OK\n",DEVICE_ID);
				lora_send_msg(nextion_msg);
				break;

			case 0x05: // HELP
				snprintf(nextion_msg,sizeof(nextion_msg),"ID:%d HELP\n",DEVICE_ID);
				lora_send_msg(nextion_msg);

				break;

			case 0x06: // DANGER
				snprintf(nextion_msg,sizeof(nextion_msg),"ID:%d DANGER\n",DEVICE_ID);
				lora_send_msg(nextion_msg);
				break;

			case 0x07: // INJURED
				snprintf(nextion_msg,sizeof(nextion_msg),"ID:%d INJURED\n",DEVICE_ID);
				lora_send_msg(nextion_msg);
				break;

			case 0x08: // AREA UNSAFE
				snprintf(nextion_msg,sizeof(nextion_msg),"ID:%d AREA UNSAFE\n",DEVICE_ID);
				lora_send_msg(nextion_msg);
				break;

			case 0x09: // RETURN TO BASE
				snprintf(nextion_msg,sizeof(nextion_msg),"ID:%d RETURN TO BASE\n",DEVICE_ID);
				lora_send_msg(nextion_msg);
				break;

			default:
				break;
			}

		}

	}
}


/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
