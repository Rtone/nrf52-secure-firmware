#include "services.h"
#include "sdk_common.h"
#include "ble_srv_common.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "aes.h"

/********************************************************************************/
/*   FUNCTION PROTOTYPES                                                        */
/********************************************************************************/

static uint32_t
_bleAddService(ble_service_t *p_service, service_uuid_t service_t);

static uint32_t
_bleAddCharacteristic(ble_service_t *p_service, service_char_t char_t,
                      prop_attr_t prop, attr_perm_t attrPerm,
                      attr_char_t attrChar);

static void
_writeHandler_X(uint16_t conn_handle, ble_service_t *p_service,
                ble_gatts_evt_write_t const *dataRec);

static void
_writeHandler_Y(uint16_t conn_handle, ble_service_t *p_service,
                uint8_t dataRec, uint8_t offset);

static void
_writeHandler_Notify(uint16_t conn_handle, ble_service_t *p_service, ble_gatts_evt_write_t const *dataRec);

static uint32_t
_bleNotifyChar(uint16_t conn_handle, ble_gatts_char_handles_t *char_handle, const uint8_t *value);

static void
on_write(ble_service_t *p_service, ble_evt_t const *p_ble_evt);

/********************************************************************************/
/*   --- END of FUNCTION PROTOTYPES ---                                         */
/********************************************************************************/

/********************************************************************************/
/*   SERVICES AND CHARACTERISTICS FUNCTION MANAGEMENT                           */
/********************************************************************************/

static uint32_t
_bleAddService(ble_service_t *p_service, service_uuid_t service_t)
{
  uint32_t err_code;
  ble_uuid_t ble_uuid;

  ble_uuid128_t base_uuid = {SERVICE_UUID_BASE};
  err_code = sd_ble_uuid_vs_add(&base_uuid, &p_service->uuid_type);
  VERIFY_SUCCESS(err_code);

  ble_uuid.type = p_service->uuid_type;
  ble_uuid.uuid = service_t.uuidHandle.uuid;
  // Add the custom service to the system
  err_code =
      sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid,
                               &p_service->serviceHandle);
  VERIFY_SUCCESS(err_code);

  return NRF_SUCCESS;
}

static uint32_t
_bleAddCharacteristic(ble_service_t *p_service, service_char_t char_t,
                      prop_attr_t prop, attr_perm_t attrPerm,
                      attr_char_t attrChar)
{

  ble_gatts_char_md_t charMd;
  ble_gatts_attr_t attrCharValue;
  ble_uuid_t ble_uuid;
  ble_gatts_attr_md_t attrMd;

  // characteristic metadata
  memset(&charMd, 0, sizeof(charMd));

  charMd.char_props.read = prop.read;
  charMd.char_props.write = prop.write;
  charMd.char_props.notify = prop.notify;
  charMd.p_char_user_desc = NULL;
  charMd.p_char_pf = NULL;
  charMd.p_user_desc_md = NULL;
  charMd.p_cccd_md = NULL;
  charMd.p_sccd_md = NULL;

  // characteristic uuid
  ble_uuid.type = p_service->uuid_type;
  ble_uuid.uuid = char_t.uuid16;

  // attribute metadata
  memset(&attrMd, 0, sizeof(attrMd));

  switch (attrPerm.permRead)
  {
  case 0x01:
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attrMd.read_perm);
    break;

  case 0x02:
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);
    break;

  case 0x03:
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attrMd.read_perm);
    break;

  case 0x04:
    BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(&attrMd.read_perm);
    break;

  case 0x05:
    BLE_GAP_CONN_SEC_MODE_SET_LESC_ENC_WITH_MITM(&attrMd.read_perm);
    break;

  case 0x06:
    BLE_GAP_CONN_SEC_MODE_SET_SIGNED_NO_MITM(&attrMd.read_perm);
    break;

  case 0x07:
    BLE_GAP_CONN_SEC_MODE_SET_SIGNED_WITH_MITM(&attrMd.read_perm);
    break;
  }

  switch (attrPerm.permWrite)
  {
  case 0x01:
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attrMd.write_perm);
    break;

  case 0x02:
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.write_perm);
    break;

  case 0x03:
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attrMd.write_perm);
    break;

  case 0x04:
    BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(&attrMd.write_perm);
    break;

  case 0x05:
    BLE_GAP_CONN_SEC_MODE_SET_LESC_ENC_WITH_MITM(&attrMd.write_perm);
    break;

  case 0x06:
    BLE_GAP_CONN_SEC_MODE_SET_SIGNED_NO_MITM(&attrMd.write_perm);
    break;

  case 0x07:
    BLE_GAP_CONN_SEC_MODE_SET_SIGNED_WITH_MITM(&attrMd.write_perm);
    break;
  }

  attrMd.vloc = BLE_GATTS_VLOC_STACK;
  attrMd.rd_auth = 0;
  attrMd.wr_auth = 0;
  attrMd.vlen = 0;

  // attribute data
  memset(&attrCharValue, 0, sizeof(attrCharValue));

  attrCharValue.p_uuid = &ble_uuid;
  attrCharValue.p_attr_md = &attrMd;
  attrCharValue.init_offs = 0;

  if (prop.read == 1 && prop.notify == 0)
  {
    attrCharValue.init_len = sizeof(attrChar.attrCharRead);
    attrCharValue.max_len = sizeof(attrChar.attrCharRead);
    attrCharValue.p_value = attrChar.attrCharRead;
  }
  else if (prop.write == 1 && prop.notify == 0)
  {
    attrCharValue.init_len = 0;
    attrCharValue.max_len = sizeof(attrChar.attrCharWrite);
    attrCharValue.p_value = NULL;
  }
  else
  {
    attrCharValue.init_len = 4;
    attrCharValue.max_len = 4;
    attrCharValue.p_value = NULL;
  }

  if (prop.notify == 0)
  {
    if (prop.write == 1)
    {
      switch (char_t.charID)
      {
      case SERVICE_X_CHAR_B_ID:
        return sd_ble_gatts_characteristic_add(p_service->serviceHandle,
                                               &charMd,
                                               &attrCharValue,
                                               &p_service->charXaWriteHandles);
        break;
      case SERVICE_Y_CHAR_B_ID:
        return sd_ble_gatts_characteristic_add(p_service->serviceHandle,
                                               &charMd,
                                               &attrCharValue,
                                               &p_service->charXaWriteHandles);
        break;
      default:
        return sd_ble_gatts_characteristic_add(p_service->serviceHandle,
                                               &charMd,
                                               &attrCharValue,
                                               &p_service->charDefaultHandles);
      }
    }
    else
    {
      return sd_ble_gatts_characteristic_add(p_service->serviceHandle,
                                             &charMd,
                                             &attrCharValue,
                                             &p_service->charDefaultHandles);
    }
  }
  else
  {
    return sd_ble_gatts_characteristic_add(p_service->serviceHandle,
                                           &charMd,
                                           &attrCharValue,
                                           &p_service->charNotifyHandles);
  }
}

void bleServiceOnBleEvt(ble_evt_t const *p_ble_evt, void *p_context)
{
  ble_service_t *p_service = (ble_service_t *)p_context;

  switch (p_ble_evt->header.evt_id)
  {
  case BLE_GATTS_EVT_WRITE:
    on_write(p_service, p_ble_evt);
    break;

  default:
    // No implementation needed.
    break;
  }
}

uint32_t
bleServiceInit(ble_service_t *p_service, service_uuid_t service_t,
               service_char_t *char_t, prop_attr_t *attrProp,
               attr_perm_t *attrPerm, attr_char_t *attrChar)
{
  uint32_t err_code;

  // Add service.
  err_code = _bleAddService(p_service, service_t);
  VERIFY_SUCCESS(err_code);

  if (service_t.serviceID == SERVICE_X_ID)
  {
    // Add characteristics.
    for (int i = 0; i < MAX_CHARACTERISTIC_X; i++)
    {
      err_code =
          _bleAddCharacteristic(p_service, char_t[i], attrProp[i],
                                attrPerm[i], attrChar[i]);
      VERIFY_SUCCESS(err_code);
    }
  }

  if (service_t.serviceID == SERVICE_Y_ID)
  {
    // Add characteristics.
    for (int i = 0; i < MAX_CHARACTERISTIC_Y; i++)
    {
      err_code =
          _bleAddCharacteristic(p_service, char_t[i], attrProp[i],
                                attrPerm[i], attrChar[i]);
      VERIFY_SUCCESS(err_code);
    }
  }

  if (service_t.serviceID == SERVICE_Z_ID)
  {
    // Add characteristics.
    for (int i = 0; i < MAX_CHARACTERISTIC_Z; i++)
    {
      err_code =
          _bleAddCharacteristic(p_service, char_t[i], attrProp[i],
                                attrPerm[i], attrChar[i]);
      VERIFY_SUCCESS(err_code);
    }
  }

  return NRF_SUCCESS;
}

/********************************************************************************/
/*   --- END of SERVICES AND CHARACTERISTICS FUNCTION MANAGEMENT ---            */
/********************************************************************************/

/********************************************************************************/
/*   HANDLERS                                                                   */
/********************************************************************************/

/*
** handler to manage write events for the "SERVICE_Y_CHAR_B_ID" characteristic 
*/
static void
_writeHandler_X(uint16_t conn_handle, ble_service_t *p_service,
                ble_gatts_evt_write_t const *dataRec)
{

  uint8_t plaintext[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  uint8_t d2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  const uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                         0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

  uint8_t ciphertext[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  for (int i = 0; i < dataRec->len; i++)
  {
    plaintext[i] = dataRec->data[i];
  }

  NRF_LOG_INFO("Value %x %x %x %x", plaintext[0], plaintext[1], plaintext[2], plaintext[3]);

  AES128_ECB_encrypt(plaintext, key, ciphertext);
  NRF_LOG_INFO("Value of ciphertext  %x %x %x %x", ciphertext[0], ciphertext[1],
               ciphertext[2], ciphertext[3]);

  AES128_ECB_decrypt(ciphertext, key, d2);
  NRF_LOG_INFO("Value of plaintext %x %x %x %x", d2[0], d2[1], d2[2],
               d2[3]);
}

/*
**  Handler to manage write events for the "SERVICE_Y_CHAR_B_ID" characteristic 
*/
uint8_t globalPlaintext[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static void
_writeHandler_Y(uint16_t conn_handle, ble_service_t *p_service,
                uint8_t dataRec, uint8_t offset)
{

  NRF_LOG_INFO("data received from service Y %x ", dataRec);
  uint8_t d2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  const uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                         0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

  uint8_t output[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  globalPlaintext[offset] = dataRec;

  NRF_LOG_INFO("Value %x %x %x %x", globalPlaintext[0], globalPlaintext[1], globalPlaintext[2], globalPlaintext[3]);

  if (offset == 3)
  {

    AES128_ECB_encrypt(globalPlaintext, key, output);
    NRF_LOG_INFO("Value of ciphertext  %x %x %x %x", output[0], output[1],
                 output[2], output[3]);

    AES128_ECB_decrypt(output, key, d2);
    NRF_LOG_INFO("Value of plaintext %x %x %x %x", d2[0], d2[1], d2[2],
                 d2[3]);
  }
}

/*
**  Function to send notifications data when write events occur by "_writeHandler_Notify"
** handler 
*/
static uint32_t
_bleNotifyChar(uint16_t conn_handle, ble_gatts_char_handles_t *char_handle, const uint8_t *value)
{
  uint32_t err_code;
  uint8_t len;
  uint16_t hvx_len;
  len = sizeof(value);
  hvx_len = (uint16_t)len;
  ble_gatts_hvx_params_t hvx_params;

  uint8_t plaintext[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  const uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                         0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

  uint8_t ciphertext[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  for (int i = 0; i < sizeof(value); i++)
  {
    plaintext[i] = value[i];
  }

  AES128_ECB_encrypt(plaintext, key, ciphertext);

  NRF_LOG_INFO("Value of ciphertext  %x %x %x %x", ciphertext[0], ciphertext[1],
               ciphertext[2], ciphertext[3]);

  hvx_params.handle = char_handle->value_handle;
  hvx_params.type = BLE_GATT_HVX_NOTIFICATION;
  hvx_params.offset = 0;
  hvx_params.p_len = &hvx_len;
  hvx_params.p_data = ciphertext;

  err_code = sd_ble_gatts_hvx(conn_handle, &hvx_params);
  if (err_code == NRF_ERROR_INVALID_STATE)
  {
    return NRF_SUCCESS;
  }

  return err_code;
}

/*
**  Handler to manage notification events for the "SERVICE_Z_CHAR_A_ID" characteristic 
*/
static void
_writeHandler_Notify(uint16_t conn_handle, ble_service_t *p_service, ble_gatts_evt_write_t const *dataRec)
{
  NRF_LOG_INFO("data received service Z %x ", dataRec->data[0]);
  NRF_LOG_INFO("data received service Z %x ", dataRec->data[1]);
  NRF_LOG_INFO("data received service Z %x ", dataRec->data[2]);
  NRF_LOG_INFO("data received service Z %x ", dataRec->data[3]);

  _bleNotifyChar(conn_handle, &p_service->charNotifyHandles, dataRec->data);
}

/*
**  Event listener on BLE Characteristics write events 
*/
static void
on_write(ble_service_t *p_service, ble_evt_t const *p_ble_evt)
{
  ble_gatts_evt_write_t const *p_evt_write =
      &p_ble_evt->evt.gatts_evt.params.write;

  // Write the bytes array on a single Write-Event
  if ((p_evt_write->len == 4) && (p_evt_write->handle == p_service->charXaWriteHandles.value_handle))
  {
    _writeHandler_X(p_ble_evt->evt.gap_evt.conn_handle, p_service,
                    p_evt_write);
  }

  // Write 1 byte on each Write-Event
  if ((p_evt_write->len == 4) && (p_evt_write->handle == p_service->charXaWriteHandles.value_handle))
  {
    _writeHandler_Y(p_ble_evt->evt.gap_evt.conn_handle, p_service,
                    p_evt_write->data[0], 0);
    _writeHandler_Y(p_ble_evt->evt.gap_evt.conn_handle, p_service,
                    p_evt_write->data[1], 1);
    _writeHandler_Y(p_ble_evt->evt.gap_evt.conn_handle, p_service,
                    p_evt_write->data[2], 2);
    _writeHandler_Y(p_ble_evt->evt.gap_evt.conn_handle, p_service,
                    p_evt_write->data[3], 3);
  }

  // Write the bytes array on a single Write-Event
  if ((p_evt_write->len == 4) && (p_evt_write->handle == p_service->charNotifyHandles.value_handle))
  {
    _writeHandler_Notify(p_ble_evt->evt.gap_evt.conn_handle, p_service,
                         p_evt_write);
  }
}

/********************************************************************************/
/*   --- END of HANDLERS ---                                                    */
/********************************************************************************/
