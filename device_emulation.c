#include <stdio.h>

//  #include <sys/select.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <signal.h>

// shcho for select
// #include <stdio.h>
// #include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
// #include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define DEBUG
#ifdef DEBUG
#define INNO_SHCHO_PRINT(fmt, args...)	fprintf(stdout, fmt, ## args)
#else
#define INNO_SHCHO_PRINT(fmt, args...)	
#endif


#include "device_spec.h"


struct _device_stat
{
	int temp_sp;
	int humi_sp;
	int temp_pv;
	int humi_pv;
	int temp_offset;
	int humi_offset;
	int rpm_pv;
	int rpm_sp;
	int operation_timer_sv_minute;
	int operation_timer_pv_minute;
	int delay_timer_sv_minute;
	int delay_timer_pv_minute;
	int illumination;
	int out_of_control_limit_high;
	int out_of_control_limit_low;
	int power_saving_day;
}__attribute__((packed));

struct _device_operation_stat
{
	char Operation;
	char Heating;
	char Compressor1;
	char Compressor2;
	char Humid_Gen;
	char Revolution;
	char Buzzer_stat_On_Off;
	char Operation_Timer;
	char Delay_Timer;
	char LCO2_Switch;
	char Buzzer_Operation_On_Off;
	char Power_Saving_Operation;
}__attribute__((packed));


#define BAUDRATE	B19200
//  #define BAUDRATE 	B38400

int select_ret;
fd_set          rfds;
struct timeval  tv;
struct timespec  ts;

int device_id = -1;
int num_selected_row ;
int bool_old_new = -1; // old : 0, new = 1 
sigset_t	sigs;

unsigned char *model_name ; 
unsigned char *model_type ; 


#define ACT_AS_OLD		0
#define ACT_AS_NEW		1

#define YES		1
#define NO		0

#define STRING_MATCH	0

int loop_stop = -1;

unsigned char   uart_buf[255];
unsigned char   stdin_buf[255];
int uart_fd;
int stat_file_fd;

int device_spec_info(void) ;
void Processing_UartPacket(unsigned char *buf);
unsigned char* find_model_name( int id );
unsigned char* find_model_type( int id );

#define LCO2_SW_OPEN	1
#define LCO2_SW_CLOSE	0

struct _device_error 
{
	char Temp_Error;
	char Temp_Sensor_Error;
	char Door_Open_Error;
	char Out_Of_Control_Error;
	char Water_Level_Error;
	char Compressor1_Error;
	char Compressor2_Error;
	char Main_Power_Off_Error;
	char Humidity_Sensor_Error;
	char Revolution_Error;
};
struct _device_stat device_stat =
{
	.temp_sp					=  8000, // 80.00도
	.temp_pv					=  6000, // 60.00도
	.humi_sp					=   601, // 60.1%
	.humi_pv					=   602, // 60.2%
	.temp_offset				=  1600, // 16.00도
	.humi_offset				=    50, //  5%
	.rpm_pv						=   300, // 300rpm
	.rpm_sp						=   320, // 320rpm
	.operation_timer_sv_minute  =   100, // minute 100 (1h 40m)
	.operation_timer_pv_minute  =    80, // minute  80 (1h 20m)
	.delay_timer_sv_minute      =    30, // minute  30 (   30m)
	.delay_timer_pv_minute      =    20, // minute  20 (   20m)
	.illumination				=     0,
	.out_of_control_limit_high	= 20000, // 200.00도
	.out_of_control_limit_low	= -5000, // -50.00도
	.power_saving_day			=     0,
};

struct _device_error device_error_stat =
{
	.Temp_Error=0,
	.Temp_Sensor_Error=0,
	.Door_Open_Error=0,
	.Out_Of_Control_Error=0,
	.Water_Level_Error=0,
	.Compressor1_Error=0,
	.Compressor2_Error=0,
	.Main_Power_Off_Error=0,
	.Humidity_Sensor_Error=0,
	.Revolution_Error=0,
};

struct _device_operation_stat device_operation_stat =
{
	.Operation		= 1,
	.Heating		= 1,
	.Compressor1	= 1,
	.Compressor2	= 1,
	.Humid_Gen		= 1,
	.Revolution		= 1,
	.Buzzer_stat_On_Off	= 0,
	.Operation_Timer= 1,
	.Delay_Timer	= 1,
	.Buzzer_Operation_On_Off	= 0,
	.LCO2_Switch	= 0,
	.Power_Saving_Operation= 0,
};

struct timeval power_up_time_timeval;
struct timeval current_time_timeval;

time_t power_up_time_sec;
time_t current_time_sec;

time_t the_time;
struct tm *pTime_tm;
struct tm Power_up_time_tm;
struct tm Ccurrent_up_time_tm;



void Processing_UartPacket(unsigned char *buf)
{
	static unsigned int id_seq_num=0;
	char tmp_buf[255];
	char tmp_b[20];
	char *end_ptr;

	char sign;
	unsigned int value;

//  	INNO_SHCHO_PRINT("\n\n\t \033[22;30;31m buf=%s line:%d:@%s in %s                \033[0m \n\n",buf, __LINE__,__FUNCTION__,__FILE__  ); 
	printf("\nP:%s-->",buf);

	if( strncasecmp("#04TEST", (char *)buf, 7) == STRING_MATCH )
	{
		if( bool_old_new == ACT_AS_OLD )
		{
			printf(" Act as old version : No Response \n");
		}
		else
		{
			fprintf(stderr,"Send to Uart :#03Err\n");
			write(uart_fd, "#03ERR", 6);
		}
	}
	else if( strncmp("#02id", (char *)buf, 5) == STRING_MATCH )
	{
		memset(tmp_buf, 0, sizeof(tmp_buf));
		sprintf(tmp_buf, "#04id%02x",device_id);

		printf("%s\n", tmp_buf);
		fprintf(stderr,"Send to Uart :%s(seq=%d)\n", tmp_buf, id_seq_num++);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#02ts", (char *)buf, 5) == STRING_MATCH )
	{
		memset(tmp_buf, 0, sizeof(tmp_buf));

		sign  = (device_stat.temp_sp > 0.0)?0:1;
		value = (device_stat.temp_sp);

		if( sign == 1 ) { value *= -1; }

		sprintf(tmp_buf, "#08ts%1d%05d",sign,value);

		printf("%s\n", tmp_buf);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#02tp", (char *)buf, 5) == STRING_MATCH )
	{
		memset(tmp_buf, 0, sizeof(tmp_buf));

		sign  = (device_stat.temp_pv > 0.0)?0:1;
		value = (device_stat.temp_pv);

		if( sign == 1 ) { value *= -1; }

		sprintf(tmp_buf, "#08tp%1d%05d",sign,value);

		printf("%s\n", tmp_buf);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#02to", (char *)buf, 5) == STRING_MATCH )
	{
//  		INNO_SHCHO_PRINT("\n\n\t \033[22;35m line:%d:@%s in %s \033[0m \n",__LINE__,__FUNCTION__,__FILE__  ); 
		memset(tmp_buf, 0, sizeof(tmp_buf));

		sign  = (device_stat.temp_offset > 0.0)?0:1;
		value = (device_stat.temp_offset);

		if( sign == 1 ) { value *= -1; }

		sprintf(tmp_buf, "#07to%1d%04d",sign,value);

		printf("%s\n", tmp_buf);
//  		INNO_SHCHO_PRINT("\n\n\t \033[22;35m tmp_buf=%s line:%d:@%s in %s \033[0m \n",tmp_buf,__LINE__,__FUNCTION__,__FILE__  ); 
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#02ho", (char *)buf, 5) == STRING_MATCH )
	{
		memset(tmp_buf, 0, sizeof(tmp_buf));

		sign  = (device_stat.humi_offset > 0.0)?0:1;
		value = (device_stat.humi_offset);

		if( sign == 1 ) { value *= -1; }

		sprintf(tmp_buf, "#06ho%1d%03d",sign,value);

		printf("%s\n", tmp_buf);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#02hs", (char *)buf, 5) == STRING_MATCH )
	{
		memset(tmp_buf, 0, sizeof(tmp_buf));

		value = (int)(device_stat.humi_sp);

		sprintf(tmp_buf, "#05hs%03d",value);

		printf("%s\n", tmp_buf);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#02hp", (char *)buf, 5) == STRING_MATCH )
	{
		memset(tmp_buf, 0, sizeof(tmp_buf));

		value = (int)(device_stat.humi_pv);

		sprintf(tmp_buf, "#05hp%03d",value);

		printf("%s\n", tmp_buf);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}


	else if( strncmp("#03ots", (char *)buf, 6) == STRING_MATCH )
	{
		memset(tmp_buf, 0, sizeof(tmp_buf));

		value = (int)(device_stat.operation_timer_sv_minute);
		time(&current_time_sec);
//  		value += ((current_time_sec - power_up_time_sec)/60);


		sprintf(tmp_buf, "#07ots%04d",value);

		printf("%s\n", tmp_buf);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#03otp", (char *)buf, 6) == STRING_MATCH )
	{
		memset(tmp_buf, 0, sizeof(tmp_buf));

		value = (int)(device_stat.operation_timer_pv_minute);
		time(&current_time_sec);
		value += ((current_time_sec - power_up_time_sec)/60);


		sprintf(tmp_buf, "#07otp%04d",value);

		printf("%s\n", tmp_buf);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#03dts", (char *)buf, 6) == STRING_MATCH )
	{
		memset(tmp_buf, 0, sizeof(tmp_buf));

		value = (int)(device_stat.delay_timer_sv_minute);
		time(&current_time_sec);
//  		value += ((current_time_sec - power_up_time_sec)/60);


		sprintf(tmp_buf, "#07dts%04d",value);

		printf("%s\n", tmp_buf);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#03dtp", (char *)buf, 6) == STRING_MATCH )
	{
		memset(tmp_buf, 0, sizeof(tmp_buf));

		value = (int)(device_stat.delay_timer_pv_minute);
		time(&current_time_sec);
		value -= ((current_time_sec - power_up_time_sec)/60);


		sprintf(tmp_buf, "#07dtp%04d",value);

		printf("%s\n", tmp_buf);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#02OA", (char *)buf, 5) == STRING_MATCH )
	{
		memset(tmp_buf, 0, sizeof(tmp_buf));

		value = (int)(device_stat.delay_timer_pv_minute);

		sprintf(tmp_buf, "#11OA%d%d%d%d%d%d%d%d%d",
							device_operation_stat.Operation,
							device_operation_stat.Heating,
							device_operation_stat.Compressor1,
							device_operation_stat.Compressor2,
							device_operation_stat.Humid_Gen,
							device_operation_stat.Revolution,
							device_operation_stat.Buzzer_Operation_On_Off,
							device_operation_stat.Operation_Timer,
							device_operation_stat.Delay_Timer);

		printf("%s\n", tmp_buf);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#08TS", (char *)buf, 5) == STRING_MATCH )
	{
		sign = buf[5];
		value = (int)strtol((const char*)&buf[6], (char**)&end_ptr, 10);
		if( sign == '1' ) value *= -1;
		device_stat.temp_sp = value;
		printf("Temp SP setting\n");
	}

	else if( strncmp("#08Ts", (char *)buf, 5) == STRING_MATCH )
	{
		sign = buf[5];
		value = (int)strtol((const char*)&buf[6], (char**)&end_ptr, 10);
		if( sign == '1' ) value *= -1;
		device_stat.temp_sp = value;
		printf("현재 제어 종료하고 Temp SP update하고 Temp control operation 다시 시작\n");
	}

	else if( strncmp("#08HS", (char *)buf, 5) == STRING_MATCH )
	{
		value = (int)strtol((const char*)&buf[5], (char**)&end_ptr, 10);
		device_stat.humi_sp = value;
		printf("Humi SP Setting \n");
	}

	else if( strncmp("#08HS", (char *)buf, 5) == STRING_MATCH )
	{
		value = (int)strtol((const char*)&buf[5], (char**)&end_ptr, 10);
		device_stat.humi_sp = value;
		printf("현재 제어 종료하고 Humi SP update하고 Humi control operation 다시 시작\n");
	}

	else if( strncmp("#08HR", (char *)buf, 5) == STRING_MATCH )
	{
		value = (int)strtol((const char*)&buf[5], (char**)&end_ptr, 10);
		device_stat.rpm_sp = value;
		printf("RPM SP Setting \n");
	}

	else if( strncmp("#08Hr", (char *)buf, 5) == STRING_MATCH )
	{
		value = (int)strtol((const char*)&buf[5], (char**)&end_ptr, 10);
		device_stat.rpm_sp = value;
		printf("현재 제어 종료하고 RPM SP update하고 RPM control operation 다시 시작\n");
	}

	else if( strncmp("#07TO", (char *)buf, 5) == STRING_MATCH )
	{
		sign = buf[5];
		value = (int)strtol((const char*)&buf[6], (char**)&end_ptr, 10);
		if( sign == '1' ) value *= -1;
		device_stat.temp_offset = value;
		printf("Temp Offset Setting \n");
	}
	else if( strncmp("#07HO", (char *)buf, 5) == STRING_MATCH )
	{
		value = (int)strtol((const char*)&buf[5], (char**)&end_ptr, 10);
		device_stat.humi_offset = value;
		printf("Humi Offset Setting \n");
	}
	else if( strncmp("#03IL", (char *)buf, 5) == STRING_MATCH )
	{
		value = (int)strtol((const char*)&buf[5], (char**)&end_ptr, 10);
		device_stat.illumination = value;
		printf("Illumination Setting \n");
	}
	else if( strncmp("#06Ot", (char *)buf, 5) == STRING_MATCH )
	{
		value = (int)strtol((const char*)&buf[5], (char**)&end_ptr, 10);
		device_stat.operation_timer_sv_minute = value;
		printf("Operation Timer SV Setting \n");
	}
	else if( strncmp("#06Dt", (char *)buf, 5) == STRING_MATCH )
	{
		value = (int)strtol((const char*)&buf[5], (char**)&end_ptr, 10);
		device_stat.delay_timer_sv_minute = value;
		printf("Delay Timer SV Setting \n");
	}

	else if( strncmp("#02OP" , (char *)buf, 5) == STRING_MATCH ) { device_operation_stat.Operation       = 1; printf("Operation 시작\n"); }
	else if( strncmp("#02OQ" , (char *)buf, 5) == STRING_MATCH ) { device_operation_stat.Operation       = 0; printf("Operation 중지\n"); }
	else if( strncmp("#02DTS", (char *)buf, 6) == STRING_MATCH ) { device_operation_stat.Delay_Timer     = 1; printf("Delay Timer 시작\n"); }
	else if( strncmp("#02DTQ", (char *)buf, 6) == STRING_MATCH ) { device_operation_stat.Delay_Timer     = 0; printf("Delay Timer 중지\n"); }
	else if( strncmp("#02OTS", (char *)buf, 6) == STRING_MATCH ) { device_operation_stat.Operation_Timer = 1; printf("Operation Timer 시작\n"); }
	else if( strncmp("#02OTQ", (char *)buf, 6) == STRING_MATCH ) { device_operation_stat.Operation_Timer = 0; printf("Operation Timer 중지\n"); }
	else if( strncmp("#02LO" , (char *)buf, 5) == STRING_MATCH ) { device_operation_stat.LCO2_Switch     = LCO2_SW_OPEN ; printf("LCO2 Switch Open\n"); }
	else if( strncmp("#02LC" , (char *)buf, 5) == STRING_MATCH ) { device_operation_stat.LCO2_Switch     = LCO2_SW_CLOSE ; printf("LCO2 Switch Close\n"); }
	else if( strncmp("#02BP" , (char *)buf, 5) == STRING_MATCH ) { device_operation_stat.Buzzer_Operation_On_Off     = 1 ; printf("Buzzer Operation On \n"); }
	else if( strncmp("#02BQ" , (char *)buf, 5) == STRING_MATCH ) { device_operation_stat.Buzzer_Operation_On_Off     = 0 ; printf("Buzzer Operation Off \n"); }

	else if( strncmp("#13OU", (char *)buf, 5) == STRING_MATCH )
	{
		sign = buf[5];
		memset(tmp_b,0,sizeof(tmp_b));
		memcpy(tmp_b, &buf[6],4);
		value = (int)strtol((const char*)tmp_b, (char**)&end_ptr, 10);
		if( sign == '1' ) value *= -1;
		device_stat.out_of_control_limit_high = value;

		sign = buf[11];
		memset(tmp_b,0,sizeof(tmp_b));
		memcpy(tmp_b, &buf[12],4);
		value = (int)strtol((const char*)tmp_b, (char**)&end_ptr, 10);
		if( sign == '1' ) value *= -1;
		device_stat.out_of_control_limit_low = value;

		printf("out_of_control_limit(%.2f도 ~%.2f도)\n",device_stat.out_of_control_limit_low/100.0, device_stat.out_of_control_limit_high/100.0);
	}
	else if( strncmp("#02ou", (char *)buf, 5) == STRING_MATCH )
	{
		char sign_high, sign_low;
		int value_high, value_low;
		sign_high = (device_stat.out_of_control_limit_high>0)?0:1;
		sign_low  = (device_stat.out_of_control_limit_low >0)?0:1;

		value_high = device_stat.out_of_control_limit_high;
		value_low  = device_stat.out_of_control_limit_low;

		if( sign_high == 1) { value_high *= -1 ; }
		if( sign_low  == 1) { value_low  *= -1 ; }


		sprintf(tmp_buf,"#13ou%1d%04d %1d%04d",sign_high,value_high, sign_low, value_low);

		printf("%s\n", tmp_buf);
		printf("out_of_control_limit(%.2f도 ~%.2f도)\n",device_stat.out_of_control_limit_low/100.0, device_stat.out_of_control_limit_high/100.0);

		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

	else if( strncmp("#04EO", (char *)buf, 5) == STRING_MATCH )
	{
		value = (int)strtol((const char*)&buf[5], (char**)&end_ptr, 10);
		device_stat.power_saving_day = value;
		device_operation_stat.Power_Saving_Operation = 1;

		printf("power_saving_day=%d(day)\n",value);
		printf("Power_Saving_Operation : Automatically 1\n");
	}
	else if( strncmp("#04EX00", (char *)buf, 7) == STRING_MATCH )
	{
		device_stat.power_saving_day = 0;
		device_operation_stat.Power_Saving_Operation = 0;

		printf("power_saving_day=0(day):Automatically\n");
		printf("Power_Saving_Operation=0\n");
	}

//  	else if( strncmp("#04save", (char *)buf, 7) == STRING_MATCH )
//  	{
//  	}
	else if( strncmp("#02ER", (char *)buf, 5) == STRING_MATCH )
	{
			//         1 2 3 4 5 6 7 8 9 10
		sprintf(tmp_buf,"#12ER%d%d%d%d%d%d%d%d%d%d", device_error_stat.Temp_Error,
											         device_error_stat.Temp_Sensor_Error,
										 	         device_error_stat.Door_Open_Error,
										 	         device_error_stat.Out_Of_Control_Error,
										 	         device_error_stat.Water_Level_Error,
										 	         device_error_stat.Compressor1_Error,
										 	         device_error_stat.Compressor2_Error,
										 	         device_error_stat.Main_Power_Off_Error,
										 	         device_error_stat.Humidity_Sensor_Error,
										 	         device_error_stat.Revolution_Error      );

		printf("%s\n", tmp_buf);
		write(uart_fd, tmp_buf, strlen(tmp_buf));
	}

// sgcho : this routine is moved to stdin control
//  	else if( strncmp("#12ES", (char *)buf, 5) == STRING_MATCH )
//  	{
//  		device_error_stat.Temp_Error  			=buf[5]-'0';
//  		device_error_stat.Temp_Sensor_Error  	=buf[6]-'0';
//  		device_error_stat.Door_Open_Error  		=buf[7]-'0';
//  		device_error_stat.Out_Of_Control_Error  =buf[8]-'0';
//  		device_error_stat.Water_Level_Error  	=buf[9]-'0';
//  		device_error_stat.Compressor1_Error  	=buf[10]-'0';
//  		device_error_stat.Compressor2_Error  	=buf[11]-'0';
//  		device_error_stat.Main_Power_Off_Error  =buf[12]-'0';
//  		device_error_stat.Humidity_Sensor_Error =buf[13]-'0';
//  		device_error_stat.Revolution_Error  	=buf[14]-'0';
//  
//  		printf("Processing #12ES\n");
//  
//  		if(device_error_stat.Temp_Error 			== 1 ) printf("Set Temp_Error\n");
//  		if(device_error_stat.Temp_Sensor_Error		== 1 ) printf("Set Temp_Sensor_Error\n");
//  		if(device_error_stat.Door_Open_Error 		== 1 ) printf("Set Door_Open_Error\n");
//  		if(device_error_stat.Out_Of_Control_Error 	== 1 ) printf("Set Out_Of_Control_Error\n");
//  		if(device_error_stat.Water_Level_Error 		== 1 ) printf("Set Water_Level_Error\n");
//  		if(device_error_stat.Compressor1_Error 		== 1 ) printf("Set Compressor1_Error\n");
//  		if(device_error_stat.Compressor2_Error 		== 1 ) printf("Set Compressor2_Error\n");
//  		if(device_error_stat.Main_Power_Off_Error 	== 1 ) printf("Set Main_Power_Off_Error\n");
//  		if(device_error_stat.Humidity_Sensor_Error 	== 1 ) printf("Set Humidity_Sensor_Error\n");
//  		if(device_error_stat.Revolution_Error 		== 1 ) printf("Set Revolution_Error\n");
//  
//  	}

	else if( strncmp("#02TC", (char *)buf, 5) == STRING_MATCH )
	{
							// O H R h r L T t
		sprintf(tmp_buf,"#55AV%d%d%d%d%d%d%d%d", 
										device_operation_stat.Operation,
										device_error_stat.Humidity_Sensor_Error,
										device_error_stat.Revolution_Error,
										device_operation_stat.Humid_Gen,
										device_operation_stat.Revolution,
										device_operation_stat.Buzzer_Operation_On_Off,
										device_operation_stat.Operation_Timer,
										device_operation_stat.Delay_Timer);
										///////////////////////
//  		sprintf(&tmp_buf[strlen(tmp_buf)],"%d%d%d%d%d%d%d%d") 

	}

	else if( strncmp("#03ERR", (char *)buf, 6) == STRING_MATCH )
	{
	}
	else
	{
//  		printf("Unknown Protocol \n");
//  		write(uart_fd,"#03ERR",6);
	}

}

struct timeval power_up_time_timeval;
struct timeval current_time_timeval;

time_t power_up_time_sec;
time_t current_time_sec;

time_t the_time;
struct tm *pTime_tm;
struct tm Power_up_time_tm;
struct tm Ccurrent_up_time_tm;


int main(int argc, char *argv[])
{
	int ret;
	struct termios  oldtio,
	                newtio;
	char *end_ptr;
	int packet_len;

	if(argc != 4)
	{
		printf("Usage : ./a.exe /dev/com[0-9][0-9] old|new ID(hex) \n");
		printf("     com1~com99   (소문자로)                 \n");
		printf("     old : #04TEST	: no response \n");
		printf("     ID(hex) : 0f	: id=15       \n");
		return -1;
	}

	device_id = (int)strtol((const char*)argv[3], (char**)&end_ptr, 16);
	
	if( strncmp(argv[2], "old", 3) == STRING_MATCH ) { bool_old_new = ACT_AS_OLD; }
	else                                             { bool_old_new = ACT_AS_NEW; }

	///////////////////////////////////////////////////////////////////////////////
	time(&power_up_time_sec);

	time(&the_time);
	pTime_tm = localtime((const time_t *)&the_time);
	memcpy(&Power_up_time_tm, pTime_tm, sizeof(struct tm));

	printf("\n\n");
    printf("		Power Up Time : %d년 %d월 %d일 %d:%d\n",
            Power_up_time_tm.tm_year + 1900, Power_up_time_tm.tm_mon +1,
            Power_up_time_tm.tm_mday,        Power_up_time_tm.tm_hour,
            Power_up_time_tm.tm_min);
	///////////////////////////////////////////////////////////////////////////////

	uart_fd = open(argv[1], O_RDWR | O_NONBLOCK | O_NDELAY);

	if (uart_fd == -1)
	{
		perror(argv[1]);
		exit(-1);
	} else
		fcntl(uart_fd, F_SETFL, 0);


	INNO_SHCHO_PRINT("\n\n\t \033[22;30;31m uart_fd=%d line:%d:@%s in %s    \033[0m \n\n" ,uart_fd     , __LINE__,__FUNCTION__,__FILE__  ); 
	INNO_SHCHO_PRINT("\n\n\t \033[22;30;31m uart_device=%s line:%d:@%s in %s \033[0m \n"  ,argv[1]     , __LINE__,__FUNCTION__,__FILE__  ); 
	INNO_SHCHO_PRINT("\n\n\t \033[22;30;31m device_id=%02x(hex) name&type=%s.%s line:%d:@%s in %s \033[0m \n",device_id   , find_model_name(device_id), find_model_type(device_id), __LINE__,__FUNCTION__,__FILE__  ); 
	INNO_SHCHO_PRINT("\n\n\t \033[22;30;31m mode = %d        line:%d:@%s in %s \033[0m \n",bool_old_new, __LINE__,__FUNCTION__,__FILE__  ); 

	tcgetattr(uart_fd, &oldtio);		/* save current serial port settings */
	bzero(&newtio, sizeof(newtio));	/* clear struct for new port settings */

	/*
	 * BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed. CRTSCTS : output hardware flow control (only used if the cable has all necessary lines.
	 * See sect. 7 of Serial-HOWTO) CS8 : 8n1 (8bit,no parity,1 stopbit) CLOCAL : local connection, no modem contol CREAD : enable receiving characters
	 */
	// newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_cflag = CS8 | CLOCAL | CREAD;
	// shcho : should be use
	cfsetispeed(&newtio, BAUDRATE);
	cfsetospeed(&newtio, BAUDRATE);

	/*
	 * IGNPAR : ignore bytes with parity errors ICRNL : map CR to NL (otherwise a CR input on the other computer will not terminate input) otherwise make device raw (no
	 * other input processing)
	 */
	// newtio.c_iflag = IGNPAR | ICRNL;
	newtio.c_iflag = IGNPAR | IGNBRK;

	/*
	 * Raw output.
	 */
	// newtio.c_oflag = 0;
	newtio.c_oflag = OFDEL;

	/*
	 * ICANON : enable canonical input disable all echo functionality, and don't send signals to calling program
	 */
	// newtio.c_lflag = ICANON;
	// non-canonical
	newtio.c_lflag = 0;

	/*
	 * initialize all control characters default values can be found in /usr/include/termios.h, and are given in the comments, but we don't need them here
	 */
	newtio.c_cc[VINTR] = 0;		/* Ctrl-c */
	newtio.c_cc[VQUIT] = 0;		/* Ctrl-\ */
	newtio.c_cc[VERASE] = 0;	/* del */
	newtio.c_cc[VKILL] = 0;		/* @ */
	newtio.c_cc[VEOF] = 4;		/* Ctrl-d */
	newtio.c_cc[VTIME] = 0;		/* inter-character timer unused */
	newtio.c_cc[VMIN] = 12;		/* blocking read until 1 character arrives */
	newtio.c_cc[VSWTC] = 0;		/* '\0' */
	newtio.c_cc[VSTART] = 0;	/* Ctrl-q */
	newtio.c_cc[VSTOP] = 0;		/* Ctrl-s */
	newtio.c_cc[VSUSP] = 0;		/* Ctrl-z */
	newtio.c_cc[VEOL] = 0;		/* '\0' */
	newtio.c_cc[VREPRINT] = 0;	/* Ctrl-r */
	newtio.c_cc[VDISCARD] = 0;	/* Ctrl-u */
	newtio.c_cc[VWERASE] = 0;	/* Ctrl-w */
	newtio.c_cc[VLNEXT] = 0;	/* Ctrl-v */
	newtio.c_cc[VEOL2] = 0;		/* '\0' */

	/*
	 * now clean the modem line and activate the settings for the port
	 */


	tcflush(uart_fd, TCIOFLUSH);
	tcsetattr(uart_fd, TCSANOW, &newtio);

//  	sigfillset(&sigs);

	loop_stop = NO;
	while(loop_stop == NO)
	{
//  		write(uart_fd, "#04TEST", 7); 
//  		INNO_SHCHO_PRINT("\n\n\t \033[22;30;31m line:%d:@%s in %s                \033[0m \n\n",__LINE__,__FUNCTION__,__FILE__  ); 

		FD_ZERO(&rfds);
		FD_SET(uart_fd, &rfds); //uart_fd
		FD_SET(0,&rfds); 		// stdin
//  		FD_SET(fileno(stdin),&rfds); 		// stdin

		tv.tv_sec = 5 ;
		tv.tv_usec = 0 ;

//  		ts.tv_sec = 5 ;
//  		ts.tv_nsec = 0 ;

		memset(uart_buf,0,sizeof(uart_buf));

//  		INNO_SHCHO_PRINT("\n\n\t \033[22;35m line:%d:@%s in %s \033[0m \n",__LINE__,__FUNCTION__,__FILE__  ); 
		select_ret = select(uart_fd+1, &rfds, NULL, NULL, &tv);
//  		select_ret = pselect(uart_fd+1, &rfds, NULL, NULL, &ts, &sigs );

//  		fprintf(stderr,"select_ret =0x%08x(%d)\n", select_ret,select_ret);
		if (select_ret == -1)
		{
			fprintf(stderr,"select error\n");
			perror("select()");
			continue;
		}
		else if (select_ret)
		{
//  			fprintf(stderr,"Data is available now.\n");
			// Read from ttyTCC1(uart serial)
			if( FD_ISSET(0,&rfds) )
			{
				memset(stdin_buf, 0 , sizeof(stdin_buf));
//  				ret = read(0,stdin_buf,sizeof(stdin_buf));
				ret = read(0,stdin_buf,3);

				if( stdin_buf[0] != '#' )
				{
					INNO_SHCHO_PRINT("\n\n\t \033[22;35m  stdin input no start with \'#\' continue \033[0m \n"); 
					ret = read(0,stdin_buf,sizeof(stdin_buf)); // just read
					continue; // protocol start from #
				}
				packet_len = (int)strtol((const char*)&stdin_buf[1], (char**)&end_ptr, 10);
				ret = read(0, &stdin_buf[3], packet_len+1); // trailing 0x0a(newline)
//  				INNO_SHCHO_PRINT("\n\n\t \033[22;35m line:%d:@%s in %s \033[0m \n",__LINE__,__FUNCTION__,__FILE__  ); 
				stdin_buf[packet_len+3] = 0 ;

				printf("\nS:%s\n",stdin_buf);
				write(uart_fd, stdin_buf, packet_len+3);

				// shcho
				// this code add because
				// shcho test environment : 1 Serial loop back
				// others : 2 serial :1 serial is Wisd Remote . 1 serial is device_emulation
				if( strncmp("#12ES", (char *)stdin_buf, 5) == STRING_MATCH )
				{
					device_error_stat.Temp_Error  			=stdin_buf[5]-'0';
					device_error_stat.Temp_Sensor_Error  	=stdin_buf[6]-'0';
					device_error_stat.Door_Open_Error  		=stdin_buf[7]-'0';
					device_error_stat.Out_Of_Control_Error  =stdin_buf[8]-'0';
					device_error_stat.Water_Level_Error  	=stdin_buf[9]-'0';
					device_error_stat.Compressor1_Error  	=stdin_buf[10]-'0';
					device_error_stat.Compressor2_Error  	=stdin_buf[11]-'0';
					device_error_stat.Main_Power_Off_Error  =stdin_buf[12]-'0';
					device_error_stat.Humidity_Sensor_Error =stdin_buf[13]-'0';
					device_error_stat.Revolution_Error  	=stdin_buf[14]-'0';
			
					printf("Processing #12ES\n");
			
					if(device_error_stat.Temp_Error 			== 1 ) printf("Set Temp_Error\n");
					if(device_error_stat.Temp_Sensor_Error		== 1 ) printf("Set Temp_Sensor_Error\n");
					if(device_error_stat.Door_Open_Error 		== 1 ) printf("Set Door_Open_Error\n");
					if(device_error_stat.Out_Of_Control_Error 	== 1 ) printf("Set Out_Of_Control_Error\n");
					if(device_error_stat.Water_Level_Error 		== 1 ) printf("Set Water_Level_Error\n");
					if(device_error_stat.Compressor1_Error 		== 1 ) printf("Set Compressor1_Error\n");
					if(device_error_stat.Compressor2_Error 		== 1 ) printf("Set Compressor2_Error\n");
					if(device_error_stat.Main_Power_Off_Error 	== 1 ) printf("Set Main_Power_Off_Error\n");
					if(device_error_stat.Humidity_Sensor_Error 	== 1 ) printf("Set Humidity_Sensor_Error\n");
					if(device_error_stat.Revolution_Error 		== 1 ) printf("Set Revolution_Error\n");
			
				}

//  				usleep(10000);
			}
			if( FD_ISSET(uart_fd,&rfds) )
			{
				ret = read(uart_fd, uart_buf, 3);
				if( uart_buf[0] != '#' )
				{
//  					fprintf(stderr,"0:uart_buf[0]=%02x\n",uart_buf[0]);
					tcflush(uart_fd, TCIOFLUSH);
					continue; // protocol start from #
				}

				packet_len = (int)strtol((const char*)&uart_buf[1], (char**)&end_ptr, 10);
				ret = read(uart_fd, &uart_buf[3], packet_len);
				uart_buf[ret+3] = 0 ; // null Termination

//  				fprintf(stderr,"1:uart_buf=%s\n",uart_buf);
//  				tcflush(uart_fd, TCIOFLUSH);

				Processing_UartPacket(uart_buf);
			}
		} 
		else
		{
//  			fprintf(stderr,"No data within %d seconds.\n",(int)tv.tv_sec);
			continue;
		}
	
//  		sleep(0.5);
	}

	tcsetattr(uart_fd, TCSANOW, &oldtio);
	close(uart_fd);

	return 1 ;
}
















int device_spec_info(void)
{
	int num_spec_data_element ;
	int i ;

	num_spec_data_element = sizeof(spec_data)/sizeof(struct _spec);

	printf("num_spec_data_element=%d\n",num_spec_data_element);

	
	for( i = 0 ; i < num_spec_data_element ; i++ )
	{
		if( spec_data[i].id == 0xffff )
			break;

		printf("=================================================\n");
		printf("           id =%3d;\n", 	spec_data[i].id);
		printf("         name =%s;\n", 		spec_data[i].name);
		printf("   model_type =%s;\n", 		spec_data[i].model_type);
		printf(" control_type =0x%04x;\n", 	spec_data[i].control_type);
		if( (spec_data[i].control_type &  CTRL_TEMP) != 0 )
		{
			printf(" SUB_CTRL_TEMP ---------------------\n");
			printf("          min =%4d;\n", 	spec_data[i].control_spec[0].min);
			printf("          max =%4d;\n", 	spec_data[i].control_spec[0].max);
			printf("display_scale =%4.1f;\n", 	spec_data[i].control_spec[0].display_scale);
			printf("setting_scale =%4.1f;\n", 	spec_data[i].control_spec[0].setting_scale);
			printf("       offset =%4.1f;\n", 	spec_data[i].control_spec[0].offset);
		}
		if( (spec_data[i].control_type &  CTRL_RPM) != 0 )
		{
			printf(" SUB_CTRL_RPM ---------------------\n");
			printf("          min =%4d;\n", 	spec_data[i].control_spec[1].min);
			printf("          max =%4d;\n", 	spec_data[i].control_spec[1].max);
			printf("display_scale =%4.1f;\n", 	spec_data[i].control_spec[1].display_scale);
			printf("setting_scale =%4.1f;\n", 	spec_data[i].control_spec[1].setting_scale);
		}
		if( (spec_data[i].control_type &  CTRL_HUMI) != 0 )
		{
			printf(" SUB_CTRL_HUMI ---------------------\n");
			printf("          min =%4d;\n", 	spec_data[i].control_spec[2].min);
			printf("          max =%4d;\n", 	spec_data[i].control_spec[2].max);
			printf("display_scale =%4.1f;\n", 	spec_data[i].control_spec[2].display_scale);
			printf("setting_scale =%4.1f;\n", 	spec_data[i].control_spec[2].setting_scale);
			printf("       offset =%4.1f;\n", 	spec_data[i].control_spec[0].offset);
		}
		if( (spec_data[i].control_type &  CTRL_LUMI) != 0 )
		{
			printf(" SUB_CTRL_LUMI ---------------------\n");
			printf("          min =%4d;\n", 	spec_data[i].control_spec[2].min);
			printf("          max =%4d;\n", 	spec_data[i].control_spec[2].max);
			printf("display_scale =%4.1f;\n", 	spec_data[i].control_spec[2].display_scale);
			printf("setting_scale =%4.1f;\n", 	spec_data[i].control_spec[2].setting_scale);
		}
		printf("--------------------------------------------------\n");
	}
	return 1;
}


unsigned char* find_model_type( int id )
{
	int num_of_element = 0 ;
	int i = 0 ;

	num_of_element = sizeof(spec_data)/sizeof(struct _spec);

	for( i=0 ; i < num_of_element ; i++ )
	{
		if( spec_data[i].id == id )
		{
			break;
		}
	}

	if( i == num_of_element )
		return (unsigned char *)"NULL";
	else
		return spec_data[i].model_type;

}
unsigned char* find_model_name( int id )
{
	int num_of_element = 0 ;
	int i = 0 ;

	num_of_element = sizeof(spec_data)/sizeof(struct _spec);

	for( i=0 ; i < num_of_element ; i++ )
	{
		if( spec_data[i].id == id )
		{
			break;
		}
	}

	if( i == num_of_element )
		return (unsigned char *)"NULL";
	else
		return spec_data[i].name;

}


