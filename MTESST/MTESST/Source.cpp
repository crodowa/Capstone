#include <wx/wx.h>
#include <mathplot.h>
#include "wx/notebook.h"
#include <wx/grid.h>

#include <math.h>

#include "icon.xpm"

class MySIN : public mpFX
{
	double m_freq, m_amp;
public:
	MySIN(double freq, double amp) : mpFX(wxT("f(x) = SIN(x)"), mpALIGN_LEFT) { m_freq = freq; m_amp = amp; m_drawOutsideMargins = false; }
	virtual double GetY(double x) { return m_amp * sin(2 * M_PI*m_freq*x); }
	virtual double GetMinY() { return -m_amp; }
	virtual double GetMaxY() { return  m_amp; }
};

class indentor_app : public wxApp
{
public:
	bool OnInit()
	{
		//Create main window
		wxFrame* window = new wxFrame(NULL, -1, "MTESST", wxDefaultPosition, wxSize(700, 750));
		window->Fit();
		wxIcon *icon = new wxIcon(_e0968bce85d4723c058a9ee8c33f35c);
		window->SetIcon(*icon);

		//End of window creation

		//Menu Creation
		wxMenuBar* menuBar = new wxMenuBar();
		wxMenu *fileMenu = new wxMenu();
		fileMenu->Append(wxID_OPEN, _T("&Open"));
		fileMenu->Append(wxID_SAVE, _T("&Save"));
		fileMenu->AppendSeparator();
		fileMenu->Append(wxID_EXIT, _T("&Quit"));
		menuBar->Append(fileMenu, _T("&File"));
		wxMenu *helpMenu = new wxMenu();
		helpMenu->Append(wxID_ABOUT, _T("&About"));
		menuBar->Append(helpMenu, _T("&Help"));
		window->SetMenuBar(menuBar);
		//End of Menu


		wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

		//Settings
		wxBoxSizer *settings = new wxBoxSizer(wxHORIZONTAL);
		settings->Add(new wxStaticText(window, -1, "Enter Desired Displacement(mm):"), 0, wxALL, 12);
		settings->Add(new wxTextCtrl(window, -1), 0, wxALL, 10);
		settings->Add(new wxStaticText(window, -1, "Enter Desired Frequency(Hz):"), 0, wxALL, 12);
		settings->Add(new wxTextCtrl(window, -1), 0, wxALL, 10);
		settings->Add(new wxStaticText(window, -1, "Enter Desired Number of Cycles:"), 0, wxALL, 12);
		settings->Add(new wxTextCtrl(window, -1), 0, wxALL, 10);

		wxBoxSizer *settings2 = new wxBoxSizer(wxHORIZONTAL);
		settings2->Add(new wxStaticText(window, -1, "Enter Max Allowable Force(N):      "), 0, wxALL, 12);
		settings2->Add(new wxTextCtrl(window, -1), 0, wxALL, 10);
		settings2->Add(new wxStaticText(window, -1, "Enter Tip Diameter(mm):       "), 0, wxALL, 12);
		settings2->Add(new wxTextCtrl(window, -1), 0, wxALL, 10);
		settings2->Add(new wxButton(window, -1, "Enter Settings"), 0, wxALL, 10);
		//End of settings

		//Start of Graph and text panel
		wxNotebook *notebook = new wxNotebook(window, wxID_ANY);
		wxBoxSizer *data = new wxBoxSizer(wxHORIZONTAL);
		
		wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		mpWindow* m_plot = new mpWindow(notebook, -1, wxDefaultPosition, wxSize(500, 300), wxSUNKEN_BORDER);
		mpScaleX* xaxis = new mpScaleX(wxT("X"), mpALIGN_BOTTOM, true, mpX_NORMAL);
		mpScaleY* yaxis = new mpScaleY(wxT("Y"), mpALIGN_LEFT, true);
		xaxis->SetFont(graphFont);
		yaxis->SetFont(graphFont);
		xaxis->SetDrawOutsideMargins(false);
		yaxis->SetDrawOutsideMargins(false);
		m_plot->SetMargins(50, 50, 50, 100);
		m_plot->AddLayer(xaxis);
		m_plot->AddLayer(yaxis);
		m_plot->AddLayer(new MySIN(1.0, 1.0));

		wxGrid* m_data = new wxGrid(notebook, -1, wxDefaultPosition, wxSize(500, 300));
		m_data->CreateGrid(0, 0);
		m_data->AppendRows(10);
		m_data->AppendCols(3);
		m_data->EnableEditing(false);
		m_data->SetColLabelValue(0,"Time(s)");
		m_data->SetColLabelValue(1, "Force(mN)");
		m_data->SetColLabelValue(2, "Displacement(mm)");
		m_data->Fit();
		
		notebook->AddPage(m_plot, wxT("Gragh"));
		notebook->AddPage(m_data, wxT("Text"));
		data->Add(notebook, 1, wxGROW | wxALL, 10);
		//End of Graph and text panel

		//Start of Control Buttons (not include setting button)

		static const wxString label[] = { "Graph","Data" };
		wxBoxSizer *control = new wxBoxSizer(wxVERTICAL);
		//control->Add(new wxRadioBox(window, -1, "Data", wxDefaultPosition, wxDefaultSize, 2, label, 1, wxRA_SPECIFY_COLS), 0, wxALL, 10);
		control->Add(new wxButton(window, -1, "Jog Indentor"), 0, wxALL, 10);
		control->Add(new wxButton(window, -1, "Cancel"), 0, wxALL, 10);
		control->Add(new wxButton(window, -1, "Run"), 0, wxALL, 10);

		//End of Control Buttons (not include setting button)

		data->Add(control, 0, wxALIGN_BOTTOM);
		topsizer->Add(settings, 0);
		topsizer->Add(settings2, 0);
		topsizer->Add(data, 1, wxEXPAND);

		window->SetSizerAndFit(topsizer);

		window->Show();
		return true;
	}
};

IMPLEMENT_APP(indentor_app);