//INCLUDE REQUIRED LIBRARY FILES
#include <SPI.h>
#include <U8g2lib.h>

//DEFINE MACROS
#define SPI_CLK 14
#define DELAY 5
#define RADIUS 5
#define SCREEN_X 128
#define SCREEN_Y 64
#define MAX_CIRCLES 20
#define MAX_TIME 5000
#define SAMPLE_FREQ 2000

//DEFINE CONSTANT VALUES 
const int BIT_DEPTH = 16;
const int AUDIO_IN = A3;
const unsigned long TIME_BETWEEN_SAMPLES = 1000000/SAMPLE_FREQ;
const unsigned long MIN_INTERVAL_TIME = 200;
const int OFFSET = 5000;
const int THRESHOLD = 30000;
const unsigned long MIN_TIME_INPUT = 500;
const unsigned long MIN_FRAME_RATE = 10;
const int MIN_CLEARANCE = 20;
const int MIN_PIPE_RATE = 500;
const int MIN_RESET_TIME = 3000;
int pipeIndex = 0;

//CREATE A TIMER FOR A SPECIFIC TASK
elapsedMicros time_since_sample= 0;
elapsedMillis time_since_pulse = 0;
elapsedMillis time_since_input = 0;
elapsedMillis frame_rate = 0;
elapsedMillis pipe_rate = 0;
elapsedMillis reset_timer = 0;

//INSTANTIATE oled OBJECT
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI oled(U8G2_R2, 10, 15, 16);

//VARIABLES TO STORE AUDIO INPUT, PULSE AND STATE DETAILS
int value = 0;
int in_pulse = 0;
bool state = false;

//FUNCTION PROTOTYPES
void record_audio();
int pulse_detector(bool ls, int in_pulse);
bool stateChange(int pulse_in, bool state);


//DEFINITION OF THE CLASS PIPE
class Pipe{
  
  private:
    unsigned int top;
    unsigned int bottom;
    int x;
    int vx;
    int w;
    unsigned int clearance;

  public:
    Pipe();
    void updatePipe();
    int getTopX();
    int getTopY();
    int getBottomX();
    int getBottomY();
    int getWidth();
    bool screenOut();
    void pipeReset();
    void pipeStop();
    void showPipe();
    
};


//PIPE CONSTRUCTOR
Pipe::Pipe()
{
  //CALL pipeReset() DURING OBJECT INSTANTIATION
  pipeReset();
}


//CREATE THE pipeReset() FUNCTION
//THIS FUNCTION RESETS THE POSITION OF THE PIPE
void Pipe::pipeReset()
{
  unsigned int half_y = SCREEN_Y / 2;
  
  top = abs(random(0, half_y));  
  bottom = abs(random(0, half_y));
  
  x = SCREEN_X + 20;
  w = 5;
  clearance = SCREEN_Y - top - bottom;

  if(clearance > MIN_CLEARANCE)
  {
    if ( top >= (clearance - MIN_CLEARANCE) )
      top = top + clearance - MIN_CLEARANCE;  
    else if ( top < (clearance - MIN_CLEARANCE) )
      bottom = bottom + clearance - MIN_CLEARANCE;
  }
  else if(clearance < MIN_CLEARANCE)
  {
    if ( top >= (clearance + MIN_CLEARANCE) )
      top = top - MIN_CLEARANCE - clearance;
    else if( top < (clearance + MIN_CLEARANCE) )
      bottom = bottom - MIN_CLEARANCE - clearance;
  }  
}


//DEFINE THE updatePipe() FUNCTION
//THIS FUNCTION UPDATES THE POSITION OF THE PIPE
void Pipe::updatePipe()
{
  //Serial.print("Top: ");
  //Serial.println(top);
  //Serial.print("Bottom: ");
  //Serial.println(bottom);
  
  if ( x+w <= 0 )
  {
    x = SCREEN_X + 20;
    unsigned int half_y = SCREEN_Y / 2;
    top = abs(random(0, half_y));  
    bottom = abs(random(0, half_y));
    clearance = SCREEN_Y - top - bottom;

    if(clearance > MIN_CLEARANCE)
    {
      if ( top >= (clearance - MIN_CLEARANCE) )
        top = top + clearance - MIN_CLEARANCE;  
      else if ( top < (clearance - MIN_CLEARANCE) )
        bottom = bottom + clearance - MIN_CLEARANCE;
    }
    else if(clearance < MIN_CLEARANCE)
    {
      if ( top >= (clearance + MIN_CLEARANCE) )
        top = top - MIN_CLEARANCE - clearance;
      else if( top < (clearance + MIN_CLEARANCE) )
        bottom = bottom - MIN_CLEARANCE - clearance;
    }  
      
  }
  
  x--;  
}


//DISPLAY THE PIPE IN THE OLED SCREEN
void Pipe::showPipe()
{
  oled.drawBox(x, 0, w, top);
  oled.drawBox(x, SCREEN_Y-bottom, w, bottom);  
}


//RETURN THE VALUE OF TOP x
int Pipe::getTopX()
{
  return x;  
}

//RETURN THE VALUE OF TOP y
int Pipe::getTopY()
{
  return top;  
}

//RETURN THE VALUE OF BOTTOM x
int Pipe::getBottomX()
{
  return x;
}

//RETURN THE VALUE OF BOTTOM y
int Pipe::getBottomY()
{
  return SCREEN_Y - bottom;  
}

//RETURN THE VALUE OF THE WIDTH OF PIPE
int Pipe::getWidth()
{
  return w;  
}

//FUNCITON TO CHECK IF THE PIPE IS OUT OF THE SCREEN
bool Pipe::screenOut()
{
  if( x+w <= 0)
    return true;
  else
    return false;  
}

//FUNCITON TO STOP THE PIPE FROM MOVING
void Pipe::pipeStop()
{
  oled.drawBox(x, 0, w, top);
  oled.drawBox(x, SCREEN_Y-bottom, w, bottom);  
}


//DEFINITION OF THE CLASS BIRD
class Bird{

  private:
    int x;
    int y;
    int r;
    int vy;
    float g;
    int score;

  public:
    Bird();
    void updateBird();
    void checkInput(bool s);
    void showScore(Pipe p);
    bool collison(Pipe p);
    void birdReset();
    void birdStop();
  
};

//CONSTRUCTOR FOR THE BIRD CLASS
Bird::Bird()
{
  //CALL FOR birdReset() FUNCTION DURING THE INSTANTIATION OF BIRD OBJECT
  birdReset();
}

//DEFINITION OF birdReset() FUNCTION
//FUNCTION TO RESET THE POSITION OF THE BIRD
void Bird::birdReset()
{
  x = 20;
  y = SCREEN_Y / 2;  
  r = 3;
  vy = 15;
  g = 1;
  score = 0;  
}

//DEFINITION OF updateBird() FUNCTION
//FUNCTION TO UPDATE THE POSITION OF BIRD 
void Bird::updateBird()
{
  
  if ( y+r >= SCREEN_Y )
  {
    y = SCREEN_Y - r; 
  }
  else if ( y-r <= 0 )
  {
    y = r ;  
  }
  oled.drawFrame(0,0,128,64);
  oled.drawDisc(x, y, r);
}

//DEFINITION OF FUNCTION checkInput()
//FUNCTION TO CHECK THE STATE AND MOVE THE BIRD UP IF STATE IS TRUE
void Bird::checkInput(bool s)
{
  static int i = 0;
  if(s)
  {
    y-=2;
    i+=2;
    if(i >= vy)
    {
       state = false;
       i = 0;
    }
  }
  else
    y += g;
}

//DEFINITION OF FUNCTION collison()
//FUNCTION TO CHECK THE COLLISION OF BIRD WITH THE PIPE
bool Bird::collison(Pipe p)
{
  int pipeTX = p.getTopX();
  int pipeBX = p.getBottomX();
  int pipeTY = p.getTopY();
  int pipeBY = p.getBottomY();
  int w = p.getWidth();

//  DEBUGGING SERIAL PRINTS
//  Serial.print("Bird y: ");
//  Serial.println(y);
//  Serial.print("Bird x: ");
//  Serial.println(x);
//  Serial.print("PipeX: ");
//  Serial.println(pipeTX);
//  Serial.print("PipeXW: ");
//  Serial.println(pipeTX + w);
//  Serial.print("Pipe TY: ");
//  Serial.println(pipeTY);
//  Serial.print("Pipe BY: ");
//  Serial.println(pipeBY);

  if ( y <= pipeTY || y >= pipeBY )
  {
    if( (x >= pipeTX) && (x <= (pipeTX + w)) )
    {
      return true;
    }
    else 
      return false;
  }
  else
    return false;
  
}

//DEFINITION OF FUNCTION showScore()
//FUNCITON TO SHOW THE SCORE 
void Bird::showScore(Pipe p)
{
  if(!collison(p) && p.screenOut())
  {
    score++;  
  }  

  oled.setFont(u8g2_font_ncenB08_tr);
  oled.setCursor(100, 15);
  oled.print(score);
}

//DEFINITION OF birdStop()
//FUNCTION TO STOP THE BIRD FROM MOVING
void Bird::birdStop()
{
  oled.drawFrame(0,0,128,64);
  oled.drawDisc(x, y, r); 
  oled.setFont(u8g2_font_ncenB08_tr);
  oled.setCursor(100, 15);
  oled.print(score); 
}

//INSTANTIATE BIRD OBJECT
Bird bird;
//INSTANTIATE PIPE OBJECT
Pipe pipe[2];


//INITIAL SETUP
void setup() {
  
  SPI.setSCK(SPI_CLK);
  oled.begin();
  Serial.begin(115200);
  
  analogReadResolution(BIT_DEPTH);
  randomSeed(analogRead(AUDIO_IN));

  oled.clearBuffer();
  oled.setDrawColor(1);
  oled.drawBox(0, 0, 128, 64);
  oled.setFont(u8g2_font_ncenB10_tr);
  oled.setDrawColor(0);
  oled.drawStr(5, 15, "Initializing ");
  oled.setFont(u8g2_font_ncenB10_tr);
  oled.drawStr(45, 30, "Bit Bird...");
  oled.setFont(u8g2_font_ncenB10_tr);
  oled.drawStr(-1, 48, "BugTheDebugger");
  oled.sendBuffer();
  oled.setDrawColor(1);

  reset_timer = 0;
  while (reset_timer <= MIN_RESET_TIME);
  
}


//MAIN LOOP FUNCTION
void loop() {

  //CALL FUNCTION record_audio() TO START LISTENING TO THE AUDIO INPUT
  record_audio();

  //CHECK LOUDNESS OF CURRENT SOUND
  bool loud_sound = (value - OFFSET) > THRESHOLD;

  //SET in_pulse 
  in_pulse = pulse_detector(loud_sound, in_pulse);
  //SET state
  state = stateChange(in_pulse, state);

  //USING frame_rate INSTEAD OF DELAY TO REMOVE LAG FROM THE GAME
  //NOTE: frame_rate IS A USER DEFINED elapsedTime VARIABLE
  if(frame_rate >= MIN_FRAME_RATE)
  {
    oled.clearBuffer();

    if( !bird.collison(pipe[pipeIndex]) )
    {
      //Serial.println("no woot");
      bird.checkInput(state);
      bird.updateBird();
  
      pipe[pipeIndex].updatePipe();
      pipe[pipeIndex].showPipe();
      bird.showScore(pipe[pipeIndex]);
    }
    else if( bird.collison(pipe[pipeIndex]) )
    {
      //Serial.println("wooot");
      reset_timer = 0;
      bird.birdStop();
      pipe[pipeIndex].pipeStop();

      oled.setFont(u8g2_font_ncenB10_tr);
      oled.setCursor(24, 38);
      oled.print("Game Over!");

      oled.sendBuffer();
      
      while(reset_timer <= MIN_RESET_TIME);
      
      bird.birdReset();
      pipe[pipeIndex].pipeReset();
    }
    
    oled.sendBuffer();
    frame_rate = 0;
  }

}


void record_audio()
{
  value = analogRead(AUDIO_IN);
  time_since_sample = 0;
  Serial.println(value);

  while (time_since_sample <= TIME_BETWEEN_SAMPLES) delayMicroseconds(10); 
}

int pulse_detector(bool ls, int in_pulse)
{
//  Serial.print("Loud Sound: ");
//  Serial.println(ls);
//  Serial.print("in_pulse: ");
//  Serial.println(in_pulse);
//  Serial.print("Time since pulse: ");
//  Serial.println(time_since_pulse);
  
  if( in_pulse == 0 )
  {
    if(ls)
    {
      time_since_pulse = 0;
      return 1;
    }
    else
      return 0;  
  }  
  else if( in_pulse == 1 )
  {
    if(ls)
      return 1;
    else
      return 2;  
  }
  else if( in_pulse == 2 )
  {
    Serial.println("25000");
    if ( state == false && time_since_pulse >= MIN_INTERVAL_TIME )
    {
      state = true;
      Serial.println("25100");
      return 0;
    }
    else if ( state == true && time_since_pulse >= MIN_INTERVAL_TIME )
    {
      Serial.println("25200");
      state = false;
      return 0;
    }
  }
}

bool stateChange(int pulse_in, bool st)
{
    if ( st == false )
    {
      if( in_pulse == 0 || in_pulse == 1)
      {
        return 0;  
      }  
      else if( in_pulse == 2 && time_since_pulse < MIN_INTERVAL_TIME )
      {
        return 0;  
      }
      else if( in_pulse == 2 && time_since_pulse >= MIN_INTERVAL_TIME )
      {
        return 1;  
      }
    }
    else if ( st == true )
    {
      if( in_pulse == 0 || in_pulse == 1 )
      {
        time_since_input = 0;
        return 1;
      }
      else if( in_pulse == 2 && time_since_pulse < MIN_INTERVAL_TIME )
      {
        time_since_input = 0;
        return 1;
      }
      else if( in_pulse == 2 && time_since_pulse >= MIN_INTERVAL_TIME )  
      {
        return 0;
      }
    }
}


