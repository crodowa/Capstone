//Include dependencies
#include <wx/wx.h>
#include <mathplot.h>
#include "wx/notebook.h"
#include <wx/grid.h>

#include<wx/numformatter.h>

#include <math.h>

#include<rs232.h>

#include "icon.xpm"

//Indentor Table class
class indentorTable : public wxGridTableBase
{
protected:
	int rows;
	int cols;
	std::vector< std::vector<double> >data;
	std::vector<wxString>columnLabels;
public:
	indentorTable() { rows = 0; cols = 0; };
	indentorTable(const indentorTable&copy);
	virtual ~indentorTable();
	virtual int GetNumberRows() wxOVERRIDE;
	virtual int GetNumberCols() wxOVERRIDE;
	virtual bool AppendRows(size_t numRow) wxOVERRIDE;
	virtual bool AppendCols(size_t numCol) wxOVERRIDE;
	virtual wxString GetValue(int row, int col) wxOVERRIDE;
	virtual void SetValue(int row, int col, const wxString& value) wxOVERRIDE;
	virtual void SetColLabelValue(int numCol, const wxString& label) wxOVERRIDE;
	virtual wxString GetColLabelValue(int numCol) wxOVERRIDE;
	bool EmptyGrid();
};

indentorTable::indentorTable(const indentorTable&copy)
{
	columnLabels = copy.columnLabels;
	rows = copy.rows;
	cols = copy.cols;
}
indentorTable::~indentorTable()
{
	for (size_t i = 0; i < rows; i++) {
		data[i].clear();
	}
	data.clear();
	columnLabels.clear();
}
int indentorTable::GetNumberRows()
{
	return rows;
}
int indentorTable::GetNumberCols()
{
	return cols;
}
wxString indentorTable::GetValue(int row, int col)
{
	if (row > -1 && row<rows && col>-1 && col < cols) {
		//return wxString::Format(wxT("%0.4f"), data[row][col]);
		return wxNumberFormatter::ToString(data[row][col],10,0x02);
	}
}
void indentorTable::SetValue(int row, int col, const wxString& value)
{
	double val;
	value.ToDouble(&val);
	if (row<rows && row>-1 && col<cols && col>-1)
	{
		data[row][col] = val;
	}
}
bool indentorTable::AppendRows(size_t numRow)
{
	for (size_t i = 0; i < numRow; i++) {
		data.push_back(std::vector<double>());
		for (size_t j = 0; j < cols; j++) {
			data[rows+i].push_back(0);
		}
	}
	rows += numRow;
	wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, numRow);
	return GetView()->ProcessTableMessage(msg);
}
bool indentorTable::AppendCols(size_t numCol)
{
	for (int i = 0; i < numCol; i++) {
		columnLabels.push_back(wxGridTableBase::GetColLabelValue(i));
		for (size_t j = 0; j < rows; j++) {
			data[j].push_back(0);
		}
	}
	cols += numCol;
	wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_APPENDED, numCol);
	return GetView()->ProcessTableMessage(msg);
}
void indentorTable::SetColLabelValue(int numCol, const wxString& label)
{
	if (numCol < columnLabels.size())
		columnLabels[numCol] = label;
}
wxString indentorTable :: GetColLabelValue(int numCol)
{
	return columnLabels[numCol];
}
bool indentorTable::EmptyGrid()
{
	for (size_t i = 0; i < rows; i++) {
		data[i].clear();
	}
	data.clear();
	wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,0, rows);
	rows = 0;
	return GetView()->ProcessTableMessage(msg);
}

//serial thread class
class serialThread :public wxThreadHelper
{
public:
	virtual wxThread::ExitCode EntrySerial()=0;
	wxThread* StartAlongTaskSerial();
protected:
	virtual wxThread::ExitCode Entry() { return(this->EntrySerial()); };
};

//data thread class
class dataThread :public wxThreadHelper
{
public:
	virtual wxThread::ExitCode EntryData() = 0;
	wxThread* StartAlongTaskData();
protected:
	virtual wxThread::ExitCode Entry() { return(this->EntryData()); };
};


//Create window (GUI) class
class MyFrame : public wxFrame, public serialThread,public dataThread
{
public:
	//Declare GUI's functions and events
	MyFrame();
	~MyFrame();
	void OnClose(wxCloseEvent& evt);
	void StartAlongTask() {serial = StartAlongTaskSerial();data = StartAlongTaskData(); };
	void Show();
	void OnSettingButtonClicked(wxCommandEvent&);
	void OnRunButtonClicked(wxCommandEvent&);
	void OnCancelButtonClicked(wxCommandEvent&);
	void OnJogButtonClicked(wxCommandEvent&);
protected:
	virtual wxThread::ExitCode EntrySerial() wxOVERRIDE;
	virtual wxThread::ExitCode EntryData() wxOVERRIDE;
	wxDECLARE_EVENT_TABLE();

private:
	//Declare GUI's variables
	wxFrame * window;
	wxGrid* m_data;
	mpWindow* m_plot;
	mpWindow* m_plot2;
	mpWindow* m_plot3;
	std::vector<double> vectorTime, vectorDisplacement, vectorForce;
	mpFXYVector* vectorLayer;
	mpFXYVector* vectorLayer2;
	mpFXYVector* vectorLayer3;
	wxTextCtrl *displacement;
	wxTextCtrl *frequency;
	wxTextCtrl *cycles;
	wxTextCtrl *force;
	wxTextCtrl *diameter;
	wxString IndentorMode ="StartUp";
	wxString portMode = "Auto";
	wxString settings = "|_\\";
	int row = 0;
	unsigned char buff[1];
	int port = 0;
	int bdrate = 256000;
	char mode[4] = { '8','N','1',0 };
	wxThread* serial;
	wxThread* data;
	bool clearing = false;
	bool updatePlots = false;

	wxNotebook *notebook;
	wxRadioBox *jogMode;
	wxRadioBox *jogDirection;
};

//(indentor_app);
//Create event table
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_CLOSE(MyFrame::OnClose)
wxEND_EVENT_TABLE()

MyFrame::MyFrame()//GUI's constructor
{
	window = new wxFrame(NULL, -1, "MTESST", wxDefaultPosition, wxSize(700, 750));
	window->Fit();
	wxIcon *icon = new wxIcon(_521760424741);
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

	//frequency = new wxTextCtrl(window, -1);
	//settings->Add(new wxStaticText(window, -1, "Enter Desired Frequency(Hz):"), 0, wxALL, 12);
	//settings->Add(frequency, 0, wxALL, 10);

	cycles = new wxTextCtrl(window, -1);
	settings->Add(new wxStaticText(window, -1, "Enter Desired Number of Cycles:"), 0, wxALL, 12);
	settings->Add(cycles, 0, wxALL, 10);

	//wxBoxSizer *settings2 = new wxBoxSizer(wxHORIZONTAL);

	//force = new wxTextCtrl(window, -1);
	//settings2->Add(new wxStaticText(window, -1, "Enter Max Allowable Force(N):      "), 0, wxALL, 12);
	//settings2->Add(force, 0, wxALL, 10);

	//diameter = new wxTextCtrl(window, -1);
	//settings2->Add(new wxStaticText(window, -1, "Enter Tip Diameter(mm):       "), 0, wxALL, 12);
	//settings2->Add(diameter, 0, wxALL, 10);

	wxButton *settingsbut = new wxButton(window, -1, "Enter Settings");
	//settings2->Add(settingsbut, 0, wxALL, 10);
	settings->Add(settingsbut, 0, wxALL, 10);
	settingsbut->Bind(wxEVT_BUTTON, &MyFrame::OnSettingButtonClicked, this);
	//End of settings

	//Start of Graph and text panel
	//wxNotebook *notebook = new wxNotebook(window, wxID_ANY);
	notebook = new wxNotebook(window, wxID_ANY);
	wxBoxSizer *data = new wxBoxSizer(wxHORIZONTAL);

	//Displacement vs Time
	wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	m_plot = new mpWindow(notebook, -1, wxDefaultPosition, wxSize(500, 300), wxSUNKEN_BORDER);
	mpScaleX* xaxis = new mpScaleX(wxT("Time"), mpALIGN_BOTTOM, true, mpX_NORMAL);
	mpScaleY* yaxis = new mpScaleY(wxT("Displacement"), mpALIGN_LEFT, true);
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

	//Force vs Time
	m_plot2 = new mpWindow(notebook, -1, wxDefaultPosition, wxSize(500, 300), wxSUNKEN_BORDER);
	
	vectorLayer2 = new mpFXYVector(_("Vector"));
	mpScaleX* xaxis2 = new mpScaleX(wxT("Time"), mpALIGN_BOTTOM, true, mpX_NORMAL);
	mpScaleY* yaxis2 = new mpScaleY(wxT("Force"), mpALIGN_LEFT, true);
	xaxis2->SetFont(graphFont);
	yaxis2->SetFont(graphFont);
	xaxis2->SetDrawOutsideMargins(false);
	yaxis2->SetDrawOutsideMargins(false);
	m_plot2->SetMargins(50, 50, 50, 100);
	m_plot2->AddLayer(xaxis2);
	m_plot2->AddLayer(yaxis2);
	m_plot2->AddLayer(vectorLayer2);

	vectorLayer2->SetData(vectorTime, vectorForce);
	vectorLayer2->SetContinuity(true);
	vectorLayer2->SetPen(vectorpen);
	vectorLayer2->SetDrawOutsideMargins(false);
	


	//Force vs Displacement
	m_plot3 = new mpWindow(notebook, -1, wxDefaultPosition, wxSize(500, 300), wxSUNKEN_BORDER);
	
	vectorLayer3 = new mpFXYVector(_("Vector"));
	mpScaleX* xaxis3 = new mpScaleX(wxT("Displacement"), mpALIGN_BOTTOM, true, mpX_NORMAL);
	mpScaleY* yaxis3 = new mpScaleY(wxT("Force"), mpALIGN_LEFT, true);
	xaxis3->SetFont(graphFont);
	yaxis3->SetFont(graphFont);
	xaxis3->SetDrawOutsideMargins(false);
	yaxis3->SetDrawOutsideMargins(false);
	m_plot3->SetMargins(50, 50, 50, 100);
	m_plot3->AddLayer(xaxis3);
	m_plot3->AddLayer(yaxis3);
	m_plot3->AddLayer(vectorLayer3);

	vectorLayer3->SetData(vectorDisplacement, vectorForce);
	vectorLayer3->SetContinuity(true);
	vectorLayer3->SetPen(vectorpen);
	vectorLayer3->SetDrawOutsideMargins(false);
	

	m_data = new wxGrid(notebook, -1, wxDefaultPosition, wxSize(500, 300));
	indentorTable *table = new indentorTable();
	m_data->SetTable(table, true);
	m_data->AppendCols(3);
	m_data->EnableEditing(false);
	m_data->SetColLabelValue(0, "Time(s)");
	m_data->SetColLabelValue(1, "Displacement(mm)");
	m_data->SetColLabelValue(2, "Force(mN)");
	m_data->Fit();

	notebook->AddPage(m_plot, wxT("Displacement vs Time"));
	notebook->AddPage(m_plot2, wxT("Force vs Time"));
	notebook->AddPage(m_plot3, wxT("Force vs Displacement"));
	notebook->AddPage(m_data, wxT("Table"));
	data->Add(notebook, 1, wxGROW | wxALL, 10);
	//End of Graph and text panel

	//Start of Control Buttons (not include setting button)

	static const wxString label[] = { "Up","Down"};
	static const wxString label2[] = { "0.01mm","0.1mm","1mm" };
	wxBoxSizer *control = new wxBoxSizer(wxVERTICAL);
	wxButton *cancel = new wxButton(window, -1, "Cancel");
	wxButton *run = new wxButton(window, -1, "Run");
	jogDirection= new wxRadioBox(window, -1, "Jog Direction", wxDefaultPosition, wxDefaultSize, 2, label, 1);
	jogMode = new wxRadioBox(window, -1, "Jog Step", wxDefaultPosition, wxDefaultSize, 3, label2, 1);
	control->Add(jogDirection, 0, wxALL, 10);
	control->Add(jogMode, 0, wxALL, 10);
	wxButton *jog = new wxButton(window, -1, "Jog Indentor");
	control->Add(jog, 0, wxALL, 10);
	control->Add(cancel, 0, wxALL, 10);
	cancel->Bind(wxEVT_BUTTON, &MyFrame::OnCancelButtonClicked, this);
	control->Add(run, 0, wxALL, 10);
	run->Bind(wxEVT_BUTTON, &MyFrame::OnRunButtonClicked, this);
	jog->Bind(wxEVT_BUTTON, &MyFrame::OnJogButtonClicked, this);

	//End of Control Buttons (not include setting button)

	data->Add(control, 0, wxALIGN_BOTTOM);
	topsizer->Add(settings, 0);
	//topsizer->Add(settings2, 0);
	topsizer->Add(data, 1, wxEXPAND);

	window->SetSizerAndFit(topsizer);
}

MyFrame::~MyFrame()//GUI's destructor
{

}

wxThread::ExitCode MyFrame::EntrySerial()//Background threads function
{
	//Gets data for graph and chart
	//Right now it internally creates a array of data that continually updates the graph since no finished prototype
	//Also recieves serial input and then  updates the first cell of the chart with the data
	while (!serial->TestDestroy()) {
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
			try {
				int error = RS232_OpenComport(port, bdrate, mode);
				if (error == 1) {
					throw std::exception();
				}
				else {
					IndentorMode = "Idle";
				}
			}
			catch (std::exception& e) {

			}
		}
		else if (IndentorMode == "Run") {
			int amountSerialRecieved;
			if(buff[0]!='|')
				amountSerialRecieved=RS232_PollComport(port, buff, 1);
			if (buff[0] == '|') {
				int i = 0;
				wxString data[3] = {};
				long time;
				double value;
				bool valid = true;
				do {
					amountSerialRecieved=RS232_PollComport(port, buff, 1);
					if (amountSerialRecieved != 0) {
						if (((buff[0] < 58 && buff[0]>28) || buff[0] == 46 || buff[0]==45) && i<3) {
							data[i].Append(buff[0]);
						}
						else if (buff[0] == '_') {
							i++;
						}
						else if (buff[0] != '\\' || i>=3) {
							valid = false;
							break;
						}
					}
				}while(buff[0] != '\\');
				if (data[0] == "" || data[1] == "" || data[2] == "")
					valid = false;
				if (valid == true) {
					data[0].ToDouble(&value);
					vectorTime.push_back(value/1000);
					data[1].ToDouble(&value);
					vectorDisplacement.push_back(value);
					data[2].ToDouble(&value);
					vectorForce.push_back(value);
				}
			}
			if (buff[0] == 'F' && amountSerialRecieved!=0) {
				IndentorMode = "Finished";
			}
		}
		else if (IndentorMode == "SendSettings") {
			RS232_SendBuf(port,(unsigned char*)(settings.mb_str()).data(), 20);
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
		else if (IndentorMode == "JOG") {
			if (jogMode->GetSelection()==0 && jogDirection->GetSelection()==0) {
				RS232_SendBuf(port, (unsigned char*)"JOG_SMA_UP", 10);
			}
			else if (jogMode->GetSelection() == 0 && jogDirection->GetSelection() == 1) {
				RS232_SendBuf(port, (unsigned char*)"JOG_SMA_DO", 10);
			}
			else if (jogMode->GetSelection() == 1 && jogDirection->GetSelection() == 0) {
				RS232_SendBuf(port, (unsigned char*)"JOG_MED_UP", 10);
			}
			else if (jogMode->GetSelection() == 1 && jogDirection->GetSelection() == 1) {
				RS232_SendBuf(port, (unsigned char*)"JOG_MED_DO", 10);
			}
			else if (jogMode->GetSelection() == 2 && jogDirection->GetSelection() == 0) {
				RS232_SendBuf(port, (unsigned char*)"JOG_LAR_UP", 10);
			}
			else if (jogMode->GetSelection() == 2 && jogDirection->GetSelection() == 1) {
				RS232_SendBuf(port, (unsigned char*)"JOG_LAR_DO", 10);
			}
			IndentorMode = "Idle";
		}
		else if(IndentorMode =="Finished") {
			updatePlots = true;
			IndentorMode = "Idle";
		}
		else {

		}
	}
	return (wxThread::ExitCode)0;
}

wxThread::ExitCode MyFrame::EntryData()
{
	while (!data->TestDestroy()) {
		if (updatePlots==true) {
			vectorLayer->SetData(vectorTime, vectorDisplacement);
			m_plot->Fit();
			vectorLayer2->SetData(vectorTime, vectorForce);
			m_plot2->Fit();
			vectorLayer3->SetData(vectorDisplacement, vectorForce);
			m_plot3->Fit();
			updatePlots = false;
		}else if ((vectorDisplacement.size() + 1) % 50 == 0 && notebook->GetSelection() == 0) {
			vectorLayer->SetData(vectorTime, vectorDisplacement);
			m_plot->Fit();
		}else if ((vectorForce.size() + 1) % 50 == 0 && notebook->GetSelection() == 1) {
			vectorLayer2->SetData(vectorTime, vectorForce);
			m_plot2->Fit();
		}else if ((vectorForce.size() + 1) % 50 == 0 && notebook->GetSelection() == 2) {
			vectorLayer3->SetData(vectorDisplacement, vectorForce);
			m_plot3->Fit();
		}else if (row < vectorTime.size() && row < vectorDisplacement.size() && row < vectorForce.size()&&clearing==false) {
			m_data->AppendRows(1);
			m_data->SetCellValue(row, 0, wxString::Format("%f",vectorTime[row]));
			m_data->SetCellValue(row, 1, wxString::Format("%f", vectorDisplacement[row]));
			m_data->SetCellValue(row, 2, wxString::Format("%f", vectorForce[row]));
			row++;
		}
	}
	return (wxThread::ExitCode)0;
}

void MyFrame::OnClose(wxCloseEvent& event)//GUI close event
{
	//Closes thread and serial port
	//Then closes GUI's window
	if (serial && serial->IsRunning())
		serial->Wait();
	if (data && data->IsRunning())
		data->Wait();
	RS232_CloseComport(port);
	Destroy();
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
	//settingValues.Append(frequency->GetValue());
	//settingValues.Append("Hz frequency \n");
	settingValues.Append(cycles->GetValue());
	settingValues.Append(" cycles \n");
	//settingValues.Append(force->GetValue());
	//settingValues.Append("N maximum force\n");
	//settingValues.Append(diameter->GetValue());
	//settingValues.Append("mm diameter tip");

	if(wxMessageBox(settingValues, "Settings Entered", wxOK | wxCANCEL)==wxOK){
		settings = "|";
		settings.Append(cycles->GetValue());
		settings.Append("_");
		settings.Append(displacement->GetValue());
		settings.Append("\\");
		IndentorMode = "SendSettings";
	}
}

void MyFrame::OnCancelButtonClicked(wxCommandEvent&)//On cancel button pressed
{
	IndentorMode = "STOP";
}

void MyFrame::OnRunButtonClicked(wxCommandEvent&)//Event for start button pressed
{
	if (settings != "|_\\") {
		clearing = true;
		dynamic_cast<indentorTable *>(m_data->GetTable())->EmptyGrid();
		row = 0;
		vectorTime.clear();
		vectorDisplacement.clear();
		vectorForce.clear();
		RS232_flushRX(port);
		clearing = false;
		IndentorMode = "START";
	}
	else {
		wxMessageBox("Please Enter Valid Settings", "Error", wxOK|wxICON_ERROR);
	}
}

//Serial thread functions
wxThread* serialThread::StartAlongTaskSerial()//Creates background thread
{
	if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
	{
		wxLogError("Could not create the worker thread!");
		return NULL;
	}
	if (GetThread()->Run() != wxTHREAD_NO_ERROR)
	{
		wxLogError("Could not run the worker thread!");
		return NULL;
	}
	return GetThread();
}

void MyFrame::OnJogButtonClicked(wxCommandEvent&)//On cancel button pressed
{
	IndentorMode = "JOG";
}

//Data thread functions
wxThread* dataThread::StartAlongTaskData()//Creates background thread
{
	if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
	{
		wxLogError("Could not create the worker thread!");
		return NULL;
	}
	if (GetThread()->Run() != wxTHREAD_NO_ERROR)
	{
		wxLogError("Could not run the worker thread!");
		return NULL;
	}
	return GetThread();
}

//App class
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