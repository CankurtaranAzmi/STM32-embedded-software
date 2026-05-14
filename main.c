/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// Gerekli kütüphaneler zaten main.h içinde tanımlıdır.
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FLASH_ADDR 0x0800FC00 // Flash Sayfa 63 (64KB'lık işlemcinin son sayfası)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
uint16_t blink_count = 4;
uint16_t timer_seconds = 0;
uint8_t toggle_count = 0;
enum { BLINKING, PAUSED } system_state = BLINKING;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void Flash_Write(uint32_t address, uint16_t value);
uint16_t Flash_Read(uint32_t address);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init();

  /* USER CODE BEGIN 2 */
  // h) PA1 pini lojik 0 olarak kalsın.
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

  // g) Başlangıçta 3 saniye buton kontrolü (Fabrika Ayarları)
  uint32_t start_tick = HAL_GetTick();
  while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
      if(HAL_GetTick() - start_tick >= 3000) {
          Flash_Write(FLASH_ADDR, 4);
          break;
      }
  }

  // e & f) Flash'tan değeri oku ve geçerliliğini kontrol et
  blink_count = Flash_Read(FLASH_ADDR);
  if(blink_count > 7 || blink_count < 4) {
      blink_count = 4;
      Flash_Write(FLASH_ADDR, 4);
  }

  // Timer interrupt'ı başlat
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // d) Buton kontrolü (4-7 arası değer yönetimi)
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
        HAL_Delay(50); // Debounce
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
            
            blink_count++;
            if (blink_count > 7) blink_count = 4;

            // e) Değer her değiştiğinde Flash'a yaz
            Flash_Write(FLASH_ADDR, blink_count);
            
            // Butonun bırakılmasını bekle
            while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET);
        }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        if (system_state == BLINKING) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            toggle_count++;
            
            if (toggle_count >= (blink_count * 2)) {
                toggle_count = 0;
                system_state = PAUSED;
                timer_seconds = 0;
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); 
            }
        } else {
            timer_seconds++;
            if (timer_seconds >= 5) {
                system_state = BLINKING;
            }
        }
    }
}
void Flash_Write(uint32_t address, uint16_t value) {
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef eraseInit;
    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.PageAddress = address;
    eraseInit.NbPages = 1;
    uint32_t pageError;
    
    HAL_FLASHEx_Erase(&eraseInit, &pageError);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, value);
    HAL_FLASH_Unlock();
    HAL_FLASH_Lock();
}

uint16_t Flash_Read(uint32_t address) {
    return *(__IO uint16_t*)address;
}
/* USER CODE END 4 */