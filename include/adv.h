#ifndef __ADV_H
#define __ADV_H

typedef struct ble_adv_init_t ble_adv_init;
pm_peer_id_t pmPeerID;
pm_peer_id_t pmWhitelistPeers[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
uint32_t pmWhitelistPeerCnt;

struct ble_adv_init_t
{
  ble_advdata_name_type_t nameType;
  bool appearance;
  bool fast;
  bool slow;
  bool whitelist;
  uint32_t intervalAdv;
  uint32_t timeoutAdv;

  //service for adv
  uint16_t uuid_cnt;
  ble_uuid_t *p_uuids;
};

void advertisingInit(ble_adv_init *adv_init);
void advertisingStart();

#endif /* __ADV_H */
