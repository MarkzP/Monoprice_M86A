
#include "monoprice.h"


#define ENDL "\r\n#"

#define M_COC(s) ((s[0] << 8) + s[1])

#define V2M_Z(z) (z == 0 ? 10 : (((((z - 1) / 6) + 1) * 10) + (((z - 1) % 6) + 1)))
#define M2V_Z(z) ((z % 10) == 0 ? 0 : ((((z / 10) - 1) * 6) + (z  % 10)))
#define VUNIT(z) (((z - 1) / 6) + 1)
#define MUNIT(z) ((z % 10) == 0 ? z / 10 : 0)

#define V2M_TB(v) (v < 4 ? 7 : ((v - 4) / 2) + 1)
#define V2M_VOL(v) ((38 * (48 - v)) / 48)

void Monoprice::processZoneCommand(int vzone)
{
    if (!_vianet->isZoneOnline(vzone)) return;
  
    switch (_coc)
    {
      case M_COC("PR"):
        _vianet->setSource(vzone, _value == 0 ? 0 : _lastSource[vzone]);
        break;
      case M_COC("MU"):
        _vianet->setMute(vzone, _value != 0);
        break;
      case M_COC("DT"):
        _vianet->setDoNotDisturb(vzone, _value != 0);
        break;
      case M_COC("VO"):
        _vianet->setVolume(vzone, (100 * _value) / 38);
        break;
      case M_COC("TR"): // 0 - 14 -> 57 - 71
        _vianet->setTreble(vzone, _value + 57);
        break;
      case M_COC("BS"):
        _vianet->setBass(vzone, _value + 57);
        break;
      case M_COC("BL"):
        _vianet->setLoudness(vzone, _value == 10);
        break;
      case M_COC("LO"):
        _vianet->setLoudness(vzone, _value != 0);
        break;
      case M_COC("CH"):
        _vianet->setSource(vzone, _value);
        if (_value != 0) _lastSource[vzone - 1] = _value;
        break;
    }
}

void Monoprice::processZoneStatusQuery(int vzone)
{
  if (!_vianet->isZoneOnline(vzone)) return;

  // Insert an extra line before reporting status
  if (_endl)
  {
    _port->print(ENDL);
    _endl = false;
  }

  _port->printf(">%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d" ENDL,
            V2M_Z(vzone),
            _vianet->getPage(vzone) ? 1 : 0,
            _vianet->getSource(vzone) != 0 ? 1 : 0,
            _vianet->getMute(vzone) ? 1 : 0,
            _vianet->getDoNotDisturb(vzone) ? 1 : 0,
            V2M_VOL(_vianet->getVolume(vzone)),
            V2M_TB(_vianet->getTreble(vzone)),
            V2M_TB(_vianet->getBass(vzone)),
            _vianet->getLoudness(vzone) ? 10 : 5, //report loudness instead of balance
            _vianet->getSource(vzone) != 0 ? _vianet->getSource(vzone) : _lastSource[vzone],
            _vianet->getSenseInput(vzone) ? 11 : 10 // report sense input instead of keypad status - this could also be used to differentiate this from the real Monoprice amp
            );
}

void Monoprice::processStatusQuery(int vzone)
{
  if (!_vianet->isZoneOnline(vzone)) return;
  
  int value = 0;
  switch (_coc)
  {
    case M_COC("PR"): value = _vianet->getSource(vzone) != 0 ? 1 : 0; break;
    case M_COC("MU"): value = _vianet->getMute(vzone) ? 1 : 0; break;
    case M_COC("DT"): value = _vianet->getDoNotDisturb(vzone) ? 1 : 0; break;
    case M_COC("VO"): value = V2M_VOL(_vianet->getVolume(vzone)); break;
    case M_COC("TR"): value = V2M_TB(_vianet->getTreble(vzone)); break;
    case M_COC("BS"): value = V2M_TB(_vianet->getBass(vzone)); break;
    case M_COC("BL"): value = _vianet->getLoudness(vzone) ? 10 : 5; break;
    case M_COC("LO"): value = _vianet->getLoudness(vzone) ? 1 : 0; break;
    case M_COC("CH"): value = _vianet->getSource(vzone) != 0 ? _vianet->getSource(vzone) : _lastSource[vzone]; break;
    case M_COC("LS"): value = _vianet->getSenseInput(vzone) ? 11 : 10; break;
    default: return;
  }

  _port->printf(">%02d%c%c%02d" ENDL, V2M_Z(vzone), (char)((_coc & 0xff00) >> 8), (char)(_coc & 0x00ff), value);
}

void Monoprice::begin(Stream* port, Vianet* vianet)
{
  _port = port;
  _vianet = vianet;
}

void Monoprice::update()
{
  if (_vianet == nullptr || _port == nullptr) return;
  
  _vianet->update();

  for (int i = 1; i <= 18; i++)
  {
    int zsrc = _vianet->getSource(i);
    if (zsrc != 0) _lastSource[i] = zsrc;
    else if (_lastSource[i] == 0) _lastSource[i] = 1;
  }
  
  if (!_port->available()) return;

  char c = (char)_port->read();
  if (c >= 'a' && c <= 'z') c &= 0x05f;

  if (c == '\r')
  { 
    int munit = MUNIT(_mzone);
    int vzone = M2V_Z(_mzone);

    if (_state == MP_cmd_complete)
    {
      for (int i = 1; i <= 18; i++)
      {
        if (vzone == i || VUNIT(i) == munit) processZoneCommand(i);
      }
      for (int i = 0; i < 5; i++)
      {
        if (_vianet->refresh()) break;
      }
      for (int i = 1; i <= 18; i++)
      {
        if (vzone == i || VUNIT(i) == munit) processStatusQuery(i);
      }
    }
    else if (_state == MP_qry_coc1)
    {
      if (_mzone == 0)
      {
        _vianet->print(_port);
        _vianet->setDebug(_port);
        _debug = true;     
      }
      else
      {
        _endl = true;
        for (int i = 1; i <= 18; i++)
        {
          if (vzone == i || VUNIT(i) == munit) processZoneStatusQuery(i);
        }
      }
    }
    else if (_state == MP_qry_complete)
    {
      for (int i = 1; i <= 18; i++)
      {
        if (vzone == i || VUNIT(i) == munit) processStatusQuery(i);
      }          
    }
    _state = MP_init;
  }
  else
  switch (_state)
  {
    case MP_init:
      // init
      _coc = 0;
      _value = 0;
      _mzone = 0;
      // valid chars: '<', '?'
      if (c == '<') _state = MP_cmd_unit;
      else if (c == '?') _state = MP_qry_unit;
      break;
    case MP_cmd_unit:
      // command - unit
      // valid chars: '1' to '4'
      if (c >= '1' && c <= '4')
      {
        _mzone = 10 * (c - '0');
        _state = MP_cmd_mzone;
      }
      else _state = MP_init;
      break;
    case MP_cmd_mzone:
      // command - zone
      // valid chars: '0' to '6'
      if (c >= '0' && c <= '6')
      {
        _mzone += c - '0';
        _state = MP_cmd_coc1;
      }
      else _state = MP_init;
      break;     
    case MP_cmd_coc1:
      // Control object code 1
      if (c >= 'A' && c <='Z')
      {
        _coc = (unsigned short)c << 8;
        _state = MP_cmd_coc2;
      }
      else _state = MP_init;
      break;
    case MP_cmd_coc2:
      // Control object code 2
      if (c >= 'A' && c <='Z')
      {
        _coc += (unsigned short)c;
        _state = MP_cmd_cv10;
      }
      else _state = MP_init;
      break;
    case MP_cmd_cv10:
      // Control value decades
      if (c >= '0' && c <= '9')
      {
        _value = 10 * (c - '0');
        _state = MP_cmd_cv1;
      }
      else _state = MP_init;
      break;
    case MP_cmd_cv1:
      // Control value units
      if (c >= '0' && c <= '9')
      {
        _value += c - '0';
        _state = MP_cmd_complete;
      }
      else _state = MP_init;
      break;
    case MP_qry_unit:
      // inquiry - unit
      // valid chars: '0' to '4' -- unit 0 is used for debugging
      if (c >= '0' && c <= '4')
      {
        _mzone = 10 * (c - '0');
        _state = MP_qry_mzone;
      }
      else _state = MP_init;
      break;
    case MP_qry_mzone:
      // inquiry - zone
      // valid chars: '0' to '6'
      if (c >= '0' && c <= '6')
      {
        _mzone += c - '0';
        _state = MP_qry_coc1;
      }
      else _state = MP_init;
      break;
    case MP_qry_coc1:
      // Control object code 1
      if (c >= 'A' && c <='Z')
      {     
        _coc = (unsigned short)c << 8;
        _state = MP_qry_coc2;
      }
      else _state = MP_init;
      break;
    case MP_qry_coc2:
      // Control object code 2
      if (c >= 'A' && c <='Z')
      {
        _coc += (unsigned short)c;
        _state = MP_qry_complete;
      }
      else _state = MP_init;
      break;

    default:
      _state = MP_init;
      break;
  }
}
