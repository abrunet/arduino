#define FRONT_IR_SENSOR_PIN  1 // Must be an analog pin

int DEBUG_DUMP = 1;

int nRearSafetyThreshold = 100; // When the rover approaches the edge the signal falls from 400 to less than 100.
int nfrontSafetyThreshold = 250; // When the rover approaches the edge the signal falls from 400 to less than 100.

int E1 = 6; //M1 Speed Control
int E2 = 5; //M2 Speed Control
int M1 = 8; //M1 Direction Control
int M2 = 7; //M2 Direction Control

/// GLOBAL STATES
int MOVING_FORWARD = 1;
int TURNING_LEFT = 2;
int TURNING_RIGHT = 3;
int REVERSING = 4;
int STOPPED = 5;
int INIT = 6;
int nMovementState_ = INIT;

/// PERMISSION STATES
boolean bSafeToReverse_ = false;
boolean bSafeToGoForward_ = true;
int lastSensorReading = 0;


void setup()
{
	int i;
	for(i=5;i<=8;i++)
	{
		pinMode(i, OUTPUT);
	}
	Serial.begin(115200); //Set serial baud rate to 115200
}


boolean isDangerousRearSensorValue(int n)
{
	return n < nRearSafetyThreshold;
}


boolean isDangerousFrontSensorValue(int n)
{
//	return n < nfrontSafetyThreshold;
        return n > nfrontSafetyThreshold;
}


void readRearIRSensor()
{
        int nRearSensorValue;
//	nRearSensorValue = analogRead(REAR_IR_SENSOR_PIN);
	if (DEBUG_DUMP) Serial.print(" RearIR:");
	if (DEBUG_DUMP) Serial.print(nRearSensorValue);
	bSafeToReverse_ = ! isDangerousRearSensorValue(nRearSensorValue);
	if (nMovementState_ != STOPPED && nMovementState_ != MOVING_FORWARD)
	{
		if (! bSafeToReverse_)
		{
			if (DEBUG_DUMP) Serial.print(" EMERGENCY STOP ");
			stop();
		}
	}
}


void readFrontIRSensor()
{
        int leftspeed = 200; //255 is maximum speed
	int rightspeed =  204;
  	int nFrontSensorValue = analogRead(FRONT_IR_SENSOR_PIN);
	if (DEBUG_DUMP) Serial.print(" FrontIR:");
	if (DEBUG_DUMP) Serial.print(nFrontSensorValue);
	bSafeToGoForward_ = ! isDangerousFrontSensorValue(nFrontSensorValue);

	if (nMovementState_ != STOPPED && nMovementState_ != REVERSING)
	{
//		if (! bSafeToGoForward_)
//		{
//			if (DEBUG_DUMP) Serial.print(" EMERGENCY STOP ");
//			stop();
//		}
              if( nMovementState_ == MOVING_FORWARD && ! bSafeToGoForward_) {
                  stop();
                  right(leftspeed,rightspeed); 
              }
              if( nMovementState_ == TURNING_RIGHT && nFrontSensorValue < 70) {
                 delay(500);
                 stop();
                 forward(leftspeed,rightspeed);
              }
	}

      if( nMovementState_ == INIT && lastSensorReading != 0 && nFrontSensorValue > 500 && lastSensorReading < 100) {
        Serial.print(" STARTING TO MOVE YO! ");
        bSafeToGoForward_ = true;
        forward(leftspeed,rightspeed);
      }
      lastSensorReading = nFrontSensorValue;
}

void stop(void) //Stop
{
	if (DEBUG_DUMP) Serial.print(" StopCommand ");
	digitalWrite(E1,LOW);
	digitalWrite(E2,LOW);
	nMovementState_ = STOPPED;
}


void forward(char a,char b)
{
	if (DEBUG_DUMP) Serial.print(" ForwardCommand ");
	if (bSafeToGoForward_)
	{
		analogWrite (E1,a);
		digitalWrite(M1,LOW);
		analogWrite (E2,b);
		digitalWrite(M2,LOW);
		nMovementState_ = MOVING_FORWARD;
	}
}


void reverse (char a,char b)
{
	if (DEBUG_DUMP) Serial.print(" ReverseCommand ");
	if (bSafeToReverse_)
	{
		analogWrite (E1,a);
		digitalWrite(M1,HIGH);
		analogWrite (E2,b);
		digitalWrite(M2,HIGH);
		nMovementState_ = REVERSING;		
	}
}


void left (char a,char b)
{
	if (DEBUG_DUMP) Serial.print(" LeftCommand ");
	analogWrite (E1,a);
	digitalWrite(M1,HIGH);
	analogWrite (E2,b);
	digitalWrite(M2,LOW);
	nMovementState_ = TURNING_LEFT;		
}


void right (char a,char b)
{
	if (DEBUG_DUMP) Serial.print(" RightCommand ");
	analogWrite (E1,a);
	digitalWrite(M1,LOW);
	analogWrite (E2,b);
	digitalWrite(M2,HIGH);
	nMovementState_ = TURNING_RIGHT;		
}


void obtainCommandInput()
{
	if (DEBUG_DUMP) Serial.print(" ReadingCommands ");

	if( Serial.available() )           // if data is available to read
	{
		if (DEBUG_DUMP) Serial.print(" CharactersRead ");

		char val = Serial.read();

		int leftspeed = 200; //255 is maximum speed
		int rightspeed =  204;
		if (DEBUG_DUMP) Serial.print(val);
		if (DEBUG_DUMP) Serial.print(" ");

		switch (val) // Perform an action depending on the command
		{
			case 'w'://Move Forward
				forward (leftspeed,rightspeed);
				break;
			case 's'://Move Backwards
				reverse (leftspeed,rightspeed);
				break;
			case 'a'://Turn Left
				left (leftspeed,rightspeed);
				break;
			case 'd'://Turn Right
				right (leftspeed,rightspeed);
				break;
			default:
				stop();
			break;
		}
	}

}


void loop()
{
	readFrontIRSensor();
	obtainCommandInput();
	delay(100);
	Serial.println();
}
