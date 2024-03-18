#ifndef __MONOPRICE_H
#define __MONOPRICE_H

// Based on info gathered from these sources (I don't actually own one):
// https://www.monoprice.com/product?p_id=10761
// https://github.com/etsinko/pymonoprice
// https://github.com/jnewland/mpr-6zhmaut-api

#include <Arduino.h>
#include "vianet.h"

class Monoprice
{
  public:
    void begin(Stream* port, Vianet* vianet);
    void update();

  private:
    enum States
    {
      MP_init,
      MP_cmd_unit,
      MP_cmd_mzone,
      MP_cmd_coc1,
      MP_cmd_coc2,
      MP_cmd_cv10,
      MP_cmd_cv1,
      MP_cmd_complete,
      MP_qry_unit,
      MP_qry_mzone,
      MP_qry_coc1,
      MP_qry_coc2,
      MP_qry_complete,
    };
    
    void processZoneCommand(int vzone);
    void processZoneStatusQuery(int vzone);

    void processStatusQuery(int vzone);
  
    Stream* _port;
    Vianet* _vianet;
    unsigned short _coc = 0;
    int _value = 0;
    int _mzone = 0;
    States _state = MP_init;
    int _lastSource[19];
    int _lastVolume[19];
    bool _endl = false;
    bool _debug = false;
};


#endif
