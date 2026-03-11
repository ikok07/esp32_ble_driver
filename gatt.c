//
// Created by Kok on 2/14/26.
//

#include "ble.h"
#include "gatt.h"

#include "host/ble_gatt.h"
#include "services/gatt/ble_svc_gatt.h"

BLE_ErrorTypeDef gatt_init(BLE_HandleTypeDef *hble) {
    uint8_t err = 0;

    // Initialize GATT service
    ble_svc_gatt_init();

    // Update GATT services counter
    if ((err = ble_gatts_count_cfg(hble->Config.GattServices)) != 0) return BLE_ERROR_GATTS_COUNT;

    // Add GATT services
    if ((err = ble_gatts_add_svcs(hble->Config.GattServices)) != 0) return BLE_ERROR_GATTS_ADD_SVCS;

    return BLE_ERROR_OK;
}

void on_gatt_event(struct ble_gatt_register_ctxt *ctxt, void *arg) {
    BLE_HandleTypeDef *hble = arg;
    if (hble->Callbacks.on_gatt_reg_event == NULL) return;

    switch (ctxt->op) {
        case BLE_GATT_REGISTER_OP_SVC:
            hble->Callbacks.on_gatt_reg_event(BLE_GATT_REG_EVENT_REG_SVC, ctxt, NULL);
            break;
        case BLE_GATT_REGISTER_OP_CHR:
            hble->Callbacks.on_gatt_reg_event(BLE_GATT_REG_EVENT_REG_CHR, ctxt, NULL);
            break;
        case BLE_GATT_REGISTER_OP_DSC:
            hble->Callbacks.on_gatt_reg_event(BLE_GATT_REG_EVENT_REG_DSC, ctxt, NULL);
            break;
        default:
            break;
    }
}
