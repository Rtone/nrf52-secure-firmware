#ifndef __LE_SECURE_H
#define __LE_SECURE_H

void numCompReply(uint16_t conn_handle, bool accept);
void onMatchRequest(uint16_t conn_handle, uint8_t role);
void onDhkeyRequest(uint16_t conn_handle,
					ble_gap_evt_lesc_dhkey_request_t const *p_dhkey_request,
					uint8_t role);
void cryptoInit();
void onNumCompButtonPress(bool accept);

uint32_t lescGenerateKeyPair(void);
void computeAndGiveDhkey(nrf_value_length_t *p_peer_public_key,
						 uint16_t conn_handle);
void serviceDhkeyRequests(void);

#endif /* __LE_SECURE_H */
