#include "doomdef.h"
#include "hudmessages.h"
#include "c_cvars.h"
#include "v_video.h"
#include "v_text.h"
#include "cmdlib.h"

DHUDMessage::~DHUDMessage()
{
	if (Lines)
	{
		V_FreeBrokenLines(Lines);
		Lines = NULL;
		//BorderNeedRefresh = screen->GetPageCount();
	}
	if (SourceText != NULL)
	{
		delete[] SourceText;
	}
}

DHUDMessage::DHUDMessage(const char *text, float x, float y, EColorRange textColor,
	float holdTime)
{
	if (fabs(x) > 2.f)
	{
		CenterX = true;
		Left = 0.5f;
	}
	else
	{
		Left = x < -1.f ? x + 1.f : x > 1.f ? x - 1.f : x;
		if (fabs(x) > 1.f)
		{
			CenterX = true;
		}
		else
		{
			CenterX = false;
		}
	}
	Top = y;
	//Next = NULL;
	Lines = NULL;
	HoldTics = (int)(holdTime * TICRATE);
	Tics = 0;
	TextColor = textColor;
	State = 0;
	//SourceText = copystring(text);
	//Font = screen->Font;
	//ResetText(SourceText);
}