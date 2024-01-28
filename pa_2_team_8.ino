//Authors: 
//MD. Asif Talukdar
//Emam Hossain
//Prasanta Bhattacharjee

#include <ArduinoJson.h>
#include <AceRoutine.h>
using namespace ace_routine;


// produces a tick or a tock to Serial Monitor every second
class Watch {
  private:
  int _counter;
 	int _tick;
  int _blinkDelay;
  int _delay;

  public:
  Watch() {
    _counter=0;
    _tick = 0;
    _delay = 1300;
    _blinkDelay = 1000;
  }
  
  // ticking action
  void tick();
  void blinking();

  void setDelay(int del) {
    _delay = del;
  }

  int getDelay() { 
    return _delay;
  }

    void setblinkDelay(int del) {
    _blinkDelay = del;
  }

  int getblinkDelay() { 
    return _blinkDelay;
  }
};

void Watch::tick() {
    Serial.println(_tick==0?"tic":"toc");
    _tick = (_tick+1)%2;
}	


void Watch::blinking() {
     digitalWrite(13, _counter==0?HIGH:LOW);
     Serial.println(_counter);
     _counter = _counter==0?1:0;
}	

class Reader : public Coroutine {
  private:
    Watch * watch;
  
  public:
  Reader(Watch * w) {
    watch = w;
  }

  int runCoroutine() override {
    StaticJsonDocument<200> doc;
    String json_str;

    COROUTINE_LOOP() {
      if (Serial.available()) {
        json_str = Serial.readString();
        Serial.println(json_str.c_str());
        DeserializationError err = deserializeJson(doc, json_str.c_str());
        if (! err) {
          if(doc["delayTIC"]){
             watch->setDelay(doc["delayTIC"]);
             Serial.println(watch->getDelay());
          }
        //  else{
        //    watch->setDelay(1300);
        //    Serial.println(watch->getDelay());
        //  }
          if(doc["delayLED"]){
             watch->setblinkDelay(doc["delayLED"]);
             Serial.println(watch->getblinkDelay());
          }
        //  else{
        //    watch->setblinkDelay(1000);
        //    Serial.println(watch->getblinkDelay());
        //  }
          
        }
        else {
          Serial.println("Error");
          Serial.println(err.f_str());
        }
      }
      COROUTINE_YIELD();    
    }
  }
};


class blinker: public Coroutine {
  private:
    Watch * watch;
  public:
    blinker(Watch * w) {
      watch = w;
    }

  int runCoroutine() override {
    COROUTINE_LOOP() {
      watch->blinking();
      COROUTINE_DELAY(watch->getblinkDelay());
    }
  }
};


class Ticker: public Coroutine {
  private:
    Watch * watch;
  public:
    Ticker(Watch * w) {
      watch = w;
    }

  int runCoroutine() override {
    COROUTINE_LOOP() {
      watch->tick();
      COROUTINE_DELAY(watch->getDelay());
    }
  }
};

// ******** objects ********
Watch the_watch;
Reader r(&the_watch);
blinker b(&the_watch);
Ticker t(&the_watch);


void setup() {
 
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  CoroutineScheduler::setup();

}

void loop() {
 
  CoroutineScheduler::loop();
}
