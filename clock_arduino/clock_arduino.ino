#include <Adafruit_NeoPixel.h>
#define WIDTH 21
#define HIGHT 16
#define DIGIT_WIDTH 3
#define DIGIT_HIGHT 5
#define WEATHER_WIDTH 5
#define WEATHER_HIGHT 5
#define PIN 5
char cString[50];
byte chPos = 0;
byte ch = 0;
char dataStr[6];
int curMode = 0;
int baseHour = 0;
int baseMin = 0;
int baseSec = 0;
long baseMillis = 0;
int curTemp = 0;
int curWeather = 0;
String curStrWeather = "clear";
int curWind = 0;
int curHum = 0;
int curR = 0;
int curG = 0;
int curB = 0;
int curBr = 0;
// 51
// 21 х 16
// 51
int len = 336;
// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(len, PIN, NEO_GRB + NEO_KHZ800);
int matrix[HIGHT][WIDTH];
//clear
//clowdy
//rain
//storm
//snow
//fog



int weather_clear[WEATHER_HIGHT][WEATHER_WIDTH]=
{
  {1,1,1,0,0},
  {1,0,1,0,0},
  {1,1,1,0,0},
  {0,1,1,0,0},
  {1,0,1,0,0}
};
int weather_clowdy[WEATHER_HIGHT][WEATHER_WIDTH]=
{
  {1,1,1,0,0},
  {1,0,1,0,0},
  {1,0,1,0,0},
  {1,0,1,0,0},
  {1,1,1,0,0}
};
int weather_rain[WEATHER_HIGHT][WEATHER_WIDTH]=
{
  {0,1,1,1,0},
  {0,1,0,1,0},
  {0,1,0,1,0},
  {1,1,1,1,1},
  {1,0,0,0,1}
};
int weather_storm[WEATHER_HIGHT][WEATHER_WIDTH]=
{
  {1,1,1,0,0},
  {1,0,0,0,0},
  {1,0,0,0,0},
  {1,0,0,0,0},
  {1,0,0,0,0}
};
int weather_snow[WEATHER_HIGHT][WEATHER_WIDTH]=
{
  {1,1,1,0,0},
  {1,0,0,0,0},
  {1,0,0,0,0},
  {1,0,0,0,0},
  {1,1,1,0,0}
};
int weather_fog[WEATHER_HIGHT][WEATHER_WIDTH]=
{
  {1,1,1,0,0},
  {0,1,0,0,0},
  {0,1,0,0,0},
  {0,1,0,0,0},
  {0,1,0,0,0}
};

int digits[10][DIGIT_HIGHT][DIGIT_WIDTH] ={ 
  {
  {1,1,1},
  {1,0,1},
  {1,0,1},
  {1,0,1},
  {1,1,1}
  },{
  {0,1,0},
  {1,1,0},
  {0,1,0},
  {0,1,0},
  {1,1,1}
  },{
  {1,1,1},
  {0,0,1},
  {0,1,0},
  {1,0,0},
  {1,1,1}
  },{
  {1,1,1},
  {0,0,1},
  {1,1,1},
  {0,0,1},
  {1,1,1}
  },{
  {1,0,1},
  {1,0,1},
  {1,1,1},
  {0,0,1},
  {0,0,1}
  },{
  {1,1,1},
  {1,0,0},
  {1,1,1},
  {0,0,1},
  {1,1,1}
  },{
  {1,1,1},
  {1,0,0},
  {1,1,1},
  {1,0,1},
  {1,1,1}
  },{
  {1,1,1},
  {0,0,1},
  {0,1,0},
  {0,1,0},
  {0,1,0}
  },{
  {1,1,1},
  {1,0,1},
  {1,1,1},
  {1,0,1},
  {1,1,1}
  },{
  {1,1,1},
  {1,0,1},
  {1,1,1},
  {0,0,1},
  {1,1,1}
  }
  };
void setup()
{
  pixels.begin(); // This initializes the NeoPixel library.
  Serial.begin(115200);
  Serial1.begin(1000); 
  pinMode(13, OUTPUT);
}
int curSec()
{
 //  Serial.println((int)(((millis() - baseMillis)*2.6)/1000));
  int shift_in_sec = (int)(((millis() - baseMillis)*2.6)/1000);
  return (baseSec + shift_in_sec)%60;
}
int curMin()
{
  int shift_in_sec = (int)(((millis() - baseMillis)*2.6)/1000);
  int shift_in_mins = (int)((shift_in_sec+baseSec)/60);
  return (baseMin + shift_in_mins)%60;
}
int curHour()
{
  int shift_in_sec = (int)(((millis() - baseMillis)*2.6)/1000);
  int shift_in_mins = (int)((shift_in_sec+baseSec)/60);
  int shift_in_hours = (int)((shift_in_mins+baseMin)/60);
  return (baseHour + shift_in_hours)%24;
}
void convert_weather()
{
  if (curWeather == 1)
  {
    curStrWeather = "clear";
  }
  if (curWeather == 2)
  {
    curStrWeather = "clowdy";
  }
  if (curWeather == 3)
  {
    curStrWeather = "clowdy";
  }
  if (curWeather == 4)
  {
    curStrWeather = "clowdy";
  }
  if (curWeather == 9)
  {
    curStrWeather = "rain";
  }
  if (curWeather == 10)
  {
    curStrWeather = "rain";
  }
  if (curWeather == 11)
  {
    curStrWeather = "storm";
  }
  if (curWeather == 13)
  {
    curStrWeather = "snow";
  }
  if (curWeather == 50)
  {
    curStrWeather = "fog";
  }
  
}
void check_serial1()
{
  //2;19;17;01;+;16;04 ;02;12;000;000;255;100
  //01234567890123456789012345678901234567890
  //00000000001111111111222222222233333333334
  //mode;hour;min;sec;sign;temp;icon;wind;hum;r;g;b;bright

  if (Serial1.available())
  {
    long start_time = millis();
    //while(Serial1.available())
    for (int i = 0; i< 40;i++)
    {
    //read incoming char by char:
      ch = Serial1.read();
      cString[chPos] = ch;
      chPos++;     
      digitalWrite(13, HIGH); //flash led to show data is arriving
      delay(10);
      digitalWrite(13, LOW);
   }
   cString[chPos] = 0; //terminate cString
   chPos = 0;
   // Serial.print("t = ");
   // Serial.print(millis()-start_time);
   // Serial.print("::");
   Serial.println(cString);
   curMode = cti(cString[0]);
   baseHour = cti(cString[2])*10+cti(cString[3]);
   baseMin = cti(cString[5])*10+cti(cString[6]);
   baseSec = cti(cString[8])*10+cti(cString[9]);
   curTemp =cti(cString[13])*10+cti(cString[14]);
   baseMillis = millis();
   // int a = '1'-48;
   // Serial.println(curMode);
   // Serial.println(baseHour);
   // Serial.println(baseMin);
   // Serial.println(baseSec);
   // Serial.println(curTemp);
   if (cString[11]=='1')
   {
      Serial.println("HOLODHO");
      curTemp = -curTemp;
   }
   Serial.print("curTemp");
   Serial.println(curTemp);
   curWeather = cti(cString[16])*10+cti(cString[17]);
   curWind =cti(cString[19])*10+cti(cString[20]);
   curHum = cti(cString[22])*10+cti(cString[23]);
   convert_weather();
   Serial.println(curStrWeather);
   curR = cti(cString[25])*100 + cti(cString[26])*10+cti(cString[27]);
   curG = cti(cString[29])*100 + cti(cString[30])*10+cti(cString[31]);
   curB = cti(cString[33])*100 + cti(cString[34])*10+cti(cString[35]);
   curBr = cti(cString[37])*100 + cti(cString[38])*10+cti(cString[39]);
  }
}
int cti(char ch)
{
  return (int)ch - 48;
}
void zero_matrix()
{
  for (int i = 0; i < HIGHT; i++)
  {
    for (int j = 0; j < WIDTH; j++)
    {
      matrix[i][j] = 0;
    }
  }
}
void fill_time(int hours,int mins,int secs)
{
  int shift = 0;
  if (hours>9)
  {
    shift = 0;
    int first_num = hours/10;
    for (int i = 0; i < DIGIT_HIGHT; i++)
        {
          for (int j = 0; j < DIGIT_WIDTH; j++)
          {
            matrix[i][j] = digits[first_num][i][j];
          }
        }
  }
  else
  {
    shift = 0;
    int first_num = 0;
    for (int i = 0; i < DIGIT_HIGHT; i++)
        {
          for (int j = 0; j < DIGIT_WIDTH; j++)
          {
            matrix[i][j] = digits[first_num][i][j];
          }
        }
  }
  int second_num = hours%10;
  shift = 3;
  for (int i = 0; i < DIGIT_HIGHT; i++)
  {
    for (int j = 0; j < DIGIT_WIDTH; j++)
    {
      matrix[i][j+shift] = digits[second_num][i][j];
    }
  }





  if (mins>9)
  {
    shift = 7;
    int first_num = mins/10;
    for (int i = 0; i < DIGIT_HIGHT; i++)
        {
          for (int j = 0; j < DIGIT_WIDTH; j++)
          {
            matrix[i][j+shift] = digits[first_num][i][j];
          }
        }
  }
  else
  {
    shift = 7;
    int first_num = 0;
    for (int i = 0; i < DIGIT_HIGHT; i++)
        {
          for (int j = 0; j < DIGIT_WIDTH; j++)
          {
            matrix[i][j+shift] = digits[first_num][i][j];
          }
        }
  }
  second_num = mins%10;
  shift = 10;
  for (int i = 0; i < DIGIT_HIGHT; i++)
  {
    for (int j = 0; j < DIGIT_WIDTH; j++)
    {
      matrix[i][j+shift] = digits[second_num][i][j];
    }
  }






   if (secs>9)
  {
    shift = 14;
    int first_num = secs/10;
    for (int i = 0; i < DIGIT_HIGHT; i++)
        {
          for (int j = 0; j < DIGIT_WIDTH; j++)
          {
            matrix[i][j+shift] = digits[first_num][i][j];
          }
        }
  }
  else
  {
    shift = 14;
    int first_num = 0;
    for (int i = 0; i < DIGIT_HIGHT; i++)
        {
          for (int j = 0; j < DIGIT_WIDTH; j++)
          {
            matrix[i][j+shift] = digits[first_num][i][j];
          }
        }
  }
  second_num = secs%10;
  shift = 17;
  for (int i = 0; i < DIGIT_HIGHT; i++)
  {
    for (int j = 0; j < DIGIT_WIDTH; j++)
    {
      matrix[i][j+shift] = digits[second_num][i][j];
    }
  }
     
}

void fill_temp(int temp)
{
  if (temp < 0)
  {
    temp = temp*-1;
    matrix[9][0] = 1;
    matrix[9][1] = 1;
  }

 int shift = 0;
 int shift_y = 7;
  if (temp>9)
  {
    shift = 2;
    int first_num = temp/10;
    for (int i = 0; i < DIGIT_HIGHT; i++)
        {
          for (int j = 0; j < DIGIT_WIDTH; j++)
          {
            matrix[i+shift_y][j+shift] = digits[first_num][i][j];
          }
        }
  }
  else
  {
    shift = 2;
    int first_num = 0;
    for (int i = 0; i < DIGIT_HIGHT; i++)
        {
          for (int j = 0; j < DIGIT_WIDTH; j++)
          {
            matrix[i+shift_y][j+shift] = digits[first_num][i][j];
          }
        }
  }
  int second_num = temp%10;
  shift = 5;
  for (int i = 0; i < DIGIT_HIGHT; i++)
  {
    for (int j = 0; j < DIGIT_WIDTH; j++)
    {
      matrix[i+shift_y][j+shift] = digits[second_num][i][j];
    }
  }
  
}

void fill_weather(String weather)
{
  int shift = 12;
  int shift_y = 7;
//clear
//clowdy
//rain
//storm
//snow
//fog
  if (weather == "clear")
  {
    for (int i = 0; i < WEATHER_HIGHT; i++)
    {
      for (int j = 0; j < WEATHER_WIDTH; j++)
      {
        matrix[i+shift_y][j+shift] = weather_clear[i][j];
      }
    }
  }
  if (weather == "clowdy")
  {
    for (int i = 0; i < WEATHER_HIGHT; i++)
    {
      for (int j = 0; j < WEATHER_WIDTH; j++)
      {
        matrix[i+shift_y][j+shift] = weather_clowdy[i][j];
      }
    }
  }
  if (weather == "rain")
  {
    for (int i = 0; i < WEATHER_HIGHT; i++)
    {
      for (int j = 0; j < WEATHER_WIDTH; j++)
      {
        matrix[i+shift_y][j+shift] = weather_rain[i][j];
      }
    }
  }
  if (weather == "storm")
  {
    for (int i = 0; i < WEATHER_HIGHT; i++)
    {
      for (int j = 0; j < WEATHER_WIDTH; j++)
      {
        matrix[i+shift_y][j+shift] = weather_storm[i][j];
      }
    }
  }
  if (weather == "snow")
  {
    for (int i = 0; i < WEATHER_HIGHT; i++)
    {
      for (int j = 0; j < WEATHER_WIDTH; j++)
      {
        matrix[i+shift_y][j+shift] = weather_snow[i][j];
      }
    }
  }
  if (weather == "fog")
  {
    for (int i = 0; i < WEATHER_HIGHT; i++)
    {
      for (int j = 0; j < WEATHER_WIDTH; j++)
      {
        matrix[i+shift_y][j+shift] = weather_fog[i][j];
      }
    }
  }
}
void fill_wind(int wind)
{
  for (int i = 0; i < wind; i++)
  {
     matrix[14][i] = 1;
  }
}
void fill_hum(int hum)
{
  for (int i = 0; i < hum; i++)
  {
     matrix[15][i] = 1;
  }
}
void show_matrix()
{
  int r = 0;
  int g = 0;
  int b = 0;
  // Serial.println(curMode);
  if (curMode == 0)
  {
    r = 0;
    g = 0;
    b = 0;
  }
  if (curMode == 1)
  {
    r = curR * (curBr/100.0);
    g = curG * (curBr/100.0);
    b = curB * (curBr/100.0);
  }
  if (curMode == 2)
  {
    r = curR * (curBr/100.0);
    g = curG * (curBr/100.0);
    b = curB * (curBr/100.0);
    for (int i = 0; i < HIGHT; i++)
    {
      for (int j = 0; j < WIDTH; j++)
      {
        pixels.setPixelColor(i*WIDTH+j, pixels.Color(r, g, b));
      }
    }
    pixels.show();
    return;
  }
  for (int i = 0; i < HIGHT; i++)
  {
    for (int j = 0; j < WIDTH; j++)
    {
      pixels.setPixelColor(i*WIDTH+j, pixels.Color(0, 0, 0));
    }
  }
  for (int i = 0; i < HIGHT; i++)
  {
    for (int j = 0; j < WIDTH; j++)
    {
      if (matrix[i][j]!=0)
      {
        if (i%2 == 0)
        {
          pixels.setPixelColor(i*WIDTH+WIDTH-j-1, pixels.Color(r, g, b));
        }
        else
        {
          pixels.setPixelColor(i*WIDTH+j, pixels.Color(r, g, b));
        }

      }
     
    }
  }
  pixels.show();
}



void loop()
{
  check_serial1();
  zero_matrix();
  fill_time(curHour(),curMin(),curSec());
  fill_temp(curTemp);
  fill_weather(curStrWeather);
  fill_wind(curWind);
  fill_hum(curHum);
  show_matrix();
}
