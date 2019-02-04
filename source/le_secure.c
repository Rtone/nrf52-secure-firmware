#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "ble_advertising.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "bsp_btn_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_log.h"
#include "nrf_crypto.h"
#include "nrf_crypto_keys.h"
#include "le_secure.h"




volatile uint16_t m_conn_handle_num_comp_central = BLE_CONN_HANDLE_INVALID;
volatile uint16_t m_conn_handle_num_comp_peripheral = BLE_CONN_HANDLE_INVALID;
volatile uint16_t m_conn_handle_dhkey_req_central = BLE_CONN_HANDLE_INVALID;
volatile uint16_t m_conn_handle_dhkey_req_peripheral =
  BLE_CONN_HANDLE_INVALID;
__ALIGN (4)
     ble_gap_lesc_p256_pk_t m_lesc_public_key;
__ALIGN (4)
     ble_gap_lesc_dhkey_t m_lesc_dh_key;


/*
 *
 * LE Secure Connection Mode :
 *
 */

/**@brief Allocated private key type to use for LESC DH generation
 */
NRF_CRYPTO_ECC_PRIVATE_KEY_CREATE (m_private_key, SECP256R1);

/**@brief Allocated public key type to use for LESC DH generation
 */
NRF_CRYPTO_ECC_PUBLIC_KEY_CREATE (m_public_key, SECP256R1);

/**@brief Allocated peer public keys to use for LESC DH generation.
 * Create two keys to allow concurrent reception of DHKEY_REQUEST.
 */
NRF_CRYPTO_ECC_PUBLIC_KEY_CREATE (m_peer_public_key_central, SECP256R1);
NRF_CRYPTO_ECC_PUBLIC_KEY_CREATE (m_peer_public_key_peripheral, SECP256R1);

/**@brief Allocated raw public key to use for LESC DH.
 */
NRF_CRYPTO_ECC_PUBLIC_KEY_RAW_CREATE_FROM_ARRAY (m_public_key_raw, SECP256R1,
						 m_lesc_public_key.pk);

/**@brief Allocated shared instance to use for LESC DH.
 */
NRF_CRYPTO_ECDH_SHARED_SECRET_CREATE_FROM_ARRAY (m_dh_key, SECP256R1,
						 m_lesc_dh_key.key);


/** @brief Function to accept or reject a numeric comparison. */
     void numCompReply (uint16_t conn_handle, bool accept)
{
  uint8_t key_type;
  ret_code_t err_code;

  if (accept)
    {
      NRF_LOG_INFO ("Numeric Match. Conn handle: %d", conn_handle);
      key_type = BLE_GAP_AUTH_KEY_TYPE_PASSKEY;
    }
  else
    {
      NRF_LOG_INFO ("Numeric REJECT. Conn handle: %d", conn_handle);
      key_type = BLE_GAP_AUTH_KEY_TYPE_NONE;
    }

  err_code = sd_ble_gap_auth_key_reply (conn_handle, key_type, NULL);
  APP_ERROR_CHECK (err_code);
}


/** @brief Function to handle a numeric comparison match request. */
void
onMatchRequest (uint16_t conn_handle, uint8_t role)
{
  // Mark the appropriate conn_handle as pending. The rest is handled on button press.
  NRF_LOG_INFO ("Press Button 3 to confirm, Button 4 to reject");
  if (role == BLE_GAP_ROLE_CENTRAL)
    {
      m_conn_handle_num_comp_central = conn_handle;
    }
  else if (role == BLE_GAP_ROLE_PERIPH)
    {
      m_conn_handle_num_comp_peripheral = conn_handle;
    }
}


/** @brief Function to handle a request for calculation of a DH key. */
void
onDhkeyRequest (uint16_t conn_handle,
		ble_gap_evt_lesc_dhkey_request_t const *p_dhkey_request,
		uint8_t role)
{
  ret_code_t err_code;
  nrf_value_length_t peer_public_key_raw = { 0 };

  peer_public_key_raw.p_value = &p_dhkey_request->p_pk_peer->pk[0];
  peer_public_key_raw.length = BLE_GAP_LESC_P256_PK_LEN;

  // Prepare the key and mark it for calculation. The calculation will be performed in main to
  // not block normal operation.
  if (role == BLE_GAP_ROLE_CENTRAL)
    {
      err_code =
	nrf_crypto_ecc_public_key_from_raw (NRF_CRYPTO_BLE_ECDH_CURVE_INFO,
					    &peer_public_key_raw,
					    &m_peer_public_key_central);
      APP_ERROR_CHECK (err_code);

      m_conn_handle_dhkey_req_central = conn_handle;
    }
  else if (role == BLE_GAP_ROLE_PERIPH)
    {
      err_code =
	nrf_crypto_ecc_public_key_from_raw (NRF_CRYPTO_BLE_ECDH_CURVE_INFO,
					    &peer_public_key_raw,
					    &m_peer_public_key_peripheral);
      APP_ERROR_CHECK (err_code);

      m_conn_handle_dhkey_req_peripheral = conn_handle;
    }
}



void
cryptoInit (void)
{
  NRF_LOG_INFO ("Initializing nrf_crypto.");
  ret_code_t err_code = nrf_crypto_init ();
  APP_ERROR_CHECK (err_code);
  NRF_LOG_INFO ("Initialized nrf_crypto.");
}



/** @brief Function to handle button presses for numeric comparison match requests. */
void
onNumCompButtonPress (bool accept)
{
  // Check whether any links have pending match requests, and if so, send a reply.
  if (m_conn_handle_num_comp_central != BLE_CONN_HANDLE_INVALID)
    {
      numCompReply (m_conn_handle_num_comp_central, accept);
      m_conn_handle_num_comp_central = BLE_CONN_HANDLE_INVALID;
    }
  else if (m_conn_handle_num_comp_peripheral != BLE_CONN_HANDLE_INVALID)
    {
      numCompReply (m_conn_handle_num_comp_peripheral, accept);
      m_conn_handle_num_comp_peripheral = BLE_CONN_HANDLE_INVALID;
    }
}




/**@brief Function to generate private key */
uint32_t
lescGenerateKeyPair (void)
{
  uint32_t ret_val;
  NRF_LOG_INFO ("Generating key-pair");
  //Generate a public/private key pair.
  ret_val =
    nrf_crypto_ecc_key_pair_generate (NRF_CRYPTO_BLE_ECDH_CURVE_INFO,
				      &m_private_key, &m_public_key);
  APP_ERROR_CHECK (ret_val);

  // Convert to a raw type
  NRF_LOG_INFO ("Converting to raw type");
  ret_val =
    nrf_crypto_ecc_public_key_to_raw (NRF_CRYPTO_BLE_ECDH_CURVE_INFO,
				      &m_public_key, &m_public_key_raw);
  APP_ERROR_CHECK (ret_val);

  // Set the public key in the PM.
  ret_val = pm_lesc_public_key_set (&m_lesc_public_key);
  APP_ERROR_CHECK (ret_val);
  return ret_val;
}


/** @brief Function to calculate a dhkey and give it to the SoftDevice. */
void
computeAndGiveDhkey (nrf_value_length_t * p_peer_public_key,
		     uint16_t conn_handle)
{
  ret_code_t err_code =
    nrf_crypto_ecdh_shared_secret_compute (NRF_CRYPTO_BLE_ECDH_CURVE_INFO,
					   &m_private_key,
					   p_peer_public_key,
					   &m_dh_key);
  APP_ERROR_CHECK (err_code);

  NRF_LOG_INFO ("Calling sd_ble_gap_lesc_dhkey_reply on conn_handle: %d",
		conn_handle);
  err_code = sd_ble_gap_lesc_dhkey_reply (conn_handle, &m_lesc_dh_key);
  APP_ERROR_CHECK (err_code);
}


/** @brief Function to check whether a key needs calculation, and calculate it. */
void
serviceDhkeyRequests (void)
{
  if (m_conn_handle_dhkey_req_central != BLE_CONN_HANDLE_INVALID)
    {
      // The central link has received a DHKEY_REQUEST.
      computeAndGiveDhkey (&m_peer_public_key_central,
			   m_conn_handle_dhkey_req_central);
      m_conn_handle_dhkey_req_central = BLE_CONN_HANDLE_INVALID;
    }
  else if (m_conn_handle_dhkey_req_peripheral != BLE_CONN_HANDLE_INVALID)
    {
      // The peripheral link has received a DHKEY_REQUEST.
      computeAndGiveDhkey (&m_peer_public_key_peripheral,
			   m_conn_handle_dhkey_req_peripheral);
      m_conn_handle_dhkey_req_peripheral = BLE_CONN_HANDLE_INVALID;
    }
}
