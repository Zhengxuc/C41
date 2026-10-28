
//-------------------------------------------------------------------------------------------------
//# include <msp430x14x.h>
#include "Device.h"

extern void uart0_init();   //串口初始化 波特率9600
extern void hart_task (void);
extern unsigned char Checksum(unsigned char *s , unsigned char nbytes);//计算校验和
extern void com_send(unsigned char *send, unsigned char send_bytes);//通过串口发送数据
extern void hart_resp( unsigned char *resp_data, unsigned char resp_num, unsigned char *addr ,unsigned char cmd_num);//按HART协议的格式生成待发送的数据帧send[]
unsigned char  x1;               //用于区别长结构和短结构的变量，x1＝1 短，x1=2时长
unsigned char receive[100];     //用于存放接收的数据 
unsigned char send[100];        //用于存放发送的数据 
unsigned char cmd_data[25];     //命令数据区
unsigned char byte_count;       //数据长度
unsigned char start_short[3]={0xFF,0xFF,0x02};   //用于寻找短结构起始字节
unsigned char start_long[3]={0xFF,0xFF,0x82};    //用于寻找长结构起始字节
unsigned char rec_ptr=0;           //用于给接收到的数据编号
unsigned char received;          //recived＝1时表示已接收到一帧数据，对数据帧解析完recived被置为0        
unsigned char rec_bytes;         //接收的字节数
unsigned char error,comm_error,command_error,old_device,needs_burn;
unsigned char polling_addr;     //轮询地址,初始值为0
unsigned char *p_polling_addr=&polling_addr;//指向轮询地址的指针

unsigned char short_addr = 0x00 ;     //本机短结构地址，ABB握手时的值
unsigned char long_addr[5]={0x26,0x31,0x01,0x00,0x00};   //本机长结构地址
unsigned char long_addr_cmp[4]={0x31,0x01,0x00,0x00};   //本机长结构地址
unsigned char cmd_num;          //命令号
unsigned char pre_num;         //前导符的个数
unsigned char status1,status2;  //状态位
unsigned char send_bytes;       //发送数据的字节数
unsigned char not_resp;         //表示从机不需要响应的标志，该变量为1时表示不响应
unsigned char pv_unit_code =0x39;//%
unsigned char v2_unit_code =0x27;//mA
unsigned char v3_unit_code =0x20;//℃
unsigned char v4_unit_code =0x39;
unsigned char unit_code_arry[4]={0x39,0x27,0x39,0x39};
//--------------------------------------------------------------------------------------------------
//命令响应数据填充区
// unsigned char resp_data_0[14] ={0xfe,0x16,0x41,0x07,0x05,0x01,0x1e,0x18,0x02,0x06,0x04,0xc8};
unsigned char resp_data_0[22] ={0xFE,0xE6,0x31,0x05,0x07,0x01,0x01,0x08,0x00,0x01,0x00,0x00,0X07,0X84,
                                0X00,0X00,0X00,0X61,0X97,0X00,0X00,0X01};
unsigned char resp_data_1[5];
unsigned char resp_data_2[8];
unsigned char resp_data_3[24];
unsigned char resp_data_6[2];
unsigned char resp_data_8[4] ={0};
unsigned char resp_data_9[69] ={0};
unsigned char resp_data_13[21];
unsigned char resp_data_14[16];
unsigned char resp_data_15[18];
unsigned char resp_data_17[24];
unsigned char resp_data_48[25]={0};

//unsigned char tag[6] ={0xE6,0x31,0x07,0x01,0x00,0x00}; //标签:设备类型、修订等级、设备ID
unsigned char tag[6] ={0x01,0x01,0x01,0x01,0x01,0x01}; //标签
unsigned char long_tag[32] ={0};//长标签
unsigned char configure_cnt[2] ={0};//配置计数器
unsigned char descriptor[12] ={0x82,0x08,0x20,0x82,0x08,0x20,0x82,0x08,0x20,0x82,0x08,0x20};  //描述符
unsigned char date[3] ={0x06,0x07,0x7C};
unsigned char message[24] ={0x82,0x08,0x20,0x82,0x08,0x20,0x82,0x08,0x20,0x82,0x08,0x20,0x82,0x08,0x20,
                           0x82,0x08,0x20,0x82,0x08,0x20,0x82,0x08,0x20};
unsigned char status[25] ={0x00,0x00 ,0x00,0x00,0x8F,0x00,0x13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                          0x00,0x00 ,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10 };//48，读附加设备信息
unsigned char Actuator_inf[5] ={0x00 ,0x00 ,0x01 ,0x00 ,0x00};
unsigned char sensor[12]={0x0C,0x87,0x40,0x02,0x3C,0x00,0x00,0x00,0x00,0x0C,0x87,0x40};
unsigned char alarm_mark[1] ={0x00};


unsigned char final_ass_num[3] ={0x00,0x00,0x01};
unsigned char resp_num_13 =21;
unsigned char resp_num_14 =16;
unsigned char resp_num_3 =24;
unsigned char rev_float_buf[4];





unsigned char *p_pv_value ;
unsigned char *p_pv_current ;  //主变量电流值
unsigned char *p_pv_upper ;  //主变量上限值
unsigned char *p_pv_lower ;   //主变量下限值
unsigned char *p_v2_value ;
unsigned char *p_v3_value ;
unsigned char *p_v4_value ;
unsigned char *p_pv_sensor_upper ;
unsigned char *p_pv_sensor_lower ;
unsigned char *p_pv_min_span ;
unsigned char *p_opening_back ;
unsigned char *p_pv_ctr_difference;
unsigned char *p_pv_band;
unsigned char *p_pv_qof;
unsigned char *p_pv_temperature;
unsigned char *p_pv_damping ;  //将指针和对应的数据值一一对应
unsigned char *p_resp_13_value[4];        
unsigned char *p_resp_13_arry[8];
        
        
float  pv_value ;
float  pv_current ;  //主变量电流值
float  pv_upper ;  //主变量上限值
float  pv_lower ;   //主变量下限值
float  v2_value ;
float  v3_value ;
float  v4_value ;
float  pv_sensor_upper ;                                      
float  pv_sensor_lower ;
float  pv_min_span ;
float  pv_ctr_difference;//控制偏差
float  pv_band=0;         //容差
float  pv_qof=0;          //速关
float  pv_temperature;    //温度
float  pv_damping_value ;  //将指针和对应的数据值一一对应

unsigned char pv_upper_754[4];    //主变量量程上限值IEEE754形式
unsigned char pv_lower_754[4];    //主变量量程下限值IEEE754形式
unsigned char pv_current_754[4];  //主变量电流值IEEE754形式
unsigned char pv_damping_754[4];
unsigned char pv_sensor_upper_754[4];
unsigned char pv_sensor_lower_754[4];
unsigned char pv_min_span_754[4];
unsigned char pv_754[4];
unsigned char v2_754[4];
unsigned char v3_754[4];
unsigned char v4_754[4];

unsigned char type_gas=0;//阀门气体状态  0/排气  1/充气



/*主程序运行后,程序一直在等待.当串口接收到一个完整的数据帧之后,received置1,主程序对接收到
的数据帧进行解析,若解析正确,则根据数据帧中的命令号执行相应的操作.*/

void hart_task (void)
{ 
  unsigned char i,j,t;
  unsigned char cs_start;
  unsigned char *r;
  unsigned char *dev_addr;
  unsigned char indexs;
  float percent_current;//电流百分比
 // float pv_range_span,f1;   //主变量量程跨度
  //WDTCTL = WDTPW + WDTHOLD;  // Stop WDT
  error = 0;               
  status1 = 0;       
  status2 = 0;
  byte_count = 0;
  memset(cmd_data,0,25);
   //pv_value = (float)(DIS.CurrentBackDis - OP.Down_4_1)/(float)( OP.Up_4_2 - OP.Down_4_1 )*100;
  pv_value = AD.opening_back;  // pv_value = g_PIDServiceData.Cur_ValvePoint *0.1;  //20161022修改
  
  
  if(AD.CurrentDis >VAL.V_p8_4_MIN)
   {
       pv_current = 4.0+16.0/( VAL.V_p8_5_MAX - VAL.V_p8_4_MIN )*(AD.CurrentDis-VAL.V_p8_4_MIN);     //主变量电流值
   }  
   else
   {
       pv_current = 4.0-16.0/( VAL.V_p8_5_MAX - VAL.V_p8_4_MIN )*(VAL.V_p8_4_MIN -AD.CurrentDis);     //主变量电流值
   }
   pv_upper =120.0;                      //主变量电流上限值
   pv_lower = -12.0;                     //主变量电流下限值
   pv_sensor_upper=120.0;
   pv_sensor_lower=-12.0;
   pv_min_span=0.1;
   pv_damping_value=0;
   v2_value =4+pv_value*0.16;           //环路电流
   percent_current =(pv_current-4.0)/0.16;   //4-20mA百分比
   if(percent_current<0) percent_current=0;
   if(percent_current>100) percent_current=100;
   v3_value=pv_value-percent_current;//TV  
   v4_value = AD.IP_Current_Percent;//IP电流百分比/QV
     
  
   

  if(CLK.received==2) 
  { 
      not_resp=0;
      r=receive;
      for(j=1;j<=rec_bytes;++j)
      {
        if (strncmp((const char*)r,(const char*)start_short,3)==0) 
        {
            x1=1; break;
        }
        if (strncmp((const char*)r,(const char*)start_long,3)==0)
        {
            x1=2; break;
        }
        r++;
      }
      if(j>rec_bytes)          //当程序未找到0x02或0x82时 
      {
        ++error;// break;
      }
      pre_num = j+1;
      j +=2 ; r +=2;              //此时j，r 都指向起始字节
      cs_start = j;
      ++j; ++r;              //指向地址首字节
      if(rec_bytes-j<2)        //当剩余数据数少于3时
      {
        ++error; //break;
      }
      if (x1==1)               //地址为短结构
      {
        if(((*r)&0x3F) != (short_addr&0x3F))     //短结构地址与本从机短地址不同时
        {
          ++error;// break;
        }
        else
        {
          if(*r==0x80)short_addr=0x80;
          else if(*r==0x00)short_addr=0x00;
          ++j; ++r;            //指向命令号
        }                 
      }
      if(x1==2)               //地址为长结构
      {
        
        if(((*r)&0x3F) != (long_addr[0]&0x3F))
        {
              ++error;// break;  
        
        }
        else
        {
              if(((*r)>>7)&1)long_addr[0]=0xA6;
              else long_addr[0]=0x26;
              ++j; ++r;
              if(strncmp((const char*)r,(const char*)long_addr_cmp,4)!=0)  //长结构地址与本从机长地址不同时
              {
                ++error; //break;
              }
              else
              {
                j += 4; r +=4;           //指向命令号
              } 
        
        }
             
      }
      cmd_num = *r;
      j++; r++;               //指向数据长度字节
      byte_count = *r;
      if (byte_count==0) 
      {
        j++; r++;           //指向校验位 
      } 
      else
      { 
        r++;                //指向命令数据的第一个字节
        j += byte_count + 1;
        for(int i=0;i<byte_count;i++)
          cmd_data[i]=*r++;
      }
      r = &receive[cs_start-1];
      memset(receive,0,100);
      if ((Checksum(r, j-cs_start+1)) != 0) ++error;  
      if (error==0) 
      {
 
           p_pv_value = (unsigned char *)&pv_value;
           p_pv_current = (unsigned char *)&pv_current;  //主变量电流值
           p_pv_upper = (unsigned char *)&pv_upper;  //主变量上限值
           p_pv_lower = (unsigned char *)&pv_lower;   //主变量下限值
           p_v2_value = (unsigned char *)&v2_value;
           p_v3_value = (unsigned char *)&v3_value;
           p_v4_value = (unsigned char *)&v4_value;
           p_pv_sensor_upper = (unsigned char *)&pv_sensor_upper;
           p_pv_sensor_lower = (unsigned char *)&pv_sensor_lower;
           p_pv_min_span = (unsigned char *)&pv_min_span;
           p_pv_damping = (unsigned char *)&pv_damping_value;  //将指针和对应的数据值一一对应
          if(x1==1) dev_addr=&short_addr;
           else dev_addr=long_addr;
           switch(cmd_num)
           {
              case   0: //0号命令组帧和其它命令不同
                         hart_resp(resp_data_0, resp_num_0, dev_addr,cmd_num);                          
                         break;
                    
              case   1: /*将主变量单位代码和主变量值IEEE754形式填充到1号命令响应数据中中*/
                        resp_data_1[0] = pv_unit_code;

                        for(i=0;i<4;i++)
                        resp_data_1[4-i] = pv_754[i] = *p_pv_value++;
                        
                        hart_resp(resp_data_1, resp_num_1,dev_addr,cmd_num);
                        break;
                   
              case   2: 
                        for(i=0;i<4;i++)
                       {
                         resp_data_2[3-i] = v2_754[i] = *p_v2_value++;
                         resp_data_2[7-i] = pv_754[i] = *p_pv_value++; 
     
                        }
                
                        hart_resp(resp_data_2, 8,dev_addr,cmd_num);
                        break;
                    
              case   3: /*按命令3响应数据的格式填充相关数据*/
                       for(i=0;i<4;i++)
                       {
                        resp_data_3[3-i] = v2_754[i] = *p_v2_value++;
                         resp_data_3[8-i] = pv_754[i] = *p_pv_value++; 
                         resp_data_3[13-i] = pv_current_754[i] = *p_pv_current++;
                         resp_data_3[18-i] = v3_754[i] = *p_v3_value++;
                         resp_data_3[23-i] = v4_754[i] = *p_v4_value++;
                        }
                        resp_data_3[4] = pv_unit_code;
                        resp_data_3[9] = v2_unit_code;
                        resp_data_3[14] = v3_unit_code;
                        resp_data_3[19] = v4_unit_code;
                        hart_resp(resp_data_3, resp_num_3,long_addr,cmd_num);
                        break;
                        
              case   6: /*写入轮询地址和环路电流模式*/
                    polling_addr = cmd_data[0];
                    short_addr = (0x80|polling_addr);
                    resp_data_6[0]= polling_addr;
                    resp_data_6[1]= 0;
                    hart_resp(resp_data_6,2,dev_addr,cmd_num);
                    break;
                    
              case   7: /*读取轮询地址和环路电流模式*/                  
                    resp_data_6[0]= 0;
                    resp_data_6[1]= 0;
                    hart_resp(resp_data_6, 2,dev_addr,cmd_num);
                    break;
                    
              case   8: /*读取动态变量分类*/                  
                    hart_resp(resp_data_8, 2,dev_addr,cmd_num);
                    break;
                    
              case   9: /*通过状态读取设备变量*/
                    p_resp_13_value[0]=p_pv_value;
                    p_resp_13_value[1]=p_pv_current;
                    p_resp_13_value[2]=p_v3_value;
                    p_resp_13_value[3]=p_v4_value;
                    resp_data_9[byte_count*8+1]=(CLK.Time_Cnt_Hart>>24)&0xFF;
                    resp_data_9[byte_count*8+2]=(CLK.Time_Cnt_Hart>>16)&0xFF;
                    resp_data_9[byte_count*8+3]=(CLK.Time_Cnt_Hart>>8)&0xFF;
                    resp_data_9[byte_count*8+4]=(CLK.Time_Cnt_Hart)&0xFF;
                    if(byte_count>8)byte_count=8;
                    for(i=0;i<byte_count;i++)
                    {
                      if(cmd_data[0+i]>3)cmd_data[0+i]=0;
                      resp_data_9[1+i*8]=cmd_data[0+i];
                      resp_data_9[3+i*8]=unit_code_arry[cmd_data[0+i]];
                      p_resp_13_arry[i]=p_resp_13_value[cmd_data[0+i]];

                      for(j=0;j<4;j++)
                      {
                        resp_data_9[(i+1)*8-1-j] = pv_754[j] = *p_resp_13_arry[i]++;

                      }
                    }
                    hart_resp(resp_data_9, (byte_count*8)+5,dev_addr,cmd_num);
                    break;
              case  11: if(strncmp((const char*)cmd_data,(const char*)tag,6)==0)//当主机发来的设备标签和设备本身标签相同时
                      hart_resp(resp_data_0, resp_num_0,dev_addr,cmd_num);
                    else
                      not_resp=1;  //当主机发来的设备标签和设备本身标签不相同时,不响应主机的命令
                    break;      
              case  12: hart_resp(message, 24,dev_addr,cmd_num);
                    break;
                    
              case  13: /*将标签,描述符和日期填充到13号命令响应数据中中*/
                    for(i=0;i<6;i++)
                      resp_data_13[i]=tag[i];
                    for(i=6;i<18;i++)
                      resp_data_13[i]=descriptor[i-6];
                    for(i=18;i<21;i++)
                      resp_data_13[i]=date[i-18];
                    hart_resp(resp_data_13,resp_num_13,dev_addr,cmd_num);
                    break;
                   
              case  14: /*将传感器序列号等内容填充到14号命令响应数据数组中*/
                    for(i=0;i<3;i++)
                    resp_data_14[i] = 0;     //填充主变量传感器序列号
                    resp_data_14[3] = pv_unit_code;  //填充主变量传感器单位代码
                    for(i=0;i<4;i++)
                    {
                      resp_data_14[7-i] = pv_sensor_upper_754[i] = *p_pv_sensor_upper++;
                      resp_data_14[11-i] = pv_sensor_lower_754[i] = *p_pv_sensor_lower++;
                      resp_data_14[15-i] = pv_min_span_754[i] = *p_pv_min_span++;
                    }           
                    hart_resp(resp_data_14, resp_num_14,dev_addr,cmd_num);
                    break;
                    
              case  15: /*填充相关数据*/
                    resp_data_15[0] = pv_alarm_code;
                    resp_data_15[1] = pv_xfer_code;
                    resp_data_15[2] = pv_unit_code;                   
                    for(i=0;i<4;i++)
                    {
                      resp_data_15[6-i] = pv_upper_754[i] = *p_pv_upper++;                   
                      resp_data_15[10-i] = pv_lower_754[i] = *p_pv_lower++;                
                      resp_data_15[14-i] = pv_damping_754[i] = *p_pv_damping++;
                    }
                    resp_data_15[15] = write_protect_code;
                    resp_data_15[16] = 250;
                    resp_data_15[17] = 1;            
                    hart_resp(resp_data_15, 18,dev_addr,cmd_num);
                    break;
                    
              case  16: hart_resp(final_ass_num, 3,dev_addr,cmd_num);
                    break;

              case  17:
                    for(i=0;i<24;i++)
                    {
                      resp_data_17[i]=cmd_data[i];
                    }
                    hart_resp(resp_data_17, 24,dev_addr,cmd_num);
                    break;
              
              case  18: /*写入标签,描述符和日期*/
                    for (i=0;i<6;i++)
                      tag[i]=cmd_data[i];
                    for(i=6;i<18;i++)
                      descriptor[i-6]=cmd_data[i];
                    for(i=18;i<21;i++)
                      date[i-18]=cmd_data[i];


                    for(i=0;i<6;i++)
                      resp_data_13[i]=tag[i];
                    for(i=6;i<18;i++)
                      resp_data_13[i]=descriptor[i-6];
                    for(i=18;i<21;i++)
                      resp_data_13[i]=date[i-18];
                    hart_resp(resp_data_13,resp_num_13,dev_addr,cmd_num);
                    break;
              case  19:/*写入最终装配编号*/
                    for ( i = 0; i < 3; i++)
                    {
                      final_ass_num[i]=cmd_data[i];
                    }
                    
                    hart_resp(final_ass_num, 3,dev_addr,cmd_num);
                    break;
              case  20:/*读取长标签*/
                    hart_resp(long_tag, 32,dev_addr,cmd_num);
                    break;
              case  21:/*读取长标签相关标识符*/
                    if(strncmp((const char*)cmd_data,(const char*)long_tag,32)==0)//当主机发来的设备标签和设备本身标签相同时
                      hart_resp(resp_data_0, resp_num_0,dev_addr,cmd_num);
                    else
                      not_resp=1;  //当主机发来的设备标签和设备本身标签不相同时,不响应主机的命令
                    break;
              case  22:/*写入长标签*/
                    for ( i = 0; i < 32; i++)
                    {
                      long_tag[i]=cmd_data[i];
                    }
                    hart_resp(long_tag, 32,dev_addr,cmd_num);
                    break;
              case  38:/*重置已更改配置标志*/
                    configure_cnt[0]=resp_data_0[14]=cmd_data[0];
                    configure_cnt[1]=resp_data_0[15]=cmd_data[1];
                    hart_resp(configure_cnt, 2,dev_addr,cmd_num);
                    break;
              case  48:/*读取附加设备状态*/
                    if(byte_count)
                    {
                      for (i = 0; i < 25; i++)
                      {
                        resp_data_48[i]=cmd_data[i];
                      }
                    }

                    hart_resp(resp_data_48, 25,dev_addr,cmd_num);
                    break;                         
                  
              case 128: /*读取指令*/
                        if(byte_count){    //有请求字节                           
                            indexs=cmd_data[0];                                                    
                        }
                        else indexs=0;

                        switch (indexs)
                        {
                        case 1:/*容差*/
                          pv_band = (float)VAL.V_p1_3BRND/10.0;                       
                          p_pv_band = (unsigned char *)&pv_band;
                          resp_data_1[0] = indexs;

                          for(i=0;i<4;i++)                       
                            resp_data_1[4-i] = pv_754[i] = *p_pv_band++;

                          hart_resp(resp_data_1, resp_num_1,dev_addr,cmd_num); 
                          break;
                        case 2:/*速关*/
                          pv_qof = (float)VAL.V_p2_5_QOF/10.0;                       
                          p_pv_qof = (unsigned char *)&pv_qof;
                          resp_data_1[0] = indexs;

                          for(i=0;i<4;i++)                       
                            resp_data_1[4-i] = pv_754[i] = *p_pv_qof++;

                          hart_resp(resp_data_1, resp_num_1,dev_addr,cmd_num);   
                          break;
                        case 3:/*温度*/
                          pv_temperature = (float)(AD.Temperature/10.0);                       
                          p_pv_temperature = (unsigned char *)&pv_temperature;
                          resp_data_1[0] = indexs;

                          for(i=0;i<4;i++)                       
                            resp_data_1[4-i] = pv_754[i] = *p_pv_temperature++;

                          hart_resp(resp_data_1, resp_num_1,dev_addr,cmd_num);   
                          break;
                        default:
                          break;
                        }
                        

                        break;
              case 129: /*写入指令*/ 
                        if(byte_count){    //有请求字节                           
                            indexs=cmd_data[0];                                                    
                        }
                        else indexs=0;

                        switch (indexs)
                        {
                        case 1:/*容差*/
                          for(i=0;i<4;i++){
                              rev_float_buf[4-i]=cmd_data[i+1];//大小端转换
                            }
                          memcpy(&pv_band,rev_float_buf,4);
                          VAL.V_p1_3BRND =(int)(pv_band*10);

                          pv_band = (float)VAL.V_p1_3BRND/10.0;                       
                          p_pv_band = (unsigned char *)&pv_band;
                          resp_data_1[0] = indexs;

                          for(i=0;i<4;i++)                       
                            resp_data_1[4-i] = pv_754[i] = *p_pv_band++;

                          hart_resp(resp_data_1, resp_num_1,dev_addr,cmd_num); 
                          break;
                        case 2:/*速关*/
                          for(i=0;i<4;i++){
                              rev_float_buf[4-i]=cmd_data[i+1];//大小端转换
                            }
                          memcpy(&pv_qof,rev_float_buf,4);
                          VAL.V_p2_5_QOF =(int)(pv_qof*10);

                          pv_qof = (float)VAL.V_p2_5_QOF/10.0;                       
                          p_pv_qof = (unsigned char *)&pv_qof;
                          resp_data_1[0] = indexs;

                          for(i=0;i<4;i++)                       
                            resp_data_1[4-i] = pv_754[i] = *p_pv_qof++;

                          hart_resp(resp_data_1, resp_num_1,dev_addr,cmd_num);   
                          break;
                        default:
                          break;
                        }

                        break;
              case 130:
                      if(byte_count){    //有请求字节                           
                            indexs=cmd_data[0];                                                    
                        }
                        else indexs=0;

                        if(0 == VAL.V_p8_2_POS)//故障安全
                        {
                          if(AD.IP_Current_Out<=0.75)type_gas=0;//排气
                          else type_gas=1;
                        }
                        else if(1 == VAL.V_p8_2_POS) //故障闭锁
                        {
                          if(AD.IP_Current_Out<=0)type_gas=0;//排气
                          else type_gas=1;
                        }

                        switch (indexs)
                        {
                          case 1:
                            resp_data_1[0] = indexs;
                            resp_data_1[1] = type_gas;
                            hart_resp(resp_data_1, 2,dev_addr,cmd_num);
                            break;
                          default:break; 
                        }
                        break;
                default: break;
             } 
            
      }        
      CLK.received = 0;
      }
 
}

/*该函数用于从指定的位置开始计算校验和,逐个数据进行异或运算.第一个参数指向要进行校验和计算的
第一个数据,第二个参数是参与计算的数据个数,返回值返回计算的结果.*/
unsigned char Checksum(unsigned char *s , unsigned char nbytes)
{
     unsigned char i,check;
     unsigned char *p;

     for (i=1, check=0, p=s; i<=nbytes; ++i) check ^= *p++;

     return(check);
}



/*通过串口发送数据,第一个参数指向需要发送数据的首地址,第二个参数说明需要发送数据的字节数*/

void com_send(unsigned char *send, unsigned char send_bytes)
{
     unsigned char *send_ptr;
     send_ptr = send;
     _EINT();
     _NOP();
    // IE1 &= ~URXIE0;
     P1OUT |=BIT4;  // 开MOS管
     P1OUT &= ~BIT1;
     //delay(10);
     delay_ms(1);
     for(int i=0;i<send_bytes;i++)
     {
       while ((IFG1 & UTXIFG0) == 0); //等待上一个字节发送完毕
       TXBUF0 = *send_ptr++;
     }
     
     delay(100);
     P1OUT &=~BIT4;  // 关MOS管
     P1OUT |= BIT1;
    // IE1 |= URXIE0;
     
}

/*该函数用于按HART协议的格式生成待发送的数据帧send[].第一个参数为指向响应数据区首字节的
指针,第二个参数为响应数据的字节数,第三个参数是指向本从机长结构地址,第四个参数是命令号*/
void hart_resp( unsigned char *resp_data, unsigned char resp_num, unsigned char *addr ,unsigned char cmd_num)
{
     unsigned char i,j;
     unsigned char *r_d;       
     unsigned char  *a;    
            
     r_d = resp_data ;
     a = addr;
     pre_num =0x07;   //前导符数
  
     for ( i=0; i<pre_num; ++i)  //pre_num是接收到数据帧中前导符的个数,响应时前导府数同主机发送给从机数据帧中前导符个数相同
     send[i] = 0xFF;
     switch(x1)  //判断是长结构还是短结构
     {
       case 0: return;
       case 1: send[i++] = 0x06;   //短结构时
               send[i++] = short_addr;  //使用短地址
               break;
       case 2: send[i++] = 0x86;   //长结构时           
               for(j=0; j<5; ++j) send[i++] = *a++; //使用长地址
               break;
      }
      send[i++] = cmd_num; //填充命令号
      send[i++] = resp_num + 2;//填充字节数
      send[i++] = status1 ; //填充状态位1
      send[i++] = status2 ; //填充状态位2
      for(j=1 ; j<=resp_num; ++j) send[i++] = *r_d++;
      send[i] = Checksum((send + pre_num), i-pre_num); //填充校验字节
      send_bytes = i+1;  //计算数据帧的总长度
      
      com_send(send,send_bytes);
      memset(send,0,100);
}
  



//



void HartMessage (void)
{
      rec_bytes = rec_ptr;       //接收的字节数
      rec_ptr = 0;               //清零以准备下一次数据帧的接收
      CLK.received = 2;              //程序运行到这里表示已接收到一个完整的数据帧,将received置位
      CLK.Hart_Flag =0;
}

/*关于用串口接收主机发来的数据帧,串口只能一个字节一个字节地接收数据,利用定时器定时30ms,当相邻的数据字节接收间隔时间不超过30ms时,便认定
二者是同一数据帧当中的数据,当超过30ms仍然没接收到下一个字节时,便认为数据帧接收已经结束.*/
#pragma vector = UART0RX_VECTOR
__interrupt void UART0 (void)
{       
  receive[rec_ptr++] = RXBUF0; // rec_ptr初始值为0,每接受一个数据便自增1
  if(receive[0]==0xff && receive[1]==0xff)  
    CLK.received = 1;
  else CLK.received = 0;
  
   CLK.Hart_Flag =1;

}
/*
#pragma vector = UART0TX_VECTOR
__interrupt void UART0_TXISR(void)
{
      if(Tindex < SendLen)
      {
          TXBUF0 = *(SDat+Tindex);
          Tindex++;
      }
      else
      {
          Tindex=0;
          //LPM1;
      }
}
*/

