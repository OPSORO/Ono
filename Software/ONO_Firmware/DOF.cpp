/*

Copyright (C) 2014 Cesar Vandevelde

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "DOF.h"

DOF::DOF(int pin, int minpos, int midpos, int maxpos){
  _pin = pin;
  _min = minpos;
  _max = maxpos;
  _mid = midpos;
  _pos = 0;
}

void DOF::Init(){
  SendPulse(_mid);
}

void DOF::SendPulse(int pulse){
  pulse = constrain(pulse, 500, 2500);
  Serial.print("#");
  Serial.print(_pin);
  Serial.print(" P");
  Serial.print(pulse);
  Serial.print(" ");
  // Does NOT send timing or \r char!
}

void DOF::SendPos(int pos){
  // Pos: -100 = minpos
  //         0 = midpos
  //       100 = maxpos
  pos = constrain(pos, -100, 100);
  _pos = pos;
  if(pos == 0){
    Init(); // Set to mid
  }else if(pos < 0){
    SendPulse(map(pos, 0, -100, _mid, _min));
  }else if(pos > 0){
    SendPulse(map(pos, 0, 100, _mid, _max));
  }
}

int DOF::GetPos(){
  return _pos;
}
