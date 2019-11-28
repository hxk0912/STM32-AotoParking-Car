#ifndef __CONTROL_H
#define __CONTROL_H


#define PI 3.1415926535

enum Direction
{
	Left=0,
	Right=1
};

void PWM_Init(void);
void Encoder_Init(void);
void Kinematic_Analysis(float velocity,float angle);
int Incremental_PI_A (int Encoder,int Target);
int Incremental_PI_B (int Encoder,int Target)	;
void Xianfu_Pwm(void);
void Set_Pwm(int motor_a,int motor_b,int servo);
int Read_Encoder(int TIMx);
void Control(float Velocity,float Angle);
void Move(float Velocity,float Angle,float TurnNumber);
void OutDoor(void);
void ParkCenter(void);
void Park1(void);
void Park2(void);
void Park3(void);
void Park4(void);
void Park4_1(void);
void Back1(void);
void Back2(void);
void Back3(void);
void Back4(void);
void Look(int x);

#endif

