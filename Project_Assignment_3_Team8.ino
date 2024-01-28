#include <AceRoutine.h>
#include <RF24.h>

using namespace ace_routine;


RF24 radio(7,8);  

const byte addr[6]="bcast";

unsigned char PAE[32] = {0x50, 0x08};


struct Node {
    int name;
    int duration;
};

class DiscoverNode {
  private:
    Node neighbors[200];
    int neighborCount = 0;
    
  public:
   void addNode(int NodeNumber) {
      Node n;
      n.duration = 0;
      n.name = NodeNumber;
      if (NodeNumber == 0){
        Serial.println("Team Number (node) with 0 not accepted");
        return;
        }
        
      for (int i = 0; i < 200; i++) {
        if (neighbors[i].name == NodeNumber) {
            Serial.print("-------Old node restored---");
            Serial.println(neighbors[i].name);
            neighbors[i].duration = 0;
            return;
        }
      }
  
     for (int i = 0; i < 200; i++) {
        if (neighbors[i].name == 0 and neighbors[i].duration==0) {
  
        neighbors[i]= n;
        Serial.print("-------New node added---");
        Serial.println(neighbors[i].name);
        neighborCount++;
            return;
        }
        
      }
   
    }
   
    void updateDuration() {
      for (int i = 0; i < 200; i++) {
          if (neighbors[i].name!=0) {
              neighbors[i].duration++;
          }
      }
    }

    void printNeighbors() {
        Serial.println("-----Current Neighbors-----");
        
        if(neighborCount == 0){
          Serial.println("No neighbors currently connected");
          return ;
        }
        int a[neighborCount];
        int count=0;
        for (int i = 0; i < 200; i++) {
          
            if(neighbors[i].name !=0){
              a[count]=neighbors[i].name;
              count++;
            }
            
        }
        Serial.print("Neighbor: ");
        for (int i = 0; i < neighborCount; i++){
              Serial.print(a[i]);
              Serial.print(",");
        }
        Serial.println(" connected.");
    }
    
    void removeNeighbors() {
      for (int i = 0; i < 200; i++) {
          if (neighbors[i].name !=0 && neighbors[i].duration >= 6) {
              neighbors[i].name = 0;
              neighbors[i].duration = 0;
              neighborCount--;
          }
      }
    }
   
};


class PrintingNeighbors: public Coroutine {
  private:
    DiscoverNode * discoverNode;
  public:
    PrintingNeighbors(DiscoverNode * dn) {
      discoverNode = dn;
    }

  int runCoroutine() override {
    COROUTINE_LOOP() {
      discoverNode->printNeighbors();
      discoverNode->updateDuration();
      COROUTINE_DELAY(1000);
       COROUTINE_YIELD();
    }
  }
};


class RemovingNeighbors : public Coroutine {
  private:
    DiscoverNode * discoverNode;
 
  public:
    RemovingNeighbors(DiscoverNode * dn) {
      discoverNode = dn;
    }

  int runCoroutine() override {
    COROUTINE_LOOP() {
      discoverNode->removeNeighbors();
      COROUTINE_DELAY(1000);
       COROUTINE_YIELD();
    }
  }
};


class TransmitReceive: public Coroutine {
  private:
    DiscoverNode * discoverNode;
  public:
    TransmitReceive(DiscoverNode * dn) {
      discoverNode = dn;
    }

  void send(){
    radio.openWritingPipe(addr);
    radio.stopListening();

    Serial.println("------Broadcasting myself (Team 8)-----");
    radio.write(PAE, sizeof(PAE));
    radio.startListening();
  }
 
  void receive(){
    radio.openReadingPipe(0, addr);
    radio.startListening();
   
    unsigned char buffer[32+1];
    buffer[32]=0;
 
    if (radio.available()) {
        radio.read(buffer, 32);
        int Type = buffer[0] >> 4;
        int NodeNumber = buffer[1];
        if(Type == 5){
          discoverNode->addNode(NodeNumber);
        }
    }
  }

  int runCoroutine() override {
    COROUTINE_LOOP() {
        
        long int randN = random(1000, 5000);
        unsigned long prevMillis = millis();
        send();
        while(millis()-prevMillis<=randN){
        receive();
        }
      COROUTINE_YIELD();
    }
  }
};


DiscoverNode X;
TransmitReceive TR(&X);
PrintingNeighbors printingN(&X);
RemovingNeighbors removeN(&X);


void setup()
{
  Serial.begin(9600);
  if (radio.begin()) {
    Serial.println(F("radio hardware is OK"));
  } else {
    Serial.println(F("radio hardware is not responding"));
  }

  radio.setPALevel(RF24_PA_MAX);
  radio.setAutoAck(false);
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
