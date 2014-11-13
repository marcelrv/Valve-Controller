
#include <AFMotor.h>

#define valveRuntime 15000L
#define writeValvePositions false


AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

AF_DCMotor * motors[] = {
  &motor1, &motor2, &motor3, &motor4};
unsigned long stopTimer[NUMBER_MOTORS] ;
uint8_t motorPositions [NUMBER_MOTORS];

//setup motors with max speed but not powered
void motorsetup() {
  statusMessage  ("Setup Valves");
  for (int i = 0; i < NUMBER_MOTORS; i++) {
    motors[i]->setSpeed(255);
    motors[i]->run(RELEASE);
    stopTimer[i] = 0;
    motorPositions [i] =  255; //undefined
  }
}


//releases vales at when time reaches the stoptime
void valveLoop() {
  for (int i = 0; i < NUMBER_MOTORS; i++) {
    if (stopTimer[i] != 0 && stopTimer[i] < millis()) {
      motors[i]->run(RELEASE);
      publish_MotorPos(i);
      snprintf (msgBuffer, MSGSIZE, "Valve %d motor released", i);
      statusMessage  (msgBuffer);
      if (writeValvePositions) writeValvePosition(i, motorPositions [i]);
      stopTimer[i] = 0;
    }
  }
}

void ValveOpen(byte motorNum) {
  if (stopTimer[motorNum] == 0 && motorPositions[motorNum] != FORWARD )  OpenValve(motorNum);
}

void ValveClose(byte motorNum) {
  if (stopTimer[motorNum] == 0 && motorPositions[motorNum] != BACKWARD)   CloseValve(motorNum);
}

void OpenValve(byte motorNum) {
  snprintf (msgBuffer, MSGSIZE, "Valve %d Opening", motorNum);
  statusMessage  (msgBuffer);
  ValveMove(motorNum, FORWARD);
  motorPositions[motorNum] = FORWARD;
}

void CloseValve(byte motorNum) {
  snprintf (msgBuffer, MSGSIZE, "Valve %d Closing", motorNum);
  statusMessage  (msgBuffer);
  ValveMove(motorNum, BACKWARD);
  motorPositions[motorNum] = BACKWARD;
}

void ValveMove(byte motorNum, uint8_t dir) {
  //  snprintf (msgBuffer, MSGSIZE, "Valve %d motor move: %d", motorNum,dir);
  //  statusMessage  (msgBuffer);
  motors[motorNum]->run(dir);
  stopTimer[motorNum] = millis() + valveRuntime;
}

   void publish_MotorPos(int motorNum) {
     snprintf (TopicBuffer, TOPICSIZE, "valves/%1d/pos",motorNum);
     publishMQTT (TopicBuffer,  (long) motorPositions [motorNum]);       
}

//void motorRun(byte motorNum, uint8_t dir) {
//  snprintf (msgBuffer, MSGSIZE, "Valve %d motor move: %d", motorNum,dir);
//  statusMessage  (msgBuffer);
//   switch (motorNum) {
//    case 1:
//      motor1.run(dir);
//      break;
//    case 2:
//      motor2.run(dir);
//      break;
//    default:
//      snprintf (msgBuffer, MSGSIZE, "Valve %d not defined", motorNum);
//        statusMessage  (msgBuffer);
//  }
//}





