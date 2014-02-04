
#define	CTRL_NONE	0x0000
#define CTRL_TEMP	0x0001
#define CTRL_RPM	0x0002
#define	CTRL_HUMI	0x0004
#define	CTRL_LUMI	0x0008

#define SUB_CTRL_TEMP	1
#define SUB_CTRL_RPM	2
#define SUB_CTRL_HUMI	3
#define SUB_CTRL_LUMI	4

struct	_control_spec
{
	int   subtype;
	int	  min;
	int   max;
	float display_scale;
	float setting_scale;
	float offset;
};

struct _spec  
{
	unsigned int id;
	unsigned char name[30];
	unsigned char model_type[10];
	unsigned short control_type;
	struct _control_spec control_spec[4];

};


struct _spec spec_data[] =
{
//  				      id   name	       model_type	control_type                         subtype       min  max  disp_scale set_scale	offset
	/* #04ID01 */	{      1, "WiseClave",	"WAC",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 25, 132, 0.1, 	    0.1, 		10.0},{},{},{} } },
	/* #04ID02 */	{      2, "WiseBath",	"WB",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 15, 120, 0.1, 	    0.1, 		10.0},{},{},{} } },
	/* #04ID03 */	{      3, "WiseBath",	"WHB",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 15, 250, 0.1, 	    0.1, 		30.0},{},{},{} } },
	/* #04ID04 */	{      4, "WiseCircu",	"WCB",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 15, 120, 0.1, 	    0.1, 		10.0},{},{},{} } },
	/* #04ID05 */	{      5, "WiseCircu",	"WCR",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	-20, 100, 0.1, 	    0.1, 		10.0},{},{},{} } },
	/* #04ID06 */	{      6, "WiseCircu",	"WCL",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	-40, 100, 0.1, 	    0.1, 		10.0},{},{},{} } },

	/* #04ID07 */	{      7, "WiseBath",	"WSB",	    CTRL_TEMP|CTRL_RPM, 	       { {SUB_CTRL_TEMP,	 20, 100, 0.1, 	    0.1, 		10.0},
																				         {SUB_CTRL_RPM ,     20, 250, 1.0,      1.0,            },   {},{} } },

	/* #04ID08 */	{      8, "WiseBath",	"WVB",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 15, 120, 0.1, 	    0.1, 		10.0},{},{},{} } },
	/* #04ID09 */	{      9, "WiseCube",	"WIG",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 25,  70, 0.1, 	    0.1, 		10.0},{},{},{} } },
	/* #04ID0A */	{   0x0a, "WiseCube",	"WIR",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	  0,  60, 0.1, 	    0.1, 		10.0},{},{},{} } },

	// ID는 같은데 model_type만 다르게 되어 있어서 합침
	/* #04ID0B */	{   0x0b, "WiseCube",	"WIS(-R)",	CTRL_TEMP|CTRL_RPM, 	       { {SUB_CTRL_TEMP,	 10,  60, 0.1, 	    0.1, 		10.0},
																				         {SUB_CTRL_RPM ,     30, 250, 1.0,      1.0,            },   {},{} } },

	/* #04ID0C */	{   0x0c, "WiseCube",	"WGC",	    CTRL_TEMP|CTRL_HUMI|CTRL_LUMI, { {SUB_CTRL_TEMP,	 10,  60, 0.1, 	    0.1, 		10.0},
																						 {},
																				         {SUB_CTRL_HUMI ,    30,  95, 1.0,      1.0,        10.0},
																				         {SUB_CTRL_LUMI ,     0,   7, 1.0,      1.0,            },         } },

	/* #04ID0D */	{   0x0d, "WiseCube",	"WTH",	    CTRL_TEMP|CTRL_HUMI, 	       { {SUB_CTRL_TEMP,	-20, 100, 0.1, 	    0.1, 		10.0},
																						 {},
																				         {SUB_CTRL_HUMI ,    30,  98, 1.0,      1.0,        10.0},      {} } },
	/* #04ID0E */	{   0x0e, "WiseCube",	"WTH-L",	CTRL_TEMP|CTRL_HUMI, 	       { {SUB_CTRL_TEMP,	-40, 100, 0.1, 	    0.1, 		10.0},
																						 {},
																				         {SUB_CTRL_HUMI ,    30,  98, 1.0,      1.0,        10.0},      {} } },

	/* #04ID0F */	{   0x0f, "WiseVen",	"WON",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 25, 230, 1.0, 	    1.0, 		20.0},{},{},{} } },
	/* #04ID10 */	{   0x10, "WiseVen",	"WOF",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 25, 250, 1.0, 	    1.0, 		20.0},{},{},{} } },
	/* #04ID11 */	{   0x11, "WiseVen",	"WOC",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 25, 250, 1.0, 	    1.0, 		20.0},{},{},{} } },
	/* #04ID12 */	{   0x12, "WiseVen",	"WOV",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 25, 200, 1.0, 	    1.0, 		20.0},{},{},{} } },
	/* #04ID13 */	{   0x13, "WiseVen",	"WOF-L",	CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 25, 200, 1.0, 	    1.0, 		20.0},{},{},{} } },
	/* #04ID14 */	{   0x14, "WiseVen",	"WCC",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	  0,  10, 1.0, 	    1.0, 		10.0},{},{},{} } },

	/* #04ID15 */	{   0x15, "WiseStir",	"MSH",	    CTRL_TEMP|CTRL_RPM, 	       { {SUB_CTRL_TEMP,	 25, 380, 0.1, 	    0.5, 		20.0},
																				         {SUB_CTRL_RPM ,     80,1500, 1.0,      5.0,            },   {},{} } },

	/* #04ID21 */	{   0x21, "WiseTherm",	"HP",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 25, 380, 1.0, 	    0.5, 		20.0},{},{},{} } },
	/* #04ID22 */	{   0x22, "WiseTherm",	"HPLP",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 25, 380, 1.0, 	    0.5, 		20.0},{},{},{} } },

	/* #04ID30 */	{   0x30, "WiseStir",	"HS",	    CTRL_RPM, 				       { {},
																						 {SUB_CTRL_RPM,	    200,3000, 1.0, 	    5.0,            },   {},{} } },
	/* #04ID31 */	{   0x31, "WiseStir",	"HT",	    CTRL_RPM, 				       { {},
																						 {SUB_CTRL_RPM,	     50,1000, 1.0, 	    1.0,            },   {},{} } },

	/* #04ID54 */	{   0x54, "WiseCryo",	"WUF",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	-86, -65, 0.1, 	    0.1, 		15.0},{},{},{} } },
	/* #04ID55 */	{   0x55, "WiseCryo",	"WUF-D",	CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	-95, -65, 0.1, 	    0.1, 		15.0},{},{},{} } },
	/* #04ID60 */	{   0x60, "WiseCube",	"WIF",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	 25,  70, 0.1, 	    0.1, 		10.0},{},{},{} } },
	/* #04ID61 */	{   0x61, "WiseTherm",	"F",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	300,1000, 1.0, 	    1.0, 		50.0},{},{},{} } },
	/* #04ID62 */	{   0x62, "WiseTherm",	"FH",	    CTRL_TEMP, 				       { {SUB_CTRL_TEMP,	300,1200, 1.0, 	    1.0, 		50.0},{},{},{} } },

	/* end     */   { 0xffff, }
};


