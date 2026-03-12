//
// Created by Kok on 2/13/26.
//

#include "ble.h"
#include "gap.h"
#include "gatt.h"

#include "nvs_flash.h"
#include "task_scheduler.h"

#include "host/ble_hs.h"

#include "nimble/nimble_port.h"
#include "nimble/ble.h"

#include "services/dis/ble_svc_dis.h"

/* ------ Library function declarations ------ */
void ble_store_config_init(void);

/* ------ Global variables ------ */
BLE_HandleTypeDef *gHble = NULL;

/* ------ Callbacks ------ */

static void on_stack_sync_cb(void);

/* ------ Tasks ------ */

static void ble_task(void *arg);

/**
 * @brief Initializes the BLE driver by enabling NVS, GAP and GATT
 * @param hble BLE Handle
 */
BLE_ErrorTypeDef BLE_Init(BLE_HandleTypeDef *hble) {

    if (hble->Config.MaxConnections > CONFIG_BT_NIMBLE_MAX_CONNECTIONS) {
        return BLE_ERROR_INVALID_MAX_CONN;
    }

    gHble = hble;

    BLE_ErrorTypeDef ble_err = BLE_ERROR_OK;
    esp_err_t err = ESP_OK;

    uint8_t nvs_ready = 0;
    while (!nvs_ready) {
        // Initialize the flash memory
        if ((err = nvs_flash_init()) != ESP_OK) {
            if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                if ((err = nvs_flash_erase()) != 0) {
                    return BLE_ERROR_NVS;
                };
            }
            continue;
        }
        nvs_ready = 1;
    }

    // Initialize the NimBLE stack
    if ((err = nimble_port_init()) != ESP_OK) {
        return BLE_ERROR_INIT;
    }

    // Set manufacturer data
    ble_svc_dis_init();
    if (hble->Config.ManufacturerData.ModelNumber) ble_svc_dis_model_number_set(hble->Config.ManufacturerData.ModelNumber);
    if (hble->Config.ManufacturerData.ManufacturerName) ble_svc_dis_manufacturer_name_set(hble->Config.ManufacturerData.ManufacturerName);
    if (hble->Config.ManufacturerData.SerialNumber) ble_svc_dis_serial_number_set(hble->Config.ManufacturerData.SerialNumber);
    if (hble->Config.ManufacturerData.FirmwareRevision) ble_svc_dis_firmware_revision_set(hble->Config.ManufacturerData.FirmwareRevision);

    // Initialize GAP
    if ((ble_err = gap_init(gHble)) != BLE_ERROR_OK) return ble_err;

    // Initialize GATT
    if ((ble_err = gatt_init(gHble)) != BLE_ERROR_OK) return ble_err;

    // Configure host
    ble_hs_cfg.reset_cb = hble->Callbacks.on_stack_reset;
    ble_hs_cfg.sync_cb = on_stack_sync_cb;
    ble_hs_cfg.gatts_register_cb = on_gatt_event;
    ble_hs_cfg.gatts_register_arg = gHble;

    ble_hs_cfg.sm_bonding = hble->Config.Security.EncryptedConnection;
    ble_hs_cfg.sm_sc = hble->Config.Security.EncryptedConnection;
    if (hble->Config.Security.ProtectionType != BLE_PROTECTION_JUST_WORKS) {
        ble_hs_cfg.sm_mitm = 1;
        ble_hs_cfg.sm_io_cap = hble->Config.Security.IOCapability;
        ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
        ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    }

    // Store host security configuration
    ble_store_config_init();

    // Create BLE task
    hble->BLE_Task->Function = ble_task;
    SCHEDULER_Create(hble->BLE_Task);

    return BLE_ERROR_OK;
}

BLE_ErrorTypeDef BLE_CheckConnEncrypted(uint16_t hconn, uint8_t *IsEncrypted) {
    if (!hconn) return BLE_ERROR_MISSING_CONN;

    struct ble_gap_conn_desc desc;
    if (ble_gap_conn_find(hconn, &desc) != 0) {
        *IsEncrypted = 0;
        return BLE_ERROR_MISSING_CONN;
    }

    *IsEncrypted = desc.sec_state.encrypted;
    return BLE_ERROR_OK;
}

void on_stack_sync_cb(void) {
    BLE_ErrorTypeDef err;
    if ((err = gap_start_adv(gHble)) != BLE_ERROR_OK && gHble->Callbacks.on_error != NULL) {
        gHble->Callbacks.on_error(err);
    }
}

void ble_task(void *arg) {
    nimble_port_run();
    vTaskDelete(NULL);
}
