
#define DTC_KCAT(a,b,c) a##b##c

#define DTC_FNAME(a,b,c) DTC_KCAT_(a,b,c)

DTCLIB_TYPE DTC_FNAME(dtc_, DTCLIB_PREFIX, _parse)(char html[static 1])
{
	
}
