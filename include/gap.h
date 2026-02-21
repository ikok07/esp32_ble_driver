//
// Created by Kok on 2/14/26.
//

#ifndef ESP32S3_BLE_GAP_H
#define ESP32S3_BLE_GAP_H

#include "ble.h"

BLE_ErrorTypeDef gap_init(BLE_HandleTypeDef *hble);
BLE_ErrorTypeDef gap_start_adv(BLE_HandleTypeDef *hble);

#endif //ESP32S3_BLE_GAP_H