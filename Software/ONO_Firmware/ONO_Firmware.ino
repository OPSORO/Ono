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

#include <Bounce.h>
#include <math.h>
#include "DOF.h"

// Digital pin joystick select button
// Default high, pressed low
#define JOYSTICK_SEL 2

// Analog pins joystick
#define JOYSTICK_H 0
#define JOYSTICK_V 1

// Joystick middle values
#define ZERO_H 509
#define ZERO_V 504

// Time between updates
#define UPDATE_TIMING 500

// Button debounce
Bounce selBouncer = Bounce(JOYSTICK_SEL, 5);

// Timing variable
unsigned long timeSince = 0;

// DOF indices
enum DOF_indices {  L_BROW_OUTER = 0, 
                    L_BROW_INNER = 1,
                    
                    R_BROW_OUTER = 2,
                    R_BROW_INNER = 3,
                    
                    L_EYE_LID = 4,
                    L_EYE_HOR = 5,
                    L_EYE_VER = 6,
                    
                    R_EYE_LID = 7,
                    R_EYE_HOR = 8,
                    R_EYE_VER = 9,
                    
                    MOUTH_L = 10,
                    MOUTH_R = 11,
                    MOUTH_MID = 12
                  };

// DOF configuration
DOF DOFs[13] = {
  // PIN   MIN  MID   MAX
  DOF(0,  1800, 1500, 1200), //L_BROW_OUTER ok
  DOF(1,  1300, 1600, 1900), //L_BROW_INNER ok
  DOF(3,  1200, 1500, 1800), //R_BROW_OUTER ok
  DOF(4,  1800, 1500, 1200), //R_BROW_INNER ok
  DOF(6,  1800, 1500, 1200), //L_EYE_LID ok
  DOF(7,  1150, 1500, 1750), //L_EYE_HOR ok
  DOF(8,  1900, 1600, 1300), //L_EYE_VER ok
  DOF(9,  1200, 1500, 1800), //R_EYE_LID ok
  DOF(10, 1250, 1500, 1850), //R_EYE_HOR ok
  DOF(11, 1300, 1600, 1900), //R_EYE_VER ok
  DOF(12, 2100, 1500, 900), //MOUTH_L ok
  DOF(13, 900, 1500, 2100), //MOUTH_R ok
  DOF(14, 1800, 1500, 1100) //MOUTH_MID ok
};

// Face configuration
const int Faces[][13] = {
  //                       L_BROW_OUTER | L_BROW_INNER | R_BROW_OUTER | R_BROW_INNER | L_EYE_LID | L_EYE_HOR | L_EYE_VER | R_EYE_LID | R_EYE_HOR | R_EYE_VER | MOUTH_L | MOUTH_R | MOUTH_MID
  /*  0: neutral       */ {0,             0,             0,             0,             50,         0,          0,          50,         0,          0,          20,       20,       50       }, // ok
  /*  1: valence       */ {0,             25,            0,             25,            50,         0,          0,          50,         0,          0,          50,       50,       100      }, // ok
  /*  2: happy         */ {0,             50,            0,             50,            50,         0,          0,          50,         0,          0,          70,       70,       50       }, // ok
  /*  3: elated        */ {0,             70,            0,             70,            50,         0,          0,          50,         0,          0,          70,       70,      -100      }, // ok
  /*  4: excited       */ {0,             70,            0,             70,            50,         0,          0,          50,         0,          0,          40,       40,      -100      }, // ok
  /*  5: alert         */ {25,            70,            25,            70,            50,         0,          0,          50,         0,          0,          0,        0,       -50       }, // ok
  /*  6: surprise      */ {25,            70,            25,            70,            80,         0,          0,          80,         0,          0,          0,        0,       -100      }, // ok
  /*  7: tense         */ {25,            50,            25,            50,            80,         0,          0,          80,         0,          0,         -50,      -50,      -80       }, // ok
  /*  8: nervous       */ {25,            25,            25,            25,            80,         0,          0,          80,         0,          0,         -50,      -50,      -80       }, // ok
  /*  9: stressed      */ {25,           -50,            25,           -50,            50,         0,          0,          50,         0,          0,         -50,      -50,      -80       }, // ok
  /* 10: upset         */ {25,           -50,            25,           -50,            50,         0,          0,          50,         0,          0,         -50,      -50,       50       }, // ok
  /* 11: unpleasant    */ {25,           -25,            25,           -25,            50,         0,          0,          50,         0,          0,         -50,      -50,       100      }, // ok
  /* 12: sad           */ {-25,           25,           -25,            25,            40,         0,          0,          40,         0,          0,         -100,     -100,      100      }, // ok
  /* 13: depressed     */ {-50,          -25,           -50,            25,            40,         0,          0,          40,         0,          0,         -100,     -100,      100      }, // ok
  /* 14: bored         */ {-25,          -25,           -25,           -25,            40,         0,          0,          40,         0,          0,         -25,      -25,       100      }, // ok
  /* 15: fatigued      */ {-25,           25,           -25,            25,            25,         0,          0,          25,         0,          0,         -25,      -25,       100      }, // ok
  /* 16: tired         */ {-25,           25,           -25,            25,            25,         0,          0,          25,         0,          0,          25,       25,       100      }, // ok
  /* 17: calm          */ {-25,           25,           -25,            25,            50,         0,          0,          50,         0,          0,          25,       25,       100      }, // ok
  /* 18: relaxed       */ {0,             25,            0,             25,            50,         0,          0,          50,         0,          0,          50,       50,       100      }, // ok
  /* 19: serene        */ {25,            25,            25,            25,            50,         0,          0,          50,         0,          0,          50,       50,       100      }, // ok
  /* 20: contented     */ {25,            50,            25,            50,            50,         0,          0,          50,         0,          0,          50,       50,       100      }  // ok
};

void setFace(int emotion, float intensity);

void setup(){
  Serial.begin(9600);
  pinMode(JOYSTICK_SEL, INPUT);
  delay(100);
  
  DOFs[L_BROW_OUTER].Init();
  DOFs[L_BROW_INNER].Init();
  Serial.print("T500\r");delay(500);
  
  DOFs[R_BROW_OUTER].Init();
  DOFs[R_BROW_INNER].Init();
  Serial.print("T500\r");delay(500);
  
  DOFs[L_EYE_LID].Init();
  DOFs[L_EYE_HOR].Init();
  DOFs[L_EYE_VER].Init();
  Serial.print("T500\r");delay(500);
  
  DOFs[R_EYE_LID].Init();
  DOFs[R_EYE_HOR].Init();
  DOFs[R_EYE_VER].Init();
  Serial.print("T500\r");delay(500);
  
  DOFs[MOUTH_L].Init();
  DOFs[MOUTH_R].Init();
  DOFs[MOUTH_MID].Init();
  Serial.print("T500\r");delay(500);
  
  setFace(0, 1.0f);
}

int currentface = 0;

void loop(){
  selBouncer.update();
  
  if(selBouncer.risingEdge()){
    Serial.println("fqdqsf");
    // Joystick button released
    DOFs[L_EYE_HOR].SendPos(100);
    DOFs[R_EYE_HOR].SendPos(100);
    Serial.print("T300\r");delay(1000);
    DOFs[L_EYE_HOR].SendPos(-100);
    DOFs[R_EYE_HOR].SendPos(-100);
    Serial.print("T300\r");delay(1000);
    DOFs[L_EYE_HOR].SendPos(0);
    DOFs[R_EYE_HOR].SendPos(0);
    Serial.print("T300\r");delay(300);
    DOFs[L_EYE_LID].SendPos(-100);
    DOFs[R_EYE_LID].SendPos(-100);
    Serial.print("T300\r");delay(300);
    DOFs[L_EYE_LID].SendPos(50);
    DOFs[R_EYE_LID].SendPos(50);
    Serial.print("T300\r");delay(300);
    DOFs[L_EYE_LID].SendPos(-100);
    DOFs[R_EYE_LID].SendPos(-100);
    Serial.print("T300\r");delay(300);
    DOFs[L_EYE_LID].SendPos(50);
    DOFs[R_EYE_LID].SendPos(50);
    Serial.print("T300\r");delay(300);
  }
  
  if(millis() - timeSince > UPDATE_TIMING){
    timeSince = millis();
    
    //long hor = map(analogRead(JOYSTICK_H), 0, 1023, 1023, 0) - ZERO_H;
    long hor = analogRead(JOYSTICK_H) - ZERO_H;
    long ver = analogRead(JOYSTICK_V) - ZERO_V;
    float length = sqrt(hor*hor + ver*ver);
    float angle = atan2(ver, hor) * 180.0f/PI;
    
    int index = 0;
    float intensity = 0.0f;
    getEmotion(angle, length, &index, &intensity);
    setFace(index, intensity);
  }
}

void getEmotion(float angle, float length, int* out_index, float* out_intensity){
  int neutral = 50;
  int maxval = 500;
  
  int newlength = constrain(int(length), 0, maxval);
  if(newlength < neutral){
    *out_index = 0; // Neutral emotion
    *out_intensity = 1.0f;
  }else{
    *out_index = angleToIndex(angle);
    *out_intensity = float(newlength - neutral)/float(maxval - neutral);
  }
}

int angleToIndex(float angle){
  if(angle > -9.0f && angle <= 9.0f){return 1;}
  if(angle > 9.0f && angle <= 27.0f){return 2;}
  if(angle > 27.0f && angle <= 45.0f){return 3;}
  if(angle > 45.0f && angle <= 63.0f){return 4;}
  if(angle > 63.0f && angle <= 81.0f){return 5;}
  if(angle > 81.0f && angle <= 99.0f){return 6;}
  if(angle > 99.0f && angle <= 117.0f){return 7;}
  if(angle > 117.0f && angle <= 135.0f){return 8;}
  if(angle > 135.0f && angle <= 153.0f){return 9;}
  if(angle > 153.0f && angle <= 171.0f){return 10;}
  if(angle > 171.0f && angle <= 180.0f){return 11;}
  if(angle > -180.0f && angle <= -171.0f){return 11;}
  if(angle > -171.0f && angle <= -153.0f){return 12;}
  if(angle > -153.0f && angle <= -135.0f){return 13;}
  if(angle > -135.0f && angle <= -117.0f){return 14;}
  if(angle > -117.0f && angle <= -99.0f){return 15;}
  if(angle > -99.0f && angle <= -81.0f){return 16;}
  if(angle > -81.0f && angle <= -63.0f){return 17;}
  if(angle > -63.0f && angle <= -45.0f){return 18;}
  if(angle > -45.0f && angle <= -27.0f){return 19;}
  if(angle > -27.0f && angle <= -9.0f){return 20;}
  return 0;
}

void setFace(int emotion, float intensity){
  for(int theDOF = 0; theDOF < 13; theDOF++){
    // blend between the selected emotion and the neutral face expression
    int pos = int( (float(Faces[emotion][theDOF]) * intensity) + (float(Faces[0][theDOF]) * (1.0f - intensity)) );
    DOFs[theDOF].SendPos(pos);
  }
  // Start the command
  Serial.print("T");
  Serial.print(UPDATE_TIMING);
  Serial.print("\r");
}
