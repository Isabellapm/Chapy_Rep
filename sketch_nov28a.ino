#define TFT_DC 8    // Pin DC (Datos/Comandos)
#define TFT_RST 9   // Pin de reinicio (RESET)
#define SCR_WD 240  // Ancho de la pantalla
#define SCR_HT 240  // Altura de la pantalla (se menciona 320 en el comentario para manejar buffers completos)
#define GYRO_X_OFFSET 220
#define GYRO_Y_OFFSET 76
#define GYRO_Z_OFFSET -85
#define ACCEL_Z_OFFSET 1688


// Inclusión de bibliotecas necesarias
#include <SPI.h>                  // Protocolo de comunicación SPI
#include <Adafruit_GFX.h>         // Biblioteca gráfica para pantallas
#include <Arduino_ST7789_Fast.h>  // Biblioteca para controlar pantallas ST7789

// Inicialización del objeto para la pantalla ST7789
Arduino_ST7789 lcd = Arduino_ST7789(TFT_DC, TFT_RST);

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

MPU6050 mpu;

bool dmpReady = false;
uint8_t mpuIntStatus;
uint8_t devStatus;
uint16_t packetSize;
uint16_t fifoCount;
uint8_t fifoBuffer[64];

Quaternion q;
VectorFloat gravity;
float ypr[3];

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);

  Serial.println("Iniciando MPU6050...");
  mpu.initialize();

  devStatus = mpu.dmpInitialize();

  // Calibración opcional

    mpu.setXGyroOffset(GYRO_X_OFFSET);
    mpu.setYGyroOffset(GYRO_Y_OFFSET);
    mpu.setZGyroOffset(GYRO_Z_OFFSET);
    mpu.setZAccelOffset(ACCEL_Z_OFFSET);

  if (devStatus == 0) {
    Serial.println("DMP listo, iniciando...");
    mpu.setDMPEnabled(true);

    mpuIntStatus = mpu.getIntStatus();
    dmpReady = true;
    packetSize = mpu.dmpGetFIFOPacketSize();
  } 
  else {
    Serial.print("Error al iniciar DMP: ");
    Serial.println(devStatus);
  }
  
  lcd.init(SCR_WD, SCR_HT);       // Inicializa la pantalla con las dimensiones especificadas
  lcd.fillScreen(RED);            // Llena la pantalla con color negro
  lcd.setCursor(0, 0);            // Establece la posición inicial del cursor en la esquina superior izquierda
  lcd.setTextColor(WHITE, BLUE);  // Configura los colores del texto (texto blanco, fondo azul)
  lcd.setTextSize(2);             // Establece el tamaño del texto (3x el tamaño base)
}

void loop() {
  if (!dmpReady) return;

  fifoCount = mpu.getFIFOCount();

  if (fifoCount == 1024) {
    mpu.resetFIFO();
    return;
  }

  if (fifoCount < packetSize) return;

  mpu.getFIFOBytes(fifoBuffer, packetSize);

  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

  lcd.println("\tGiro: ");
  lcd.println(ypr[1] * 180 / M_PI);
  lcd.setCursor(0, 0);
}

