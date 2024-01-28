//Team_08[Md. Asif Talukdar, Prasanta Bhattacharjee, Emam Hossain]
#include <AceRoutine.h>
#include <RF24.h>

using namespace ace_routine;

RF24 radio(7,8);  

const byte addr[6]="bcast";

long int IntTime = millis();

uint8_t team = 8;

unsigned char PAE[32] = {0x50, 0x08};
uint8_t N[24] = {0};

uint8_t TwoHop1[24] = {0};
uint8_t TwoHop2[24] = {0};
uint8_t TwoHop3[24] = {0};

uint8_t Source[100] = {0};
uint8_t SeqMsb[100] = {0};
uint8_t SeqLsb[100] = {0};
int BroadcastReceiveCount = 0;
int BroadcastSentCount = 0;
uint8_t Mymsg = 0;
uint8_t MySeqMsb = 0;
uint8_t MySeqLsb = 0;

class DiscoverNode {
  private:
    int neighborCount = 0;
    
  public:
   void addNode(const char* buffer) {
      int NodeNumber = buffer[1];
   
      int TwoHopNumbers = buffer[2];
      if (NodeNumber == 0 || NodeNumber>24){
        Serial.println("Team Number (node) with 0 or greater than 24 not accepted");
        return;
        }

      int temp = (N[NodeNumber-1] >> 0) & 1; //0 is the index

      if (temp == 1){
        Serial.print("-------Old node restored---");
        Serial.println(NodeNumber);
        N[NodeNumber-1] &= ~(1 << 1);//duration to 0
        N[NodeNumber-1] &= ~(1 << 2);//duration to 0
        }
       else if (temp == 0){
        N[NodeNumber-1] |= (1 << 0); //value changed to 1 for this one hop neighbor
        Serial.print("-------New node added---");
        Serial.println(NodeNumber);
        neighborCount++;
       }

       for (int j=0; j<24; j++){   //clearing the 2 hop neighbors with 0 value
          if(j<8){
            TwoHop1[NodeNumber-1] &= ~(1 << j);//value changed to 0
            }
           else if(j<16){
            TwoHop2[NodeNumber-1] &= ~(1 << j-8);//value changed to 0
            }
           else if(j<24){
            TwoHop3[NodeNumber-1] &= ~(1 << j-16);//value changed to 0
            }
          }

       for (int j=0; j<TwoHopNumbers; j++){//imputing 2 hop neighbor values
          int twoHopNode = buffer[j+3];
          if(twoHopNode<=8){
            TwoHop1[NodeNumber-1] |= (1 << twoHopNode-1);//value changed to 1
            }
           else if(twoHopNode<=16){
            TwoHop2[NodeNumber-1] |= (1 << twoHopNode-9);//value changed to 1
            }
           else if(twoHopNode<=24){
            TwoHop3[NodeNumber-1] |= (1 << twoHopNode-17);//value changed to 1
            }
          }    
   
    }

    void SBA(const char* buffer) {
      uint8_t Src = buffer[1];
      uint8_t msb = buffer[2];
      uint8_t lsb = buffer[3];
   
      
      for (int i = 0; i<100; i ++){   
        if ( (Src == Source[i]) && (msb == SeqMsb[i]) && (lsb == SeqLsb[i]) ){ 
        return ;
        } 
      }
     
     Source[BroadcastReceiveCount] = Src;
     SeqMsb[BroadcastReceiveCount] = msb;
     SeqLsb[BroadcastReceiveCount] = lsb;
     BroadcastReceiveCount++;
     BroadcastReceiveCount=BroadcastReceiveCount%100;
     
     

     uint8_t SenderAdd = buffer[4];
     int neighborArray[24] = {0};
     int countN = 0;

     Serial.print("Unique message from Node");
     
     Serial.println(SenderAdd);

    //Serial.print("What my neigbor have");
      for (int j=0; j<24; j++){   //updating neighbors of Sender
          if(j<8){
            if ( ((TwoHop1[SenderAdd-1] >> j) & 1)==1 ){
            neighborArray[countN]=j+1;
          // Serial.print(j+1);
            countN++;
              }//checking whether value is 1 to detect neighbor
      
            }
           else if(j<16){
             if ( ((TwoHop2[SenderAdd-1] >> j-8) & 1)==1 ){
            neighborArray[countN]=j+1;
      // Serial.print(j+1);
            countN++;
              }//checking whether value is 1 to detect neighbor

            }
           else if(j<24){
             if ( ((TwoHop3[SenderAdd-1] >> j-16) & 1)==1 ){
            neighborArray[countN]=j+1;
        //Serial.print(j+1);
            countN++;
              }//checking whether value is 1 to detect neighbor
              
            }
          }
     neighborArray[countN]=SenderAdd;
     //Serial.print(SenderAdd);
     countN++;

     int  myArray[24] = {0};
     int  Mycount = 0;
     //Serial.print("     What I have");
     for (int i = 0; i < 24; i++) {
       if( ((N[i] >> 0) & 1) == 1 || i == team-1 ){
         myArray[Mycount] = i+1;
         //Serial.print(i+1);
         Mycount++;
       }
      } 
      //Serial.println();
      
      bool allInNeighbor = true;
    for (int i = 0; i < Mycount; i++) {
        bool found = false;
        for (int j = 0; j < countN; j++) {
            if (myArray[i] == neighborArray[j]) {
                found = true;
                //Serial.print(myArray[i]);
                //Serial.print("  found");
                break;
            }
        }
        if (!found) {
            allInNeighbor = false;
            break;
        }
    }
    //Serial.println(allInNeighbor);        
      
    if(allInNeighbor == false){

    long int randN = random(0, neighborCount*3);
    unsigned long prevMillis = millis();
    while(millis()-prevMillis<=randN){
        //waiting for M = tdv time t = 3, dv = 1st hop neighbor count fo mine
        }
      
    BroadcastSentCount++;
    radio.openWritingPipe(addr);
    radio.stopListening();
   
    Serial.print("------Rebroadcasting the received packet got from ");
    Serial.println(SenderAdd);

    char bufferCopy[sizeof(buffer)];
    memcpy(bufferCopy, buffer, sizeof(buffer));
    bufferCopy[4] = 0x08;  //need to update the sender address as my address
    radio.write(bufferCopy, sizeof(bufferCopy));
    radio.startListening();
      }
      
    }
    
    void updateDuration() {
      
      for (int i = 0; i < 24; i++) {
       
      int temp3 = (N[i] >> 0) & 1; //0 is the index
      
      if (temp3 == 1){
       
        int x = (N[i] >> 1) & 1;
        int y = (N[i] >> 2) & 1;
        int z = (N[i] >> 3) & 1;
        
        if (x == 0 && y == 0 && z == 0) {
                N[i] |= (1 << 3);  // Set bit 3 to 1
            } else if (x == 0 && y == 0 && z == 1) {
                N[i] |= (1 << 2);  // Set bit 2 to 1
                N[i] &= ~(1 << 3); // Clear bit 3
            } else if (x == 0 && y == 1 && z == 0) {
                N[i] |= (1 << 3);  // Set bit 3 to 1
            } else if (x == 0 && y == 1 && z == 1) {
                N[i] |= (1 << 1);  // Set bit 1 to 1
                N[i] &= ~(1 << 2); // Clear bit 2
                N[i] &= ~(1 << 3); // Clear bit 3
            } else if (x == 1 && y == 0 && z == 0) {
                N[i] |= (1 << 3);  // Set bit 3 to 1
            } else if (x == 1 && y == 0 && z == 1) {
                N[i] |= (1 << 1);  // Set bit 1 to 1
                N[i] |= (1 << 2);  // Set bit 2 to 1
                N[i] &= ~(1 << 3); // Clear bit 3
            }
      }
      }
      
    }

    void AddNeighborInPacket() {

      int a[neighborCount];
        int count=0;
        for (int i = 0; i < 24; i++) {
            if( ((N[i] >> 0) & 1) == 1 ){
              a[count]=i+1;
              count++;
            }    
        }
 
       
      PAE[2] = count;

      for (int i = 0; i < count; i++) {
          PAE[3+i] = a[i];
      }
     

    }
   
    void printNeighbors() {
        Serial.println("-----Current Neighbors-----");
       
        if(neighborCount == 0){
          Serial.println("No neighbors currently connected");
          return ;
        }
         
        Serial.print("Neighbor: ");
        for (int i = 0; i < 24; i++) {
            if( ((N[i] >> 0) & 1) == 1 ){//just checking the value of 0th bit postision is 1 or not
              Serial.print(i+1);
              Serial.print(",");
            }  
        }
        Serial.println(" connected.");
   
      Serial.print("Two Hop Neighbor: ");
        for (int j = 0; j < 24; j++) {//intuition is, for a bit(j, 1 to 24) value position check the whole array from 1 to 24(i)
          for (int i = 0; i < 24; i++) {
             if(j<8){
              if( ((TwoHop1[i] >> j) & 1) == 1 ){
             Serial.print(j+1);
             Serial.print(",");
              break;
              }
            }
           else if(j<16){
            if( ((TwoHop2[i] >> j-8) & 1) == 1 ){
             Serial.print(j+1);
             Serial.print(",");
              break;
              }
            }
           else if(j<24){
            if( ((TwoHop3[i] >> j-16) & 1) == 1 ){
             Serial.print(j+1);
             Serial.print(",");
              break;
              }
            }
            }
          }
        Serial.println(" connected.");
       
    }


  void printBroadcasts() {
    unsigned char M[32]={0x30, 0x08};
    M[2] = MySeqMsb;
    M[3] = MySeqLsb;
    M[4] = 0x08;
    M[5] = 1;
    M[6] = Mymsg;

    MySeqMsb = MySeqMsb+1%256;
    MySeqLsb = MySeqLsb+1%256;
    Mymsg = Mymsg+1%256;
  
    radio.openWritingPipe(addr);
    radio.stopListening();

    String message = "------broadcasting my message with sequence msb: " + String(MySeqMsb) + ", lsb: " + String(MySeqLsb)+ ", Mymsg: " + String(Mymsg);
    Serial.println(message);

    radio.write(M, sizeof(M));
    radio.startListening();

    Serial.print("*************Total Number of unique broadcast request received : ");
    Serial.println(BroadcastReceiveCount);
    Serial.print("*************Total Number of eligle message rebroadcasted : ");
    Serial.println(BroadcastSentCount);
    }

    void removeNeighbors() {
      for (int i = 0; i < 24; i++) {
        int x = (N[i] >> 1) & 1;
        int y = (N[i] >> 2) & 1;
          if (x == 1 and y ==1) {
              N[i] = {0};
              TwoHop1[i] = {0};
              TwoHop2[i] = {0};
              TwoHop3[i] = {0};


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

class PrintingBroadcasts: public Coroutine {
  private:
    DiscoverNode * discoverNode;
  public:
    PrintingBroadcasts(DiscoverNode * dn) {
      discoverNode = dn;
    }

  int runCoroutine() override {
    COROUTINE_LOOP() {
     
      if(millis()-IntTime>=10000){  
      discoverNode->printBroadcasts();  
      COROUTINE_DELAY(10000);
       
      }
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
   
    Serial.println("------Broadcasting my hello packet (Team 8)-----");
    discoverNode->AddNeighborInPacket();
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
        if(Type == 5){
          discoverNode->addNode(buffer);
        }
        else if (Type == 3 and (millis()-IntTime>=10000) ){
           discoverNode->SBA(buffer);
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
PrintingBroadcasts printingB(&X);

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
