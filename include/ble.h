//
// Created by Kok on 2/13/26.
//

#ifndef ESP32S3_BLE_BLE_H
#define ESP32S3_BLE_BLE_H

#include "task_scheduler.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"

#define __weak __attribute__((weak))

typedef enum {
    BLE_ERROR_OK,
    BLE_ERROR_INVALID_MAX_CONN,
    BLE_ERROR_INVALID_STATE,
    BLE_ERROR_MISSING_CONN,
    BLE_ERROR_MISSING_HANDLE,
    BLE_ERROR_NVS,
    BLE_ERROR_INIT,
    BLE_ERROR_GAP_CONN_FULL,
    BLE_ERROR_GAP_CONN_NOT_FOUND,
    BLE_ERROR_GAP_NAME,
    BLE_ERROR_GAP_APPEARANCE,
    BLE_ERROR_GAP_ADDRESS,
    BLE_ERROR_GATTS_COUNT,
    BLE_ERROR_GATTS_ADD_SVCS,
    BLE_ERROR_ADV_ADDR,
    BLE_ERROR_ADV_ADDR_CALC,
    BLE_ERROR_ADV_ADDR_COPY,
    BLE_ERROR_ADV_FIELDS,
    BLE_ERROR_RSP_FIELDS,
    BLE_ERROR_ADV_START
} BLE_ErrorTypeDef;

typedef enum {
    BLE_GAP_ROLE_PERIPHERAL,
    BLE_GAP_ROLE_CENTRAL,
} BLE_GapRoleTypeDef;

typedef enum {
    BLE_GAP_EVENT_CONN_SUCCESS,                         // Passed argument - pointer to struct ble_gap_conn_desc
    BLE_GAP_EVENT_CONN_FAILED,
    BLE_GAP_EVENT_CONN_STORE_FAILED,
    BLE_GAP_EVENT_CONN_STORE_NOTIFY_TOGGLE_FAILED,
    BLE_GAP_EVENT_CONN_ENC,
    BLE_GAP_EVENT_CONN_ENC_FAILED,
    BLE_GAP_EVENT_CONN_DISCONNECT,
    BLE_GAP_EVENT_CONN_UPD,                             // Passed argument - pointer to struct ble_gap_conn_desc
    BLE_GAP_EVENT_SUB,
    BLE_GAP_EVENT_UNSUB,
    BLE_GAP_EVENT_PASSKEY,
} BLE_GapEventTypeDef;

typedef enum {
    BLE_GATT_REG_EVENT_REG_SVC,                             // Register service
    BLE_GATT_REG_EVENT_REG_CHR,                             // Register characteristic
    BLE_GATT_REG_EVENT_REG_DSC,                             // Register descriptor
} BLE_GattRegisterEventTypeDef;

typedef enum {
    BLE_PROTECTION_JUST_WORKS,
    BLE_PROTECTION_YESNO,
    BLE_PROTECTION_PASSKEY
} BLE_ProtectionTypeDef;

typedef enum {
    BLE_IOCAP_DISP_ONLY,
    BLE_IOCAP_DISP_YESNO,                                   // This would require the user to accept or decline the displayed code. This will required in most cases some GPIO interrupt
    BLE_IOCAP_NO_INP_OUT = 3,
} BLE_IOCapabilityTypeDef;

typedef struct {
    uint8_t EncryptedConnection;                            // This enabled device bonding, random private address as well as secure connection flag
    BLE_ProtectionTypeDef ProtectionType;
    BLE_IOCapabilityTypeDef IOCapability;
} BLE_SecurityConfigTypeDef;

typedef struct {
    uint8_t Active;
    uint16_t hconn;
    uint8_t NotificationsEnabled;
} BLE_ConnTypeDef;

typedef struct {
    char *ModelNumber;
    char *ManufacturerName;
    char *SerialNumber;
    char *FirmwareRevision;
} BLE_ManufactureDataTypeDef;

typedef struct {

    /**
     * @brief This callback will be executed whenever the BLE stack gets reset by an error
     */
    void (*on_stack_reset)(int Reason);

    /**
        * @brief This callback will be executed when a GAP event occurs
     */
    void (*on_gap_event)(BLE_GapEventTypeDef Event, struct ble_gap_event *GapEvent, void *Arg);

    /**
     * @brief This callback will be executed whenever some service, characteristic or descriptor was registered
     * @param Event BLE GATT Event
     * @param EventCtxt Passed event context
     * @param Arg Additional arguments
     */
    void (*on_gatt_reg_event)(BLE_GattRegisterEventTypeDef Event, struct ble_gatt_register_ctxt *EventCtxt, void *Arg);

    /**
     * @brief This callback will be executed whenever some device subscribes to some attribute
     * @param event GAP Event
     * @return 0 - if access allowed; BLE_ATT_ERR_INSUFFICIENT_AUTHEN - if access denied
     */
    uint8_t (*on_gatt_subscribe_event)(struct ble_gap_event *event);

    /**
     * @brief This callback will be executed when an error occurs while the driver is running
     * @param Error BLE Error
     */
    void (*on_error)(BLE_ErrorTypeDef Error);

    /**
     * @brief This callback will be executed when configuring GAP advertisement.
     *        It is used to set the required service UUIDs in the advertised fields.
     * @param Fields Advertisement fields
     */
    void (*on_advertise_services)(struct ble_hs_adv_fields *Fields);


} BLE_CallbacksTypeDef;

typedef struct {
    char *DeviceName;
    uint16_t GapAppearance;
    uint8_t PrivateAddressEnabled;
    uint8_t NonResolvablePrivateAddress;                    // Only valid when private address is enabled
    BLE_GapRoleTypeDef GapRole;
    uint16_t AdvertisingIntervalMS;
    uint8_t MaxConnections;                                 // This number should not be greater than CONFIG_NIMBLE_MAX_CONNECTIONS in menuconfig
    BLE_SecurityConfigTypeDef Security;
    BLE_ManufactureDataTypeDef ManufacturerData;            // You should enable these services in menuconfig first
    struct ble_gatt_svc_def *GattServices;
} BLE_ConfigTypeDef;

typedef struct {
    SCHEDULER_TaskTypeDef *BLE_Task;
    BLE_ConfigTypeDef Config;
    BLE_ConnTypeDef Connections[CONFIG_BT_NIMBLE_MAX_CONNECTIONS];
    uint8_t AddressType;
    uint8_t Address[6];
    char AddressStr[20];
    BLE_CallbacksTypeDef Callbacks;
} BLE_HandleTypeDef;

/* ------ Main methods ------ */
BLE_ErrorTypeDef BLE_Init(BLE_HandleTypeDef *hble);
BLE_ErrorTypeDef BLE_CheckConnEncrypted(uint16_t hconn, uint8_t *IsEncrypted);

#endif //ESP32S3_BLE_BLE_H