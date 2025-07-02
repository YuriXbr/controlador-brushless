#include <Wire.h>
#include <MPU6050_light.h>
#include "gyroController.h"
#include "debugHandler.h"
#include "configs.h"
#include "flags.h"

MPU6050 mpu(Wire);

void initGyro() {
    debugPrint("[GYRO] Iniciando MPU6050...");
    // Inicia o MPU6050
    int status = mpu.begin();

    // Verifica se houve erro ao iniciar o MPU6050
    if (status != 0) {
        // Se houve erro, imprime a mensagem de erro e entra em loop infinito
        debugPrint("[GYRO] Erro ao iniciar MPU6050, código: " + String(status));
        MPU_INITIALIZED = false;
        MPU_ERROR = true;
        // Loop infinito para evitar continuar a execução
        while (true) delay(1000);
    }
    // Calcula os offsets do MPU6050
    MPU_INITIALIZED = true;
    MPU_ERROR = false;
    mpu.calcOffsets();
    debugPrint("[GYRO] MPU6050 iniciado com sucesso");
}

float readGyroAngle() {
    // Atualiza os dados do MPU6050
    if (MPU_ERROR) {
        debugPrint("[GYRO] Erro com MPU6050, não é possível ler o ângulo");
        return 0.0; // Retorna 0 se houver erro
    }
    if (!MPU_INITIALIZED) {
        debugPrint("[GYRO] MPU6050 não inicializado, iniciando...");
        initGyro(); // Tenta inicializar o MPU6050 se não estiver inicializado
    }
    // Atualiza os dados do MPU6050 e retorna o ângulo em X
    mpu.update();

    debugPrint("[GYRO] Ângulo X: " + String(mpu.getAngleX()));
    // Retorna o ângulo em X
    return mpu.getAngleX();
}