#include <stdshit.h>
#include "htmlRead.h"

// Original core
// Dont change whats not broken!
void killWhiteSpace(char* str)
{
	while(*str)
	{
		char* curPos = str;
		while((curPos[0] == ' ')
		&&(curPos[1] == ' '))
			curPos++;
		if(curPos > str)
			strcpy(str, curPos);
		str++;
	}
}

struct GetHtmlEvent
{
	char *HtmlPos;
	char NextTag;

	void Init(char *HtmlData) {
		this->HtmlPos = HtmlData;
		this->NextTag = '\0'; }		
	struct HtmlEvent {
		char* Tag;
		char* Arg; };
	HtmlEvent Get(void);
};		

GetHtmlEvent::HtmlEvent GetHtmlEvent::Get(void)
{
	int test;
TRY_AGAIN1:
	HtmlEvent htmlEvent;
	if(HtmlPos == NULL){
		htmlEvent.Tag = NULL;
		return(htmlEvent);
	}
	if(NextTag == 0){
		switch(*HtmlPos){
			case '<':
			case '>':
				NextTag = *HtmlPos;
				HtmlPos++;
				break;
			case '\0':
				htmlEvent.Tag = NULL;
				return(htmlEvent);
			default:
				NextTag = '>';
				break;
		}
	}	
		
	// Check for Readable  Text
	if(NextTag == '>'){
		while(*HtmlPos == '>')
			HtmlPos++;
		if(*HtmlPos == '<'){
			HtmlPos++;
			goto IsTag;
		}
		htmlEvent.Arg = HtmlPos;
		while((*HtmlPos != '\0')&&(*HtmlPos != '<'))
			HtmlPos++;
		NextTag = *HtmlPos;
		if(*HtmlPos != '\0'){
			*HtmlPos ='\0';
			htmlEvent.Tag = HtmlPos++;
		}
		else
			htmlEvent.Tag = HtmlPos;
		killWhiteSpace(htmlEvent.Arg);
		if(htmlEvent.Arg[0] == '\0')
			goto TRY_AGAIN1;
		return(htmlEvent);
	}
	else
	{
	IsTag:	
		// Process comment
		if(HtmlPos[0] == '!')
		{
			if(strncmp(HtmlPos, "!--", 3) == 0)
				HtmlPos = 3+strstr(HtmlPos, "-->");
			else
				HtmlPos = 1+strchr(HtmlPos, '>');
			if(size_t(HtmlPos) < 10)
				HtmlPos = NULL;
			NextTag = 0;
			goto TRY_AGAIN1;
		}
	
		// Process Tag
		char *TagEnd, *NextTagPos;
		TagEnd = HtmlPos;
		while((*TagEnd != '\0')&&(*TagEnd != '<')&&(*TagEnd != '>'))
			TagEnd++;
		NextTagPos = TagEnd;
		NextTag = *TagEnd;
		if(*TagEnd != '\0'){
			*TagEnd = '\0';
			NextTagPos++;
		}
		
		htmlEvent.Tag = HtmlPos;
		HtmlPos = strchr(HtmlPos, ' ');
		if(HtmlPos != NULL){
			*HtmlPos = '\0';
			htmlEvent.Arg = HtmlPos+1;
		}
		else
			htmlEvent.Arg = TagEnd;
		HtmlPos = NextTagPos;
		return(htmlEvent);
	}
}

// New Functionality
// 1. Nest Level
// 2. Attribute getter
// 3. Tags as array

void HtmlRead::init(const char* HtmlData)
{
	free(HtmlPos);
	HtmlPos = xstrdup(HtmlData);
	GetHtmlEvent getHtmlEvent;
	getHtmlEvent.Init(HtmlPos);
	this->htmlData = HtmlData;
	int parent = -1;
	int nestLevel = 0;
	nEvents = 0;
	
	while(1)
	{
		auto htmlEvent = getHtmlEvent.Get();
		if(htmlEvent.Tag == NULL)
			break;
			
		if(htmlEvent.Tag[0] == '/')
		{
			nestLevel--;
			if(parent >= 0)
			{
				int innerLen = events[parent].innerLen;
				events[parent].innerLen = htmlEvent.Tag - events[parent].innerStr;
				events[parent].innerLen -= 2;
				if(innerLen < 0)
					parent = -innerLen;
				else
					parent = -1;
			}
		}
		else
		{
			HtmlEvent* event = 
				&xNextAlloc(events, nEvents);

			event->tag = htmlEvent.Tag;
			event->arg = htmlEvent.Arg;
			event->innerStr = htmlEvent.Arg;
			event->innerLen = 0;
			event->nestLevel = nestLevel;
			if(htmlEvent.Tag[0] != '\0')
			{
				int tagLength = strlen(htmlEvent.Tag);
				int argLength = strlen(htmlEvent.Arg);
				if((tagLength != 0)&&(argLength == 0)
				&&(htmlEvent.Tag[tagLength-1] == '/'))
					htmlEvent.Tag[tagLength-1] = '\0';
					
				event->innerStr += argLength;
				if((argLength == 0)
				|| (htmlEvent.Arg[argLength-1] != '/'))
				{
					// check singleton tags
					const char* const sigleTon[] = {
						"area", "base", "br", "col",
						"command", "embed", "hr",
						"img", "input", "link", "wbr",
						"meta", "param", "source", 0 };
					for(int i = 0; sigleTon[i]; i++)
						if(strcmp(htmlEvent.Tag, sigleTon[i]) == 0)
							goto IS_SINGLETON;
					
					nestLevel++;
					if(parent >= 0)
						event->innerLen = -parent;
					parent = event-events;
					
				IS_SINGLETON:;
				}
			}
		}
	}
	
	
	HtmlEvent* event = 
		&xNextAlloc(events, nEvents);
	event->tag = NULL;
	event->nestLevel = 0;
	nextPos = events;
}

HtmlEvent* HtmlRead::next()
{
	if(nextPos->tag == NULL)
		return NULL;
	parentPos = nextPos;
	return nextPos++;
}

HtmlEvent* HtmlRead::child()
{
	if(nextPos->nestLevel > parentPos->nestLevel)
		return nextPos++;
	return NULL;
}

HtmlEvent* HtmlRead::childFind(const char* str)
{
	HtmlEvent* child;
	while(child = this->child())
	{
		if(child->tagTest(str))
			return child;
	}
	return NULL;
}

HtmlEvent* HtmlRead::childFind(const char* str, int nest)
{
	HtmlEvent* child;
	while((child = childFind("a"))
	&&(child->nestLevel != parentPos->nestLevel+1));
	return child;
}

HtmlEvent* HtmlRead::sibling()
{
	HtmlEvent* newPos = event()->sibling();
	if(newPos != NULL)
		nextPos = newPos+1;
	return newPos;
}

HtmlEvent* HtmlRead::siblingFind(const char* str)
{
	HtmlEvent* newPos = event()->siblingFind(str);
	if(newPos != NULL)
		nextPos = newPos+1;
	return newPos;
}

char* HtmlRead::innerHtml(HtmlEvent* event)
{
	if(event == NULL)
		event = this->event();
	if(event->innerLen == 0)
		return NULL;

	xrealloc(innerBuff, event->innerLen+1);
	innerBuff[event->innerLen] = '\0';
	int offset = event->innerStr+1 - HtmlPos;
	memcpy(innerBuff, &htmlData[offset], event->innerLen);
	return innerBuff;
}

int HtmlRead::offsetLength(int& length)
{
	char* opening = event()->opening();
	length = event()->closing()-opening;
	return opening-HtmlPos;
}

HtmlEvent* HtmlEvent::child(int idx)
{
	THIS_NULL_CHECK();
	if(this[idx+1].nestLevel > this->nestLevel)
		return &this[idx+1];
	return NULL;
}

HtmlEvent* HtmlEvent::childFind(const char* str)
{
	THIS_NULL_CHECK();
	HtmlEvent* child;
	for(int i = 0; child = this->child(i); i++)
	{
		if(child->tagTest(str))
			return child;
	}
	return NULL;
}

HtmlEvent* HtmlEvent::sibling(int idx)
{
	THIS_NULL_CHECK();
	HtmlEvent* curPos = this;
	do{
		curPos++;
		if(curPos->nestLevel < this->nestLevel)
			return NULL;
		while(curPos->nestLevel > this->nestLevel)
			curPos++;
	}while(idx--);
	return curPos;
}

HtmlEvent* HtmlEvent::siblingFind(const char* str)
{
	THIS_NULL_CHECK();
	HtmlEvent* sibling;
	for(int i = 0; sibling = this->sibling(i); i++)
	{
		if(sibling->tagTest(str))
			return sibling;
	}
	return NULL;
}

HtmlEvent* HtmlEvent::sibchild(int idx)
{
	THIS_NULL_CHECK();
	HtmlEvent* curPos = this;
	do{
		curPos++;
		if(curPos->nestLevel < this->nestLevel)
			return NULL;
	}while(idx--);
	return curPos;
}

bool HtmlEvent::textTest(const char* str)
{
	THIS_NULL_CHECK();
	char* text = this->text();
	if(text == NULL)
		return false;
	return !strcmp(text, str);
}

bool HtmlEvent::tagTest(const char* str)
{
	THIS_NULL_CHECK();
	HtmlEvent* child = this;
	int strLen = strcspn(str, " >");
	if(str[0] == '>')
		{ str++; goto COMPARE_TEXT; }
	if(!strcmpn(tag, str, strLen))
		return false;
	str += strLen;
	
	while(*str++ == ' ')
	{
		char tmp[32];
		strLen = strcspn(str, " >=");
		strcpyn(tmp, str, strLen);
		char* attr = this->attr(tmp);
		if(attr == NULL)
			return false;
		str += strLen;
			
		if(*str++ == '=')
		{
			strLen = strcspn(str, " >");
			if( strncmp(attr, str, strLen))
				return false;
			str += strLen;
		}
	}
	
	if(str[-1] == '>')
	{
		child = this->child(0);
	COMPARE_TEXT:
		if((child == NULL)||(child->tag[0]))
			return false;
			
		strLen = strcspn(str, ">");
		if(str[strLen] == '>')
		{
			if(strncmp(str, child->arg, strLen))
				return false;
		}
		else
		{
			if(strcmp(str, child->arg))
				return false;
		}
	}
	return true;
}

char* HtmlEvent::text(void)
{
	THIS_NULL_CHECK();
	if(this->tag[0] == '\0')
		return this->arg;
	HtmlEvent* child = this->childFind("");
	if(child == NULL)
		return NULL;
	return child->arg;
}

char* HtmlEvent::text2(void)
{
	HtmlEvent* event = this;
	HtmlEvent* (HtmlEvent::*func)(int) =
		&HtmlEvent::child;
	int length = 0, idx = 0;
	static char* buff;
	free(buff); buff = 0;
	
	if(this->tag[0] == '\0')
	{
		func = &HtmlEvent::sibchild;
		goto SIBLING_FIRST;
	}
	
	while(event = (this->*func)(idx++))
	{
		if(strcmp(event->tag, "br") == 0)
		{
			int start = length++;
			xrealloc(buff, length+1);
			strcpy(buff+start, "\n");
		}
		else if(event->tag[0] == '\0')
		{
	SIBLING_FIRST:
			int start = length;
			length += strlen(event->arg);
			xrealloc(buff, length+1);
			strcpy(buff+start, event->arg);
		}
	}
	return buff;
}

char* HtmlEvent::attr(const char* name)
{
	THIS_NULL_CHECK();
	
	// Find Attribute
	int argLength = innerStr-arg;
	char* arg = NULL;
	int len = strlen(name);
	for(int i = 0; i < argLength; i++)
	{
		if((strncmp(this->arg+i, name, len) == 0)
		&&((this->arg[i+len] == '=')||(this->arg[i+len] == ' ')
		||(this->arg[i+len] == '\0')))
		{
			arg = this->arg+i+len+1;
			break;
		}
	}
	if(arg == NULL)
		return NULL;
		
	// Process Attribute
	if(arg[0] == '"')
		return strtok(arg+1, "\"");
	if(arg[0] == '\'')
		return strtok(arg+1, "'");
	return strtok(arg, " \t\r\n");
}

char* HtmlEvent::opening(void)
{
	if(tag[0])
		return tag-1;
	return arg;
}

char* HtmlEvent::closing(void)
{
	HtmlEvent* curPos = this+1;
	while(curPos->nestLevel > this->nestLevel)
		curPos++;
	return curPos->opening();
}
