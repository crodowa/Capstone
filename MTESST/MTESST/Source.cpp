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
	std::vector<double> vectorTime, vectorDisplacement, vectorForce;
	mpFXYVector* vectorLayer;
	wxTextCtrl *displacement;
	wxTextCtrl *frequency;
	wxTextCtrl *cycles;
	wxTextCtrl *force;
	wxTextCtrl *diameter;
	bool cancel = false;
	wxString IndentorMode ="StartUp";
	wxString portMode = "Auto";
	wxString settings = "|_\\";
	int row = 0;
	unsigned char buff[1];
	int port = 0;
	int bdrate = 256000;
	char mode[4] = { '8','N','1',0 };
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
	fileMenu->Append(wxID_SAVE, _T("&Save"));
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, _T("&Quit"));
	menuBar->Append(fileMenu, _T("&File"));
	wxMenu *optionsMenu = new wxMenu();
	wxMenu *IndentorConnection = new wxMenu();
	IndentorConnection->Append(wxID_ANY,_T("Auto"));
	wxMenu *ManualConnection = new wxMenu();
	ManualConnection->Append(wxID_ANY, _T("Com1"));
	ManualConnection->Append(wxID_ANY, _T("Com2"));
	ManualConnection->Append(wxID_ANY, _T("Com3"));
	ManualConnection->Append(wxID_ANY, _T("Com4"));
	ManualConnection->Append(wxID_ANY, _T("Com5"));
	ManualConnection->Append(wxID_ANY, _T("Com6"));
	ManualConnection->Append(wxID_ANY, _T("Com7"));
	ManualConnection->Append(wxID_ANY, _T("Com8"));
	ManualConnection->Append(wxID_ANY, _T("Com9"));
	ManualConnection->Append(wxID_ANY, _T("Com10"));
	ManualConnection->Append(wxID_ANY, _T("Com11"));
	ManualConnection->Append(wxID_ANY, _T("Com12"));
	ManualConnection->Append(wxID_ANY, _T("Com13"));
	ManualConnection->Append(wxID_ANY, _T("Com14"));
	ManualConnection->Append(wxID_ANY, _T("Com15"));
	ManualConnection->Append(wxID_ANY, _T("Com16"));
	IndentorConnection->AppendSubMenu(ManualConnection, _T("Manual"));
	optionsMenu->AppendSubMenu(IndentorConnection,_T("&Indentor Connection"));
	menuBar->Append(optionsMenu, _T("&Options"));
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

	vectorLayer->SetData(vectorTime, vectorDisplacement);
	vectorLayer->SetContinuity(true);
	wxPen vectorpen(*wxBLUE, 2, wxSOLID);
	vectorLayer->SetPen(vectorpen);
	vectorLayer->SetDrawOutsideMargins(false);

	m_data = new wxGrid(notebook, -1, wxDefaultPosition, wxSize(500, 300));
	m_data->CreateGrid(0, 0);
	//m_data->AppendRows(10);
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
	//Right now it internally creates a array of data that continually updates the graph since no finished prototype
	//Also recieves serial input and then  updates the first cell of the chart with the data
	wxThreadEvent evt(wxEVT_THREAD, myEVT_THREAD_UPDATE);
	while (!GetThread()->TestDestroy()) {
		if (IndentorMode == "StartUp"&&portMode=="Auto") {
			try{
				int error=RS232_OpenComport(port, bdrate, mode);
				if (error == 1) {
					throw std::exception();
				}
				else {
					int serialReturned = 0;
					wxLongLong time = wxGetLocalTimeMillis();
					do {
						serialReturned = RS232_PollComport(port, buff, 1);
					} while (serialReturned == 0 && wxGetLocalTimeMillis()-time<2000);
					if (buff[0] ==(unsigned char) '1') {
						RS232_SendBuf(port,(unsigned char *)"1",1);
						IndentorMode = "Idle";
					}
					else {
						RS232_CloseComport(port);
						port++;
					}
				}
			}
			catch (std::exception& e) {
				if (port == 15) {
					port = 0;
				}
				else {
					port++;
				}
			}
		}
		else if (IndentorMode == "StartUp"&&portMode == "Manual") {

		}
		else if (IndentorMode == "Run") {
			RS232_PollComport(port, buff, 1);
			if (buff[0] == '|') {
				int i = 0;
				wxString data[3];
				double value;
				do {
					RS232_PollComport(port, buff, 1);
					if (buff[0] != '_' && buff[0]!='\\') {
						data[i].Append(buff[0]);
					}
					else {
						i++;
					}
				}while(buff[0] != '\\');
				m_data->AppendRows(1);
				data[0].ToDouble(&value);
				vectorTime.push_back(value);
				m_data->SetCellValue(row, 0, data[0]);
				data[1].ToDouble(&value);
				//vectorDisplacement.push_back(value);
				//m_data->SetCellValue(row, 1, data[1]);
				data[2].ToDouble(&value);
				//vectorForce.push_back(value);
				//m_data->SetCellValue(row, 2, data[2]);
				//vectorLayer->SetData(vectorTime, vectorDisplacement);
				m_plot->Fit();
				row++;
			}
		}
		else if (IndentorMode == "SendSettings") {
			RS232_SendBuf(port,(unsigned char*)(settings.mb_str()).data(), 10);
			IndentorMode = "Idle";
		}
		else if (IndentorMode == "START") {
			RS232_SendBuf(port, (unsigned char*)"/START\\", 10);
			IndentorMode = "Run";
		}
		else if (IndentorMode == "STOP") {
			RS232_SendBuf(port, (unsigned char*)"$", 10);
			IndentorMode = "Idle";
		}
		else {

		}
		wxThread::Sleep(50);
	}
	return (wxThread::ExitCode)0;
}

void MyFrame::OnThreadUpdate(wxThreadEvent& evt)//GUI event for when background function updates
{
	//Outputs data to graph
	vectorLayer->SetData(vectorTime, vectorDisplacement);
	m_plot->Fit();
}

void MyFrame::OnClose(wxCloseEvent&)//GUI close event
{
	//Closes thread and serial port
	//Then closes GUI's window
	if (GetThread() && GetThread()->IsRunning())
		GetThread()->Wait();
	RS232_CloseComport(port);
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

	settings = "|";
	settings.Append(cycles->GetValue());
	settings.Append("_");
	settings.Append(displacement->GetValue());
	settings.Append("\\");
	IndentorMode = "SendSettings";
}

void MyFrame::OnCancelButtonClicked(wxCommandEvent&)//On cancel button pressed
{
	//Sets cancel to true to stop work thread from getting data
	cancel = true;
}

void MyFrame::OnRunButtonClicked(wxCommandEvent&)//Event for start button pressed
{
	if (settings != "|_\\") {
		row = 0;
		vectorTime.clear();
		vectorDisplacement.clear();
		vectorForce.clear();
		IndentorMode = "START";
	}
	else {
		wxMessageBox("Please Enter Valid Settings", "Error", wxOK|wxICON_ERROR);
	}
}

class indentor_app : public wxApp//Creates app class
{
public:
	bool OnInit()//Start up function
	{
		//Creates GUI and opens serial port and creates background thread
		MyFrame* frame = new MyFrame();
		frame->Show();
		frame->StartAlongTask();
		return true;
	}
};

IMPLEMENT_APP(indentor_app);