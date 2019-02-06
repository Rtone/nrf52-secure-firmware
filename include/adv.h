#ifndef __ADV_H
#define __ADV_H

typedef struct ble_adv_init_t ble_adv_init;

struct ble_adv_init_t
{
  ble_advdata_name_type_t nameType;
  bool appearance;
  bool fast;
  bool slow;
  uint32_t intervalAdv;
  uint32_t timeoutAdv;

  //service for adv
  uint16_t uuid_cnt;
  ble_uuid_t *p_uuids;
};

void advertisingInit(ble_adv_init *adv_init);
void advertisingStart();

#endif /* __ADV_H */
