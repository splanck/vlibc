#include "time.h"
#include "stdio.h"

static int is_leap(int year)
{
    if ((year % 4) != 0)
        return 0;
    if ((year % 100) != 0)
        return 1;
    return (year % 400) == 0;
}

static const int days_per_month[2][12] = {
    {31,28,31,30,31,30,31,31,30,31,30,31},
    {31,29,31,30,31,30,31,31,30,31,30,31}
};

static struct tm tm_buf;

struct tm *gmtime(const time_t *timep)
{
    time_t t = timep ? *timep : time(NULL);
    if (t < 0)
        t = 0;

    int sec = t % 60; t /= 60;
    int min = t % 60; t /= 60;
    int hour = t % 24; t /= 24;
    int days = (int)t;

    int wday = (days + 4) % 7; /* 1970-01-01 was Thursday */
    int year = 1970;
    while (1) {
        int ydays = is_leap(year) ? 366 : 365;
        if (days >= ydays) {
            days -= ydays;
            year++;
        } else {
            break;
        }
    }
    int yday = days;
    const int *month_lengths = days_per_month[is_leap(year)];
    int mon = 0;
    while (days >= month_lengths[mon]) {
        days -= month_lengths[mon];
        mon++;
    }
    int mday = days + 1;

    tm_buf.tm_sec = sec;
    tm_buf.tm_min = min;
    tm_buf.tm_hour = hour;
    tm_buf.tm_mday = mday;
    tm_buf.tm_mon = mon;
    tm_buf.tm_year = year - 1900;
    tm_buf.tm_wday = wday;
    tm_buf.tm_yday = yday;
    tm_buf.tm_isdst = 0;
    return &tm_buf;
}

struct tm *localtime(const time_t *timep)
{
    /* no timezone handling, just use gmtime */
    return gmtime(timep);
}

time_t mktime(struct tm *tm)
{
    if (!tm)
        return (time_t)-1;

    int year = tm->tm_year + 1900;
    time_t days = 0;
    for (int y = 1970; y < year; y++)
        days += is_leap(y) ? 366 : 365;

    const int *ml = days_per_month[is_leap(year)];
    for (int m = 0; m < tm->tm_mon; m++)
        days += ml[m];
    days += tm->tm_mday - 1;

    tm->tm_yday = (int)days;
    tm->tm_wday = (int)((days + 4) % 7);
    tm->tm_isdst = 0;

    time_t t = days * 86400 + tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
    return t;
}

time_t timegm(struct tm *tm)
{
    return mktime(tm);
}

char *ctime(const time_t *timep)
{
    static char buf[32];
    struct tm *tm = localtime(timep);
    static const char *wd[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    static const char *mn[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    if (!tm)
        return NULL;
    char dd[3] = {0}, hh[3] = {0}, mm[3] = {0}, ss[3] = {0};
    dd[0] = '0' + (tm->tm_mday / 10);
    dd[1] = '0' + (tm->tm_mday % 10);
    hh[0] = '0' + (tm->tm_hour / 10);
    hh[1] = '0' + (tm->tm_hour % 10);
    mm[0] = '0' + (tm->tm_min / 10);
    mm[1] = '0' + (tm->tm_min % 10);
    ss[0] = '0' + (tm->tm_sec / 10);
    ss[1] = '0' + (tm->tm_sec % 10);
    snprintf(buf, sizeof(buf), "%s %s %s %s:%s:%s %d\n",
             wd[tm->tm_wday], mn[tm->tm_mon], dd, hh, mm, ss,
             tm->tm_year + 1900);
    return buf;
}
