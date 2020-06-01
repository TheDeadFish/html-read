#ifndef _HTTPDOWN_H_
#define _HTTPDOWN_H_

extern const char* httpDown_agent;
void httpDown_init(void);
void httpDown_deInit(void);
bool __stdcall httpDown_get(char* url, char* fileName);
DEF_RETPAIR(httpDown_open_t, char*, data, int, len);
httpDown_open_t __stdcall httpDown_open(char* url, bool);
char* FetchHtml_UTF8(char* url);

#endif
