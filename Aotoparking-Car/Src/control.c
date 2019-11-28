#include "control.h"
#include "tim.h"
#include "main.h"
#include "gpio.h"
#define T 0.156f
#define L 0.1445f
#define K 622.8f
#define __HAL_TIM_SET_PULSE1(__HANDLE__, __COUNTER__)  ((__HANDLE__)->Instance->CCR1 = (__COUNTER__))
#define __HAL_TIM_SET_PULSE2(__HANDLE__, __COUNTER__)  ((__HANDLE__)->Instance->CCR2 = (__COUNTER__))
int Flag_Target;
int CaptureNumber=0;    //储存编码器读取值
int TurnNumberA=0;   //记录转过的总圈数
int TurnNumberB;
int Encoder_Left,Encoder_Right;             //左右编码器的脉冲计数
int Motor_A,Motor_B,Servo,Target_A,Target_B;  //电机PWM变量 
float Velocity_KP=10,Velocity_KI=1;	  

/**************************************************************************
函数功能：电机舵机PWM输出初始化
入口参数：无
返回  值：无
**************************************************************************/
void PWM_Init(void)
{
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2);	
}
/**************************************************************************
函数功能：编码器功能初始化
入口参数：无
返回  值：无
**************************************************************************/
void Encoder_Init(void)
{
	HAL_TIM_Encoder_Start(&htim1,TIM_CHANNEL_1);
	HAL_TIM_Encoder_Start(&htim1,TIM_CHANNEL_2);
	HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_1);
	HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_2);
}

/**************************************************************************
函数功能：小车运动数学模型
入口参数：速度和转角
返回  值：无
**************************************************************************/
void Kinematic_Analysis(float velocity,float angle)
{
		Target_A=velocity*(1+T*tan(angle)/2/L); 
		Target_B=velocity*(1-T*tan(angle)/2/L);      //后轮差速
		Servo=1500+angle*K;                    //舵机转向   
}


/**************************************************************************
函数功能：增量PI控制器
入口参数：编码器测量值，目标速度
返回  值：电机PWM
根据增量式离散PID公式 
pwm+=Kp[e（k）-e(k-1)]+Ki*e(k)+Kd[e(k)-2e(k-1)+e(k-2)]
e(k)代表本次偏差 
e(k-1)代表上一次的偏差  以此类推 
pwm代表增量输出
在我们的速度控制闭环系统里面，只使用PI控制
pwm+=Kp[e（k）-e(k-1)]+Ki*e(k)
**************************************************************************/
int Incremental_PI_A (int Encoder,int Target)
{ 	
	 static int Bias,Pwm,Last_bias;
	 Bias=Target-Encoder;                //计算偏差
	 Pwm+=Velocity_KP*(Bias-Last_bias)+Velocity_KI*Bias;   //增量式PI控制器
	 Last_bias=Bias;	                   //保存上一次偏差 
	 return Target;                         //增量输出
}
int Incremental_PI_B (int Encoder,int Target)
{ 	

	 static int Bias,Pwm,Last_bias;
	 Bias=Target-Encoder;                //计算偏差
	 Pwm+=Velocity_KP*(Bias-Last_bias)+Velocity_KI*Bias;   //增量式PI控制器
	 Last_bias=Bias;	                   //保存上一次偏差 
	 return Target;                         //增量输出
}

/**************************************************************************
函数功能：绝对值函数
入口参数：int
返回  值：unsigned int
**************************************************************************/
int myabs(int a)
{ 		   
	  int temp;
		if(a<0)  temp=-a;  
	  else temp=a;
	  return temp;
}
/**************************************************************************
函数功能：限制PWM赋值 
入口参数：无
返回  值：无
**************************************************************************/
void Xianfu_Pwm(void)
{	
	  int Amplitude=800;    //===PWM满幅是100 限制在90
    if(Motor_A<-Amplitude) Motor_A=-Amplitude;	
		if(Motor_A>Amplitude)  Motor_A=Amplitude;	
	  if(Motor_B<-Amplitude) Motor_B=-Amplitude;	
		if(Motor_B>Amplitude)  Motor_B=Amplitude;		
		if(Servo<(1500-500))     Servo=1500-500;	  //舵机限幅
		if(Servo>(1500+500))     Servo=1500+500;		  //舵机限幅
}
/**************************************************************************
函数功能：赋值给PWM寄存器
入口参数：左轮PWM、右轮PWM
返回  值：无
**************************************************************************/
void Set_Pwm(int motor_a,int motor_b,int servo)
{
		__HAL_TIM_SET_PULSE1(&htim3,motor_a);
		__HAL_TIM_SET_PULSE2(&htim3,motor_b);
		__HAL_TIM_SET_PULSE1(&htim4,servo);
}

int Read_Encoder(int TIMx)
{
	switch(TIMx)
	{
		case 1: 
			CaptureNumber=-(__HAL_TIM_GET_COUNTER(&htim1)-10000);
			__HAL_TIM_SET_COUNTER(&htim1, 10000);
			break;
//		case 2: 	
//			CaptureNumber=__HAL_TIM_GET_COUNTER(&htim2)-10000;
//			__HAL_TIM_SET_COUNTER(&htim2, 10000);
//			break;		
	}
	
	return CaptureNumber;  // 返回的是单位时间内轮子转过的圈数
}
	
void Control(float Velocity,float Angle)
{
		HAL_GPIO_WritePin(STBY_GPIO_Port,STBY_Pin,GPIO_PIN_SET);
		TurnNumberA+=Read_Encoder(1);
//	  Encoder_Right=Read_Encoder(2);   																	//===读取编码器的值
	
	if(Velocity>0)
	{
		HAL_GPIO_WritePin(AIN1_GPIO_Port,AIN1_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port,AIN2_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(BIN1_GPIO_Port,BIN1_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BIN2_GPIO_Port,BIN2_Pin,GPIO_PIN_SET);		
		Kinematic_Analysis(Velocity,Angle);     														//小车运动学分析   
		Motor_A=Incremental_PI_A(Encoder_Left,Target_A);                   //===速度闭环控制计算电机A最终PWM
		Motor_B=Incremental_PI_B(Encoder_Right,Target_B);                  //===速度闭环控制计算电机B最终PWM 
		Xianfu_Pwm();                                                      //===PWM限幅
		Set_Pwm(Motor_A,Motor_B,Servo);   																	//===赋值给PWM寄存器  
	}		   
	else	
	{	
		HAL_GPIO_WritePin(AIN1_GPIO_Port,AIN1_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port,AIN2_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BIN1_GPIO_Port,BIN1_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(BIN2_GPIO_Port,BIN2_Pin,GPIO_PIN_RESET);		
		Kinematic_Analysis(-Velocity,Angle);     														//小车运动学分析   
		Motor_A=Incremental_PI_A(Encoder_Left,Target_A);                   //===速度闭环控制计算电机A最终PWM
		Motor_B=Incremental_PI_B(Encoder_Right,Target_B);                  //===速度闭环控制计算电机B最终PWM 
		Xianfu_Pwm();                                                      //===PWM限幅
		Set_Pwm(Motor_A,Motor_B,Servo);   																	//===赋值给PWM寄存器  
	}		
}


void Move(float Velocity,float Angle,float TurnNumber)
{
	if(TurnNumber>0)
	{
	while((TurnNumber*390*4-TurnNumberA)>0)
	{
	Control(Velocity,Angle);
	HAL_Delay(10);
	}
	Control(0,0);
	TurnNumberA=0;
	}
	else
	{
	while((TurnNumber*390*4-TurnNumberA)<0)
	{
	Control(Velocity,Angle);
	HAL_Delay(10);
	}
	Control(0,0);
	TurnNumberA=0;
	}
}

void OutDoor(void)
{
		Move(200,0,2.5);
}

void ParkCenter()
{
	Move(200,-PI*50/180,0.98);
	Move(200,0,0.7);
}

void Park1(void)
{
	Move(-200,0,-0.2);
	Move(-200,PI*50/180,-2.8);
	Move(-200,0,-1);
}

void Park2(void)
{
	Move(-200,0,-1.9);
	Move(200,PI*50/180,3.0);
}

void Park3(void)
{
	Move(-200,0,-0.6);
	Move(200,PI*50/180,3.10);
}

void Park4(void)
{
	Move(-200,0,-1.95);
	Move(200,-PI*48/180,0.8);
	Move(200,PI*60/180,1.9);
	Move(-100,0,-0.5);
}
void Park4_1(void)
{
	Move(-200,0,-1.95);
	Move(200,-PI*48/180,0.7);
	Move(200,PI*60/180,1.5);
	Move(-100,0,-0.3);
}
void Back1(void)
{
	Move(200,0,4);
}
void Back2(void)
{
	Move(-200,-PI*50/180,-1.10);
	Move(200,0,0.5);
	Move(200,PI*50/180,3.1);
}

void Back3(void)
{
	Move(-200,PI*50/180,-3.3);
	Move(200,0,0.5);
	Move(-200,-PI*50/180,-1.0);
	Move(-200,0,-1.0);
	
}
void Look(int x)
{
	switch(x)
	{
		case 0:
			__HAL_TIM_SET_PULSE2(&htim4,500);
			break;
		case 1:
			__HAL_TIM_SET_PULSE2(&htim4,2300);
			break;
		case 2:
			__HAL_TIM_SET_PULSE2(&htim4,1500);
			
	}
}
