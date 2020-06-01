// HtmlRead V2.25, 25/05/2014
// This code Parses HTML on the lowest possible leve1

#ifndef _HTMLREAD_H_
#define _HTMLREAD_H_

struct HtmlEvent
{
	char* tag;
	char* arg;
	char* innerStr;
	int innerLen;
	int nestLevel;
	
	HtmlEvent* child(int idx = 0);
	HtmlEvent* childFind(const char* str);
	HtmlEvent* sibling(int idx = 0);
	HtmlEvent* siblingFind(const char* str);
	HtmlEvent* sibchild(int idx = 0);
	
	bool textTest(const char* str);
	bool tagTest(const char* str);	
	char* text(void);
	char* text2(void);
	char* attr(const char* name);
	char* href(void);
	char* opening(void);
	char* closing(void);
};

struct HtmlRead
{
	HtmlRead();
	~HtmlRead();
	void init(const char* htmlData);
	HtmlEvent* event();
	HtmlEvent* parent();
	HtmlEvent* next();
	HtmlEvent* child();
	HtmlEvent* childFind(const char* str);
	HtmlEvent* childFind(const char* str, int nest);
	HtmlEvent* sibling();
	HtmlEvent* siblingFind(const char* str);
	void reset();
	
	bool textTest(const char* str);
	bool tagTest(const char* str);	
	char* text(void);
	char* text2(void);
	char* attr(const char* name);
	char* href(void);
	char* innerHtml(HtmlEvent* event = NULL);
	int offsetLength(int& length);

	int nEvents;
	HtmlEvent* events;
	
private:
	const char* htmlData;
	char* HtmlPos;
	char* innerBuff;
	HtmlEvent* nextPos;
	HtmlEvent* parentPos;
};

// IMPLEMENTATION
inline void HtmlRead::reset() { 
	nextPos = parentPos+1; }
inline char* HtmlEvent::href(void)
	{	return this->attr("href"); }
inline HtmlRead::HtmlRead() 
	: events(0), nEvents(0), HtmlPos(0), innerBuff(0) {}
inline HtmlRead::~HtmlRead()
	{	free(events); free(innerBuff);	}
inline char* HtmlRead::text(void)
	{	return event()->text(); }
inline char* HtmlRead::text2(void)
	{	return event()->text2(); }
inline bool HtmlRead::textTest(const char* str)
	{	return event()->textTest(str); }
inline bool HtmlRead::tagTest(const char* str)
	{	return event()->tagTest(str);	}
inline char* HtmlRead::attr(const char* name)
	{ 	return event()->attr(name);	}
inline char* HtmlRead::href(void)
	{	return event()->href();	}
inline HtmlEvent* HtmlRead::event()
	{	return nextPos-1; }
inline HtmlEvent* HtmlRead::parent()
	{	return parentPos; }

#endif
