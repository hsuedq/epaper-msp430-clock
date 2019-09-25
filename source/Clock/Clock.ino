

#include <SPI.h>
#include "epd2in13.h"
#include "SPI.h" 
#include "SPIFlash.h"


#define CSFLASH_PIN      8             // chip select pin 
#define MIN_LEVEL  1 //  1440
#define TMO_LEVEL   5
volatile uint8_t ready =0;

volatile unsigned int min_cntdown =0;
volatile uint8_t tmo_cntdown =0;

volatile unsigned int picCounterHour = 0;
volatile unsigned int picCounterMinute = 0;
volatile bool toggle1 = false;
volatile bool doLog = false;
volatile bool darkTheme = false;

Epd epd;
#define F_CPU  8000000

int analogIn;
void unusedpins()
{
  pinMode(P1_0, INPUT_PULLUP);
  pinMode(P1_1, INPUT_PULLUP);
  pinMode(P1_2, INPUT_PULLUP);
 
}

SPIFlash flash(CSFLASH_PIN); //, 0xC840);

void setup()
{
  
  pinMode(CSFLASH_PIN, OUTPUT);
digitalWrite(CSFLASH_PIN, HIGH);

  doLog=false;
  
  picCounterHour=22;
picCounterMinute=48;
ready=true;


   doLog=false;
 Serial.begin(57500);  
   pinMode(P1_3, INPUT_PULLUP);
   for(int i=0;i < 3;i++)
   {
    delay(500);
     Serial.print("\n Start \n\r");
     if(digitalRead(P1_3)==0)
     {
      Serial.print("\n Upload \n\r");
      delay(1000);
        uploadFlash();
     }
   }
 
 
 
 // unusedpins();
  
  delay(500);
       delay(500);
   
 
 

min_cntdown = 0;


   pinMode(PUSH2, INPUT_PULLUP);


 ready=true;
 
  

Serial.println("Sleep setup");


BCSCTL1 |= DIVA_3;              // ACLK/8
    BCSCTL3 |= XCAP_3;              //12.5pF cap- setting for 32768Hz crystal

   
   

    CCTL0 = CCIE;                   // CCR0 interrupt enabled
    CCR0 =  30720;           // 512 -> 1 sec, 30720 -> 1 min 10240; //
    TACTL = TASSEL_1 + ID_3 + MC_1;         // ACLK, /8, upmode

   delay(200);


   DisplayClock(lut_full_update,true);
}
  
/* Stop with dying message */         
void die ( int pff_err  )
{
   Serial.println();
   Serial.print("Failed with rc=");
   Serial.print(pff_err,DEC);
   //for (;;) ;
 epd.DisplayFrame();
   delay(300);
   epd.Sleep();
   SPI.end();
   digitalWrite(CSFLASH_PIN, HIGH);
    delay(200);
  //  digitalWrite(enable_sd, HIGH);
    delay(200);
    
   pinMode (CSFLASH_PIN, INPUT);
   pinMode(CS_PIN, INPUT);
    pinMode(RST_PIN, INPUT_PULLUP);
    pinMode(DC_PIN, INPUT_PULLUP);
    pinMode(BUSY_PIN, INPUT_PULLDOWN); 
  

      delay(500);
       delay(500);

   ready=true;
   Serial.println("Enter LPM3 w/ interrupt");
   _BIS_SR(LPM3_bits + GIE);           // Enter LPM3 w/ interrupt
   Serial.println("After LPM3 w/ interrupt");

  
}




/*-----------------------------------------------------------------------*/
/* Program Main                                                          */
/*-----------------------------------------------------------------------*/
void loop()
{
  if(doLog)
    Serial.print("-");
 
  if(ready == true)
  {
     DisplayClock(lut_partial_update,false);
  }

   if(doLog)
     Serial.println("L Enter LPM3 w/ interrupt");
  
   __bis_SR_register(LPM3_bits + GIE);           // Enter LPM3 w/ interrupt
 if(doLog)
   Serial.println("L After LPM3 w/ interrupt");
  
}


// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
 //  WDTCTL = WDTPW + WDTHOLD; 
    if(doLog) 
     Serial.println("T");


 picCounterMinute++;

 if(picCounterMinute > 59)
 {
  picCounterMinute=0;
  picCounterHour++;
 }
if(picCounterHour > 23)
 {
    picCounterHour=0;
 }

 if(ready ==false)
  {

    tmo_cntdown++;
    if(tmo_cntdown >= TMO_LEVEL)
    {
      if(doLog) 
       Serial.println("D");
       WDTCTL = 0xDEAD;
    }
  }else
  {
    tmo_cntdown = 0;
  }

  __bic_SR_register_on_exit(LPM3_bits); 

}

void DisplayClock(const unsigned char* lut,bool fullupdate)
{
  unsigned char tfilter=0;

  if(doLog)
   Serial.println("N Start");
  if(!ready)
  {

   if(doLog)
      Serial.println("Not Ready");
  //   digitalWrite(LED, LOW); 
    return;
  }
  ready=false;

 pinMode (CSFLASH_PIN, OUTPUT);
  digitalWrite(CSFLASH_PIN, HIGH);

    pinMode(CS_PIN, OUTPUT);
      digitalWrite(CS_PIN, HIGH);
    pinMode(RST_PIN, OUTPUT);
    pinMode(DC_PIN, OUTPUT);
    pinMode(BUSY_PIN, INPUT); 

   delay(100);


if(fullupdate)
{
   digitalWrite(RST_PIN, LOW);
    delay(20);
    digitalWrite(RST_PIN, HIGH);
    delay(20); 
}
     SPI.begin();
      Epd epd;  
 

 delay(20); 
 
   if (epd.Init(lut,fullupdate) != 0) {
    if(doLog)
    Serial.print("e-Paper init failed");
      die(0);
  
  }

 if(doLog)
   Serial.println("e-Paper init good");

  if (flash.initialize())
  {
    if(doLog)
    Serial.println("Flash Init OK!");
  
   
  toggle1 = !toggle1;
  if(toggle1)
      tfilter =0xAA;
    else
    tfilter =0x55;
   
   if(fullupdate)
   {
    if(darkTheme)
    {
      epd.ClearFrameMemory(0xff,0x26);
      epd.ClearFrameMemory(0x00,0x24);
    }else
    {
      epd.ClearFrameMemory(0x00,0x26);
      epd.ClearFrameMemory(0xff,0x24);
    }
   }
  
     if(fullupdate || picCounterMinute == 0 || picCounterMinute == 10 || picCounterMinute % 10 == 1)
     {
       if(darkTheme)
        {
            epd.ClearFrameMemory(0xff ,0x26);
            epd.ClearFrameMemory(0x00,0x24);
         
        }else
        {
            epd.ClearFrameMemory(0x00 ,0x26);
            epd.ClearFrameMemory(0xff,0x24);
           
        }
          epd.SetFrameMemory(flash,picCounterHour,0,0,128,125,0x26,0,!darkTheme );//!toggle1);
          epd.SetFrameMemory(flash,picCounterHour,0,0,128,125,0x24,0,darkTheme);//!toggle1);
          
     }
    epd.SetFrameMemory(flash,picCounterMinute+24,0,125,128,125,0x26,0,!darkTheme);//toggle1);
    epd.SetFrameMemory(flash,picCounterMinute+24,0,125,128,125,0x24,0,darkTheme);//toggle1);
      
    
    epd.DisplayFrame();
    if(!fullupdate)
    {
      delay(20);
      epd.DisplayFrame();
    }
 

    
    
  }
  else
  {
    if(doLog)
    Serial.println("Flash Init FAIL!");
     
  }

 
   if(doLog) 
    Serial.println("EPD Going toSleep");
   epd.Sleep();
   if(doLog) 
    Serial.println("DisplayFrame End");
  
  ready=true;

   if(doLog) 
    Serial.println("N End");
  return;
  

}




void SetTime()
{
  int setlevel = 0;
  bool dosettime =false;
  unsigned char tfilter1=0;
  int lowdigit=0;
  int highdigit=0;
unsigned int setuptimeout=0;
 
  dosettime=true;
  setlevel=1;

  
       pinMode(P1_2, INPUT_PULLUP);

        pinMode(P1_1, INPUT_PULLUP);

        

 pinMode (CSFLASH_PIN, OUTPUT);
  digitalWrite(CSFLASH_PIN, HIGH);

    pinMode(CS_PIN, OUTPUT);
      digitalWrite(CS_PIN, HIGH);
    pinMode(RST_PIN, OUTPUT);
    pinMode(DC_PIN, OUTPUT);
    pinMode(BUSY_PIN, INPUT); 

   delay(100);

   digitalWrite(RST_PIN, LOW);
    delay(20);
    digitalWrite(RST_PIN, HIGH);
    delay(20); 
     SPI.begin();
      Epd epd;  
 

 delay(20); 
 
   if (epd.Init(lut_full_update,true) != 0) {
    if(doLog)
    Serial.print("e-Paper init failed");
      die(0);

  }
 
  if (flash.initialize())
  {

    epd.ClearFrameMemory(0xff,0x26);
        epd.ClearFrameMemory(0x00,0x24);
    epd.DisplayFrame();
    epd.Init(lut_partial_update,false);

    picCounterMinute=0;
    if(doLog)
    Serial.println("Init OK!");
          DisplaySettings(epd,flash,setlevel,picCounterHour,true,true);

        
   while(dosettime)
  {
    setuptimeout++;
    if(setuptimeout > 10000)
    {
        dosettime=false;
          DisplayClock(lut_partial_update,false);
            return;
    }
        
     if(digitalRead(P1_1)==0)
         {
          setuptimeout=0;
            setlevel++;
              if(setlevel>2)
            {  
                DisplaySettings(epd,flash,setlevel,picCounterMinute,true,true);
            }else
            {
              DisplaySettings(epd,flash,setlevel,picCounterHour,true,true);
            }
             
            if(setlevel>6)
          {  
            setlevel=1;
              DisplaySettings(epd,flash,setlevel,picCounterHour,true,true);
          }
                
         }
         else
       if(digitalRead(P1_2)==0)
         {
          setuptimeout=0;
           if(setlevel==1)
              {
                
                 picCounterHour+=10;
                   if(picCounterHour > 20)
                  {
                      picCounterHour=0;
                      
                  }
                  highdigit = picCounterHour;
                    DisplaySettings(epd,flash,setlevel,picCounterHour,true,false); 
                   
                
                }
                  if(setlevel==2)
              {
                
                 picCounterHour+=1;
                   if(picCounterHour > highdigit+9  || picCounterHour > 23)
                  {
                      picCounterHour=highdigit;
                     
                  }
                    DisplaySettings(epd,flash,setlevel,picCounterHour,true,false); 
                   
                
                }
              if(setlevel==3)
              {
                
                 picCounterMinute+=10;
                   if(picCounterMinute > 59)
                  {
                      picCounterMinute=0;
                      
                  }
                  highdigit = picCounterMinute;
                    DisplaySettings(epd,flash,setlevel,picCounterMinute,true,false); 
                   
                
                }

                 if(setlevel==4)
              {
                
                 picCounterMinute+=1;
                   if(picCounterMinute > highdigit+9)
                  {
                      picCounterMinute=highdigit;
                     
                  }
                    DisplaySettings(epd,flash,setlevel,picCounterMinute,true,false); 
                   
                
                }
                   if(setlevel==5)
                  {  
         
                    darkTheme = !darkTheme;
                    DisplaySettings(epd,flash,setlevel,darkTheme,true,false); 
                }
                if(setlevel==6)
                  {  
         
                    DisplayClock(lut_partial_update,true);
                    return;
                }
         
            
    
         }else
         {
          delay(10);
         }
    }

  }
  else
  {
    if(doLog)
    Serial.println("Init FAIL!");
     
  }
  /* Deep sleep */
   delay(20);
   epd.Sleep();
 
}

void DisplaySettings(Epd epd, SPIFlash spiflush,int menuid,int value,bool left,bool init)
{
  if(init)
  {
  
           epd.SetFrameMemory(flash,menuid+59,0,4,128,150,0x26,0x0,0);
          epd.SetFrameMemory(flash,value,0,128,128,122,0x26,0x0,0);
      
         epd.SetFrameMemory(flash,menuid+59,0,4,128,150,0x24,0x0,1);
          epd.SetFrameMemory(flash,value,0,128,128,122,0x24,0x0,1);
          epd.DisplayFrame();
    
        
  }
  else
  {
    epd.SetFrameMemory(flash,value,0,128,128,122,0x26,0x0,0);
    epd.SetFrameMemory(flash,value,0,128,128,122,0x24,0x0,1);
  
          epd.DisplayFrame();
    if(left)
    {
        
    }
  }
}

void uploadFlash(){


  uint32_t addr1=0; //5808;
  char input = 0;

   pinMode (CSFLASH_PIN, OUTPUT);
  digitalWrite(CSFLASH_PIN, HIGH);


   delay(500);
     SPI.begin();
  if (flash.initialize())
  {
    Serial.println("Flash Init OK!");

    Serial.print("DeviceID: ");
    Serial.println(flash.readDeviceId(), HEX);
    
    Serial.print("Erasing Flash chip ... ");
      flash.chipErase();
     while(flash.busy());
     Serial.println("DONE");

        Serial.println("Flash content:");
      int counter = 0;
      addr1 =0;// 160000; // 5808;  
      while(counter < 4000){
        Serial.print(flash.readByte(addr1++), HEX);
        counter++;
       // Serial.print('.');
      }
      
      Serial.println();
      addr1=0; //48000; //280000;
      for(;;)
      {
        if (Serial.available() > 0) {
          input = Serial.read();
         
              
              flash.writeByte(addr1,(uint8_t) input);
              addr1++;
            //   Serial.print((uint8_t) flash.readByte(addr1), HEX);
              Serial.print('.');
        
           
           
        }else
        {
          Serial.print('*');
          delay(100);
        }
      }
  }else
  {
     Serial.println("Flash Init FAILED!");
  }

  
}
