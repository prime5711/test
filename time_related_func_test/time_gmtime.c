
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
    int       i;
    time_t  the_time;
    struct    tm *tm_ptr;

    time(&the_time);
    tm_ptr = gmtime(&the_time);
    printf("현재시간 : %d년 %d월 %d일 %d:%d\n",
            tm_ptr->tm_year + 1900, tm_ptr->tm_mon +1,
            tm_ptr->tm_mday, tm_ptr->tm_hour,
            tm_ptr->tm_min);
}

