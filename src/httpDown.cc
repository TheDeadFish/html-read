#include "stdafx.h"
#include "httpDown.h"
#include <iconv.h>
#include <curl/curl.h>

const char* httpDown_agent;
CURL* httpDown_ctx;

static int __stdcall do_perform(
	char* url, void* wrCtx, void* wrFunc)
{
	curl_easy_setopt(httpDown_ctx, CURLOPT_LOW_SPEED_TIME, 20L);
	curl_easy_setopt(httpDown_ctx, CURLOPT_LOW_SPEED_LIMIT, 30L);
	if(curl_easy_setopt(httpDown_ctx, CURLOPT_WRITEDATA, wrCtx)
	|| curl_easy_setopt(httpDown_ctx, CURLOPT_WRITEFUNCTION, wrFunc)
	|| curl_easy_setopt(httpDown_ctx, CURLOPT_URL, url)) errorAlloc();
	return curl_easy_perform(httpDown_ctx);
}

static void __stdcall http_progress(void) {
	static char ch; ch = (ch == '/') ?
		'\\' : '/'; printf("%c\b", ch);  }

void httpDown_init(void)
{
	CURL* curl = errorAlloc(curl_easy_init());
	char buf[MAX_PATH+1]; char *ptr = NULL; int len = SearchPathA(
		NULL, "curl-ca-bundle.crt", NULL, MAX_PATH+1, buf, &ptr);
	if(len <= 0) fatalError("httpDown; \"curl-ca-bundle.crt\" not found");
	if((curl_easy_setopt(curl, CURLOPT_CAINFO, buf))
	||(curl_easy_setopt(curl, CURLOPT_USERAGENT, httpDown_agent))
	||(curl_easy_setopt(curl, CURLOPT_COOKIEFILE, ""))) errorAlloc();
	httpDown_ctx = curl;
}

void httpDown_deInit()
{
	curl_easy_cleanup(httpDown_ctx);
	httpDown_ctx = NULL;
}


bool __stdcall httpDown_get(char* url, char* fileName)
{
	auto file = httpDown_open(url, false);
	if(file.data == NULL) return false;
	FILE* fp = xfopen(fileName, "!wb");
	xfwrite(file.data, 1, file.len, fp);
	fclose(fp); free(file.data); return true;
}

static
size_t httpDown_owrite(void *ptr, size_t size, size_t nmemb, xvector_ *vect) {
	http_progress(); size *= nmemb; memcpy(vect->xnxalloc_(size), ptr, size); return nmemb; }
httpDown_open_t httpDown_open(char* url, bool nullTerm) { printf("@\b");
	DO_OVER: if(httpDown_ctx == NULL) httpDown_init(); http_progress();
	xvector_ fileData; fileData.init_(); int result =
	do_perform(url, &fileData, (void*)httpDown_owrite);
	if(result) { fileData.free();
	if(result == CURLE_OPERATION_TIMEDOUT) {
		printf("!"); httpDown_deInit(); goto DO_OVER; }
		 return httpDown_open_t(0,0); }
	if(nullTerm) fileData.xnxalloc_(1)[0] = '\0'; printf("#\b");
	return httpDown_open_t(fileData.dataPtr, fileData.dataSize);
}

char* FetchHtml_UTF8(char* url)
{
	// get html file & charset
	auto file = httpDown_open(url, true);
	if(file.data == NULL) return NULL;
	char* charSet; char* curPos = file.data;
	while(curPos = strstri(curPos, "<meta", -1))
	{
		char* metaEnd = strchr(curPos+=5, '>');
		if(metaEnd == NULL) continue;
		charSet = strstri(curPos, "charset=", metaEnd-curPos);
		if(charSet == NULL) continue;
		charSet = xstrdup(charSet, metaEnd-charSet);
		strcpy(charSet, strtok(charSet+8, "'\">"));
		goto FOUND_CHARSET;
	}
	return file.data;

	// convert file to utf8
FOUND_CHARSET:
	SCOPE_EXIT(free(charSet));
	if(!stricmp(charSet, "UTF-8"))
		return file.data;
	iconv_t cd = iconv_open("UTF-8", charSet);
	if(cd == (iconv_t)-1) return file.data;
	SCOPE_EXIT(free(file.data));
	char* buffer2 = xMalloc(file.len*3);
	const char* srcPos = file.data; char* dstPos = buffer2;
	size_t inbytesleft = file.len;
	size_t outbytesleft = file.len*3;
	if(iconv(cd, &srcPos, &inbytesleft,
	&dstPos, &outbytesleft) == (size_t)-1) {
		Beep(50, 250); free(buffer2); return NULL; }
	return buffer2;
}
