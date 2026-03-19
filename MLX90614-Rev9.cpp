///////////////////////////////////////////////////////////////
// This is a C++ program for the MLX90614 temperature sensor //
// which uses a thread to get data and update the GUI        //
///////////////////////////////////////////////////////////////
#include <wx/wx.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <gpiod.h>
#include <linux/i2c-dev.h>
extern "C" {  //Prevents a C method mangling problem
    #include <i2c/smbus.h>
}
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>

 
class MyMLX90614App : public wxApp
{
public:
    bool OnInit() override;
};
 
wxIMPLEMENT_APP(MyMLX90614App);
 
class MyForm : public wxFrame
{
public:
    MyForm(); //Add constructor declaration
    ~MyForm(); // Add destructor declaration
 
    //Initialize member variables for I2C
    int MyChipHandle = 0;
    int I2C_ADDR = 0x5A;
    const char* I2C_BUS = "/dev/i2c-1";
    void MyThread();
    //Readable memory
    const uint8_t MLX90614_RAWIR1 = 0x04;
    const uint8_t MLX90614_RAWIR2 = 0x05;
    const uint8_t MLX90614_TA = 0x06;
    const uint8_t MLX90614_TOBJ1 = 0x07;
    const uint8_t MLX90614_TOBJ2 = 0x08;
    //EEPROM
    const uint8_t MLX90614_TOMAX = 0x20;
    const uint8_t MLX90614_TOMIN = 0x21;
    const uint8_t MLX90614_PWMCTRL = 0x22;
    const uint8_t MLX90614_TARANGE = 0x23;
    const uint8_t MLX90614_EMISS = 0x24;
    const uint8_t MLX90614_CONFIG = 0x25;
    const uint8_t MLX90614_ADDR = 0x2E;
    const uint8_t MLX90614_ID1 = 0x3C;
    const uint8_t MLX90614_ID2 = 0x3D;
    const uint8_t MLX90614_ID3 = 0x3E;
    const uint8_t MLX90614_ID4 = 0x3F;
private:
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnTemperatureButton(wxCommandEvent& event);
    wxTextCtrl *textArea;
    wxStaticText *label1;
};

enum
{
    ID_MLX90614 = 1,
    ID_TemperatureButton,
};
 
bool MyMLX90614App::OnInit()
{
    MyForm *form = new MyForm();
    form->Show(true);
    return true;
}

MyForm::MyForm()
    : wxFrame(nullptr, wxID_ANY, "MLX90614", wxDefaultPosition, wxSize(600, 306))
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_MLX90614, "&MLX90614\tCtrl-H",
                     "Press a button to get the temperature.");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
 
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
 
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
 
    SetMenuBar(menuBar);
 
    CreateStatusBar();
    SetStatusText("MLX90614 Program");
    ///////////////////////
    // Create main panel //
    ///////////////////////
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    
    ///////////////////
    //Create a label //
    ///////////////////
    label1 = new wxStaticText(panel, wxID_ANY, "Infrared Temperature Measurement", wxPoint(20, 0), wxSize(560, 20), wxALIGN_CENTER_HORIZONTAL);
                            
    ////////////////////////////////////////////////////
    // Create TextArea at specified position and size //
    ////////////////////////////////////////////////////
    textArea = new wxTextCtrl(panel, wxID_ANY, "",
                              wxPoint(20, 40),
                              wxSize(560, 118),
                              wxTE_MULTILINE | wxTE_READONLY);
    
    /////////////////////////////////////////////////////////
    // Create StaticBox (GroupBox) on the right - Get Info //
    /////////////////////////////////////////////////////////
    wxButton *temperatureButton = new wxButton(panel, ID_TemperatureButton, "Temperature", wxPoint(233, 190), wxSize(120, 26));
    temperatureButton->Raise();
   
    // Bind events
    Bind(wxEVT_MENU, &MyForm::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyForm::OnExit, this, wxID_EXIT);
    Bind(wxEVT_BUTTON, &MyForm::OnTemperatureButton, this, ID_TemperatureButton);
   
    ///////////////////////////////////////////////////
    // Initialize MLX90614 I2C - Open I2C connection //
    ///////////////////////////////////////////////////
    try {
        
        textArea->AppendText("MLX90614 initialized successfully\n");
        MyChipHandle = open(I2C_BUS, O_RDWR);
        if (MyChipHandle < 0) {
            throw std::runtime_error("Failed to open I2C device: " + std::string(I2C_BUS) + 
            " - " + std::string(strerror(errno)));
        }
        //Set I2C slave address
        if (ioctl(MyChipHandle, I2C_SLAVE, I2C_ADDR) < 0) {
            close(MyChipHandle);
            throw std::runtime_error("Failed to set I2C address: " + std::string(strerror(errno)));
        }
        
        textArea->AppendText(wxString::Format("MLX90614 initialized on %s at address 0x%02X\n", I2C_BUS, I2C_ADDR));

    } catch (const std::runtime_error& e) {
        wxMessageBox(wxString::Format("Failed to initialize MLX90614: %s", e.what()),
                     "Initialization Error", wxOK | wxICON_ERROR);
    }
}

///////////////////////////////////
// Add destructor implementation //
///////////////////////////////////
MyForm::~MyForm()
{
    // Clean up MLX90614 - destructor will close I2C connection
    if (MyChipHandle >= 0) {
        close(MyChipHandle);
        std::cout << "MLX90614 connection closed" << std::endl;
    }
}
 
void MyForm::OnExit(wxCommandEvent& event)
{
    // Destructor will be called automatically when window closes
    Close(true);
}

void MyForm::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a MLX90614 Infrared Temperature C++ example program",
                 "About MLX90614", wxOK | wxICON_INFORMATION);
}

void MyForm::OnTemperatureButton(wxCommandEvent& event)
{
    textArea->Clear();
    //Start a thread
    std::thread t(&MyForm::MyThread, this); //Start the thread
    t.detach(); //Run the thread independently
}

void get_obj_temp(int MyChipHandle1, uint8_t tobj1_reg, wxTextCtrl *TArea) {
    //Get the object temperature and show Celsius and Fahrenheit
    //Temperatures in text control
    if (MyChipHandle1 < 0) {
        throw std::runtime_error("Invalid file handle in get_obj_temp");
    }
    
    // Use SMBus word read
    int32_t raw = i2c_smbus_read_word_data(MyChipHandle1, tobj1_reg);
    if (raw < 0) {
        throw std::runtime_error("Failed to read object temperature from MLX90614");
    }
    
    uint16_t RawTemp = (uint16_t)raw;
    double TempC = (RawTemp * 0.02) - 273.15;
    
    TArea->AppendText("Object Temperature: ");
    std::stringstream ss;
    //Format Temperature(C) to two decimal points
    ss << std::fixed << std::setprecision(2) << TempC;
    TArea->AppendText(ss.str());
    TArea->AppendText(" Celsius, ");
    //Convert to Fahrenheit
    std::stringstream st;
    float_t Fahrenheit = TempC * 9/5 + 32;
    st << std::fixed << std::setprecision(2) << Fahrenheit;
    TArea->AppendText(st.str());
    TArea->AppendText(" Fahrenheit \n");    
}

void get_amb_temp(int MyChipHandle1, uint8_t tamb1_reg, wxTextCtrl *TArea) {
    //Get the ambient temperature and show the result in both
    //Celsius and Fahrenheit. Temperatures are shown in the text control.
    if (MyChipHandle1 < 0) {
        throw std::runtime_error("Invalid file handle in get_amb_temp");
    }
        // Use SMBus word read
    int32_t raw = i2c_smbus_read_word_data(MyChipHandle1, tamb1_reg);
    if (raw < 0) {
        throw std::runtime_error("Failed to read ambient temperature from MLX90614");
    }
    
    uint16_t RawTemp = (uint16_t)raw;
    double TempC = (RawTemp * 0.02) - 273.15;
    
    TArea->AppendText("Ambient Temperature: ");
    std::stringstream ss;
    //Format Temperature(C) to two decimal points
    ss << std::fixed << std::setprecision(2) << TempC;
    TArea->AppendText(ss.str());
    TArea->AppendText(" Celsius, ");
    //Convert to Fahrenheit
    std::stringstream st;
    float_t Fahrenheit = TempC * 9/5 + 32;
    st << std::fixed << std::setprecision(2) << Fahrenheit;
    TArea->AppendText(st.str());
    TArea->AppendText(" Fahrenheit \n");
}

void get_emissivity(int MyChipHandle1, uint8_t emiss_reg, wxTextCtrl *TArea) {
    if (MyChipHandle1 < 0) {
        throw std::runtime_error("Invalid file handle in get_emiss");
    }
    // Use SMBus word read
    uint16_t raw = i2c_smbus_read_word_data(MyChipHandle1, emiss_reg);
    if (raw < 0) {
        throw std::runtime_error("Failed to read emissivity value from MLX90614");
    }
    
    uint16_t data = (uint16_t)raw;
    double_t Emiss = data/65535;
    TArea->AppendText("Emissivity: ");
    std::stringstream ss;
    //Format emissivity to one decimal point
    ss << std::fixed << std::setprecision(1) << Emiss;
    TArea->AppendText(ss.str());
    TArea->AppendText("  \n");
}

void MyForm::MyThread() {
    //Update GUI in here
    CallAfter([this]() {
        //Get Object Temperature 
        get_obj_temp(MyChipHandle, MLX90614_TOBJ1, textArea);
        //Get Ambient Temperature
        get_amb_temp(MyChipHandle, MLX90614_TA, textArea);
        //Get Emissivity value
        get_emissivity(MyChipHandle, MLX90614_EMISS, textArea);
    });
}
