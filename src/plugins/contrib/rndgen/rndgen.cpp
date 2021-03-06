#include "sdk.h"
#ifndef CB_PRECOMP
    #include <wx/intl.h>
    #include <wx/string.h>
    #include <wx/regex.h>

    #include "cbeditor.h"
    #include "manager.h"
#endif
#include "rndgen.h"
#include "cbstyledtextctrl.h"

#define SCI_SETUNDOCOLLECTION 2012

#include <random>

static unsigned int get_random()
{
	static std::mersenne_twister_engine <unsigned int, 32, 624, 397, 31,
                                        0x9908b0df, 11,
                                        0xffffffff,  7,
                                        0x9d2c5680, 15,
                                        0xefc60000, 18, 1812433253> randgen(time(0));
	return randgen();
};


namespace
{
    PluginRegistrant<RndGen> reg(_T("rndgen"));
}

void RndGen::OnAttach()
{
	Manager::Get()->RegisterEventSink(cbEVT_EDITOR_BEFORE_SAVE, new cbEventFunctor<RndGen, CodeBlocksEvent>(this, &RndGen::OnSave));
}

void RndGen::OnSave(CodeBlocksEvent& event)
{
	cbEditor* ed = (cbEditor*) event.GetEditor();
	cbStyledTextCtrl* ctrl = ed->GetControl();

	wxString quicktest = ctrl->GetText();
	if(quicktest.Contains(_T("RANDGEN:")) == false)
		return;

	int pos = ctrl->GetCurrentPos();
	ctrl->SendMsg(SCI_SETUNDOCOLLECTION, 0, 0);

	wxRegEx int_re(_T("([0-9]+)\\ *;?\\ */\\*(\\ *RANDGEN:INT\\((.*))\\*/"));
//	wxRegEx alnum_re(_T("\\\"([^\"]+)\\\"\\ *;?\\ */\\*(\\ *RANDGEN:ALNUM\\((.*))\\*/"));
	wxRegEx alnum_re(_T("\\\"([^\\\"]+)\\\"\\ *;?\\ */\\*(\\ *RANDGEN:(ALNUM|DIGITS|CHARS|UPPERCHARS|LOWERCHARS)\\((.*))\\*/"));

	wxString c(_T("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"));

	assert(alnum_re.IsValid() && int_re.IsValid());

	unsigned int n = ctrl->GetLineCount();

	for(unsigned int i = 0; i < n; ++i)
	{
		int a = ctrl->GetLineIndentPosition(i);
		int b = ctrl->GetLineEndPosition(i);

		wxString s = ctrl->GetTextRange(a,b);

		// This is rather crude and will only match a single hit per line.
		// However, that should be good enough for any sensible use, as you will not want more than that anyway.
		// Still, if more correct matching is needed, feel free to do this as homework :-)
		if(int_re.Matches(s))
		{
			wxString search = int_re.GetMatch(s, 1);
			long arg;
			int_re.GetMatch(s, 3).ToLong(&arg);
			wxString replace;
			int rnd = get_random() % (arg+1);
			replace.Printf(_T("%u"), rnd);
			s.Replace(search, replace, false);

			ctrl->SetTargetStart(a);
			ctrl->SetTargetEnd(b);
			ctrl->ReplaceTarget(s);
		}
		else if(alnum_re.Matches(s))
		{
			wxString search = alnum_re.GetMatch(s, 1);
			long arg;
			wxString what = alnum_re.GetMatch(s, 3);
			alnum_re.GetMatch(s, 4).ToLong(&arg);
			wxString replace;
			if(what == _T("ALNUM"))
			{
				for(int j = 0; j<arg; ++j)
					replace += c[get_random() % c.length()];
			}
			if(what == _T("DIGITS"))
			{
				for(int j = 0; j<arg; ++j)
					replace += c[get_random() % 10];
			}
			if(what == _T("CHARS"))
			{
				for(int j = 0; j<arg; ++j)
					replace += c[10+ get_random() % (c.length() - 10)];
			}
			if(what == _T("UPPERCHARS"))
			{
				for(int j = 0; j<arg; ++j)
					replace += c[36 + get_random() % 26];
			}
			if(what == _T("LOWERCHARS"))
			{
				for(int j = 0; j<arg; ++j)
					replace += c[10 + get_random() % 26];
			}
			s.Replace(search, replace, false);

			ctrl->SetTargetStart(a);
			ctrl->SetTargetEnd(b);
			ctrl->ReplaceTarget(s);
		}
	}
	ctrl->SendMsg(SCI_SETUNDOCOLLECTION, 1, 0);
	ctrl->SetCurrentPos(pos);
}
