#define INIT_YEAR 2012

#define nSecondsInJan 2678400L
#define nSecondsInApril 2592000L
#define nSecondsInLeapFeb 2505600L
#define nSecondsInNonLeapFeb 2419200L
const nSecondsInNonLeapYear=31536000L;
const nSecondsInLeapYear=31622400L;
const long int nSecondsInMonths_NonLeap[] ={
	0,nSecondsInJan, nSecondsInNonLeapFeb, nSecondsInJan, nSecondsInApril,nSecondsInJan, nSecondsInApril, nSecondsInJan, nSecondsInJan,nSecondsInApril, nSecondsInJan, nSecondsInApril, nSecondsInJan};
const long int nSecondsInMonths_Leap[] ={
	0,nSecondsInJan, nSecondsInLeapFeb, nSecondsInJan, nSecondsInApril,nSecondsInJan, nSecondsInApril, nSecondsInJan, nSecondsInJan,nSecondsInApril, nSecondsInJan, nSecondsInApril, nSecondsInJan};

typedef struct{
	short int year;//INIT_YEAR-INIT_YEAR+67
	short int month;//1-12
	short int day;//1-31
	short int hour;
	short int minute;
	short int second;
} time;
typedef enum{
	DATE_AND_TIME,TIME,COUNTDOWN
} TIMEFORMAT;

int isleapyear(int n)
{
	if(n%100==0) return (n%400==0); else return (n%4==0);
}
long int getsecond(time t)
{
	int year,month;
	long int n = 0;
	for (year = INIT_YEAR; year < t.year; year++){
		if (isleapyear(year)) n+=nSecondsInLeapYear;
		else n+=nSecondsInNonLeapYear;
	}
	for (month = 1; month < t.month; month++){
		if (isleapyear(t.year)) n += nSecondsInMonths_Leap[month];
		else n += nSecondsInMonths_NonLeap[month];
	}
	n += (t.day - 1) * 86400;
	n+= t.hour * 3600;
	n += t.minute * 60;
	n += t.second;
	return n;
}
void gettime(time *buff,long int s)
{
	int year,month,n;
	year=INIT_YEAR;
	n=isleapyear(year)?nSecondsInLeapYear:nSecondsInNonLeapYear;
	while(s>=n){
		s-=n;
		year++;
		n=isleapyear(year)?nSecondsInLeapYear:nSecondsInNonLeapYear;
	}
	month=1;
	n=isleapyear(year)?nSecondsInMonths_Leap[month]:nSecondsInMonths_NonLeap[month];
	while(s>=n){
		s-=n;
		month++;
		n=isleapyear(year)?nSecondsInMonths_Leap[month]:nSecondsInMonths_NonLeap[month];
	}
	buff->year=year;
	buff->month=month;
	buff->day=s/86400+1;s=s%86400;
	buff->hour=s/3600;s=s%3600;
	buff->minute=s/60;s=s%60;
	buff->second=s;
}
void getcountdown(time *buff,long int s)
{
	buff->year=s/nSecondsInNonLeapYear;s=s%nSecondsInNonLeapYear;
	buff->month=s/nSecondsInApril;s=s%nSecondsInApril;
	buff->day=s/86400+1;s=s%86400;
	buff->hour=s/3600;s=s%3600;
	buff->minute=s/60;s=s%60;
	buff->second=s;
}
int writetime(char *buff,time t,TIMEFORMAT f)
{
	switch(f){
	case DATE_AND_TIME:
		strprintf(buff,"%04d.%02d.%02d %02d:%02d:%02d",t.year,t.month,t.day,t.hour,t.minute,t.second);
	return 19;
	case TIME:
		strprintf(buff,"%02d:%02d:%02d",t.hour,t.minute,t.second);
	return 8;
	case COUNTDOWN:
		if(t.year>0){strprintf(buff,"%04dy%02dm%02dd %02d:%02d:%02d",t.year,t.month,t.day,t.hour,t.minute,t.second);return 20;}
		if(t.month>0){strprintf(buff,"%02dm%02dd %02d:%02d:%02d",t.month,t.day,t.hour,t.minute,t.second);return 15;}
		if(t.day>0){strprintf(buff,"%02dd %02d:%02d:%02d",t.day,t.hour,t.minute,t.second);return 12;}
		if(t.hour>0){strprintf(buff,"%02d:%02d:%02d",t.hour,t.minute,t.second);return 8;}
		if(t.minute>0 || t.second>0){strprintf(buff,"%02d:%02d",t.minute,t.second);return 5;}
		strprintf(buff,"--:--");return 5;
	}
}
void readtime(time *buff)
{
	buff->year=RTCReadYear();
	buff->month=RTCReadMonth();
	buff->day=RTCReadDayOfMonth();
	buff->hour=RTCReadHour();
	buff->minute=RTCReadMinute();
	buff->second=RTCReadSecond();
}
