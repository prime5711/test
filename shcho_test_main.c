#include <stdio.h>
#include "device_spec.h"

int main(void)
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
