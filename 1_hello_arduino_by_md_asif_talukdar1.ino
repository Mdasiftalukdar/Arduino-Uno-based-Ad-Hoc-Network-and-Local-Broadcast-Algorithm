int delaytime = 1000;
// produces a tick or a tock to Serial Monitor every second
class Watch {
  private:
 	int _tick;
  public:
  Watch() {
    _tick = 0;   
  }
  
  void tick();
};

/*1st task, feature as variable named delaytime added for delay
2nd task, an LED with resistor added 
3rd task, BlinkingLED class created with toogle function which 
takes _tick variable value and toggles the LED
4th task, LED lits with the rythm of tick-tock and takes
Serial Monitor input for the duration of delay
*/

int Asif = 13;

class BlinkingLED{
public:
void toggle(int x) {
  if (x==0){
  digitalWrite(Asif, HIGH);  
  }
  else{
  digitalWrite(Asif, LOW); 
  }
}
};

BlinkingLED wow;

void Watch::tick() {
  Serial.println(_tick==0?"tic":"toc");
  if(_tick==0){
  wow.toggle(0);
  }
  if(_tick==1){
  wow.toggle(1);
  }
  _tick = (_tick+1)%2;
  delay(delaytime);
}	


void setup()
{
  Serial.begin(9600);
}


Watch citizen;

// Demonstration of loop and running functions and taking input
void loop() {
  citizen.tick();
 
  if (Serial.available()) {
    int value = Serial.parseInt();
  	Serial.println(value);
    delaytime = value;
  }
}