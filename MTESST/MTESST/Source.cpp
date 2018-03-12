//Include dependencies
#include <wx/wx.h>
#include <mathplot.h>
#include "wx/notebook.h"
#include <wx/grid.h>

#include <math.h>

#include<rs232.h>

#include "icon.xpm"

wxDECLARE_EVENT(myEVT_THREAD_UPDATE, wxThreadEvent);

//Create window (GUI) class
class MyFrame : public wxFrame, public wxThreadHelper
{
public:
	//Declare GUI's functions and events
	MyFrame();
	~MyFrame();
	void OnThreadUpdate(wxThreadEvent& evt);
	void OnClose(wxCloseEvent& evt);
	void StartAlongTask();
	void Show();
	void OnSettingButtonClicked(wxCommandEvent&);
	void OnRunButtonClicked(wxCommandEvent&);
	void OnCancelButtonClicked(wxCommandEvent&);
protected:
	virtual wxThread::ExitCode Entry();
	wxDECLARE_EVENT_TABLE();

private:
	//Declare GUI's variables
	wxFrame * window;
	wxGrid* m_data;
	mpWindow* m_plot;
	std::vector<double> vectorx, vectory;
	mpFXYVector* vectorLayer;
	wxTextCtrl *displacement;
	wxTextCtrl *frequency;
	wxTextCtrl *cycles;
	wxTextCtrl *force;
	wxTextCtrl *diameter;
	bool cancel = false;
};

//(indentor_app);
//Create event table
wxDEFINE_EVENT(myEVT_THREAD_UPDATE, wxThreadEvent);
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_THREAD(myEVT_THREAD_UPDATE, MyFrame::OnThreadUpdate)
EVT_CLOSE(MyFrame::OnClose)
wxEND_EVENT_TABLE()

MyFrame::MyFrame()//GUI's constructor
{
	window = new wxFrame(NULL, -1, "MTESST", wxDefaultPosition, wxSize(700, 750));
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

	displacement = new wxTextCtrl(window, -1);
	settings->Add(new wxStaticText(window, -1, "Enter Desired Displacement(mm):"), 0, wxALL, 12);
	settings->Add(displacement, 0, wxALL, 10);

	frequency = new wxTextCtrl(window, -1);
	settings->Add(new wxStaticText(window, -1, "Enter Desired Frequency(Hz):"), 0, wxALL, 12);
	settings->Add(frequency, 0, wxALL, 10);

	cycles = new wxTextCtrl(window, -1);
	settings->Add(new wxStaticText(window, -1, "Enter Desired Number of Cycles:"), 0, wxALL, 12);
	settings->Add(cycles, 0, wxALL, 10);

	wxBoxSizer *settings2 = new wxBoxSizer(wxHORIZONTAL);

	force = new wxTextCtrl(window, -1);
	settings2->Add(new wxStaticText(window, -1, "Enter Max Allowable Force(N):      "), 0, wxALL, 12);
	settings2->Add(force, 0, wxALL, 10);

	diameter = new wxTextCtrl(window, -1);
	settings2->Add(new wxStaticText(window, -1, "Enter Tip Diameter(mm):       "), 0, wxALL, 12);
	settings2->Add(diameter, 0, wxALL, 10);

	wxButton *settingsbut = new wxButton(window, -1, "Enter Settings");
	settings2->Add(settingsbut, 0, wxALL, 10);
	settingsbut->Bind(wxEVT_BUTTON, &MyFrame::OnSettingButtonClicked, this);
	//End of settings

	//Start of Graph and text panel
	wxNotebook *notebook = new wxNotebook(window, wxID_ANY);
	wxBoxSizer *data = new wxBoxSizer(wxHORIZONTAL);

	wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	m_plot = new mpWindow(notebook, -1, wxDefaultPosition, wxSize(500, 300), wxSUNKEN_BORDER);
	mpScaleX* xaxis = new mpScaleX(wxT("X"), mpALIGN_BOTTOM, true, mpX_NORMAL);
	mpScaleY* yaxis = new mpScaleY(wxT("Y"), mpALIGN_LEFT, true);
	vectorLayer = new mpFXYVector(_("Vector"));
	xaxis->SetFont(graphFont);
	yaxis->SetFont(graphFont);
	xaxis->SetDrawOutsideMargins(false);
	yaxis->SetDrawOutsideMargins(false);
	m_plot->SetMargins(50, 50, 50, 100);
	m_plot->AddLayer(xaxis);
	m_plot->AddLayer(yaxis);
	m_plot->AddLayer(vectorLayer);

	vectorLayer->SetData(vectorx, vectory);
	vectorLayer->SetContinuity(true);
	wxPen vectorpen(*wxBLUE, 2, wxSOLID);
	vectorLayer->SetPen(vectorpen);
	vectorLayer->SetDrawOutsideMargins(false);

	m_data = new wxGrid(notebook, -1, wxDefaultPosition, wxSize(500, 300));
	m_data->CreateGrid(0, 0);
	m_data->AppendRows(10);
	m_data->AppendCols(3);
	m_data->EnableEditing(false);
	m_data->SetColLabelValue(0, "Time(s)");
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
	wxButton *cancel = new wxButton(window, -1, "Cancel");
	wxButton *run = new wxButton(window, -1, "Run");
	control->Add(new wxButton(window, -1, "Jog Indentor"), 0, wxALL, 10);
	control->Add(cancel, 0, wxALL, 10);
	cancel->Bind(wxEVT_BUTTON, &MyFrame::OnCancelButtonClicked, this);
	control->Add(run, 0, wxALL, 10);
	run->Bind(wxEVT_BUTTON, &MyFrame::OnRunButtonClicked, this);

	//End of Control Buttons (not include setting button)

	data->Add(control, 0, wxALIGN_BOTTOM);
	topsizer->Add(settings, 0);
	topsizer->Add(settings2, 0);
	topsizer->Add(data, 1, wxEXPAND);

	window->SetSizerAndFit(topsizer);
}

MyFrame::~MyFrame()//GUI's destructor
{

}

wxThread::ExitCode MyFrame::Entry()//Background threads function
{
	//Gets data for graph and chart
	wxThreadEvent evt(wxEVT_THREAD, myEVT_THREAD_UPDATE);
	int p = -100;
	while (1) {
		if (cancel == true)
		{
			break;
		}
		else if (!GetThread()->TestDestroy())
		{
			p++;
			vectorx.push_back(((double)p - 50.0)*5.0);
			vectory.push_back(0.0001*pow(((double)p - 50.0)*5.0, 3));
			unsigned char buff[4096];
			RS232_PollComport(2, buff, 2095);
			m_data->SetCellValue(0, 0, buff);
		}
		wxThread::Sleep(500);
		wxQueueEvent(this, evt.Clone());

	}
	return (wxThread::ExitCode)0;
}

void MyFrame::OnThreadUpdate(wxThreadEvent& evt)//GUI event for when background function updates
{
	//Outputs data to graph
	vectorLayer->SetData(vectorx, vectory);
	m_plot->Fit();
}

void MyFrame::OnClose(wxCloseEvent&)//GUI close event
{
	//Closes thread and serial port
	//Then closes GUI's window
	if (GetThread() && GetThread()->IsRunning())
		GetThread()->Wait();
	RS232_CloseComport(2);
	Destroy();
}

void MyFrame::StartAlongTask()//Creates background thread
{
	if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
	{
		wxLogError("Could not create the worker thread!");
		return;
	}
	if (GetThread()->Run() != wxTHREAD_NO_ERROR)
	{
		wxLogError("Could not run the worker thread!");
		return;
	}
}

void MyFrame::Show()//Function that makes GUI's window visable
{
	window->Show();
}

void MyFrame::OnSettingButtonClicked(wxCommandEvent&)//Event for settings button press
{
	//Retreives inputed settings and output them to the screen for confirmation
	wxString settingValues = displacement->GetValue();
	settingValues.Append("mm displacement \n");
	settingValues.Append(frequency->GetValue());
	settingValues.Append("Hz frequency \n");
	settingValues.Append(cycles->GetValue());
	settingValues.Append(" cycles \n");
	settingValues.Append(force->GetValue());
	settingValues.Append("N maximum force\n");
	settingValues.Append(diameter->GetValue());
	settingValues.Append("mm diameter tip");

	wxMessageBox(settingValues, "Settings Entered", wxOK | wxCANCEL);
}

void MyFrame::OnCancelButtonClicked(wxCommandEvent&)//On cancel button pressed
{
	//Sets cacel to true to stop work thread from getting data
	cancel = true;
}

void MyFrame::OnRunButtonClicked(wxCommandEvent&)//Event for start button pressed
{
	//Creates background thread
	this->StartAlongTask();
	cancel = false;
	vectorx.clear();
	vectory.clear();
}

class indentor_app : public wxApp//Creates app class
{
public:
	bool OnInit()//Start up function
	{
		//Creates GUI and opens serial port
		MyFrame* frame = new MyFrame();
		frame->Show();
		int port = 2;
		int bdrate = 9600;
		char mode[] = { '8','N','1',0 };
		RS232_OpenComport(port, bdrate, mode);
		return true;
	}
};

IMPLEMENT_APP(indentor_app);