/*
 * main.cpp

 *
 *  Created on: 2018/01/17
 *      Author: yoneken
 */
#include <mainpp.h>
#include <ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Float64.h>
#include <geometry_msgs/Twist.h>
#include <iostream>

//Private variable
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

//Private function
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);

//Private user code
uint8_t RX2_Char[2];
uint8_t Char_Buffer[20];
uint8_t Char_Buffer_length = 0;
uint8_t Char_Buffer_isRecieving = 0;

uint8_t MSG1[50];
uint8_t MSG1_length;

uint8_t MSG2[50];
uint8_t MSG2_length;

uint32_t encoder_value = 0;
float rpm_value = 0.00;
//OWENNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN A
float linear_vel_x=0.0;
float angular_vel_z=0.0;
uint8_t direction_l = 1;
uint8_t direction_r = 1;
float wheel_l = 0.0;
float wheel_r = 0.0;
int dir_l = 0;
int dir_r = 0;
ros::NodeHandle nh;

std_msgs::String str_msg;
std_msgs::Float64 float_msg;
/*
communicate with ros-master
*/

ros::Publisher pub2ros("wheel_vel", &str_msg);

void cmd_subCb(const geometry_msgs::Twist& toggle_msg){
 linear_vel_x = toggle_msg.linear.x;
 angular_vel_z = toggle_msg.angular.z;
}

ros::Subscriber<geometry_msgs::Twist> cmd_sub("cmd_vel", &cmd_subCb);
//OWENNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN B
void set_speed(float Vx, float W ){
 direction_l = 1;
 direction_r = 1;
 float _Wl = ((Vx - (2.22169002*W))/1.08)*100.0;
 float _Wr = (((2.22169002*W) + Vx)/1.08)*100.0;
 if(_Wl < 0){
  direction_l = 0;
  _Wl = -_Wl;
  }
 if(_Wr <0){
  direction_r = 0;
  _Wr = -_Wr;
 }
 wheel_l = _Wl;
 wheel_r = _Wr;
 dir_l = direction_l;
 dir_r = direction_r;
}
//OWENNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN A
char buffer[50];
int sOut;
//OWENNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN B
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
  nh.getHardware()->flush();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  nh.getHardware()->reset_rbuf();
}

void setup(void)
{
  nh.initNode();
  //OWENNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN A
  nh.advertise(pub2ros);
  nh.subscribe(cmd_sub);
  //OWENNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN B
}

void loop(void)
{
 while(1){
 HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
  //OWENNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN A
  set_speed(linear_vel_x, angular_vel_z);
  sOut = sprintf(buffer, "%f,%f,%d,%d", wheel_l, wheel_r, dir_l, dir_r);
  str_msg.data = buffer;
  pub2ros.publish( &str_msg);
  nh.spinOnce();
  //OWENNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN B
  HAL_Delay(1000);
 }
}
