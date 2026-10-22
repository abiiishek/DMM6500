#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "DMMFactory.h"

struct DMMDevice {
    int id;
    std::string address;
    std::unique_ptr<IDMMDriver> driver;
    std::string identity;
};

void PrintMenu(int activeIndex, size_t totalDevices) {
    std::cout << "\n=============================================" << std::endl;
    std::cout << "      MULTI-DMM ADVANCED CLI CONTROLLER      " << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << " Connected Devices: " << totalDevices << std::endl;
    if (activeIndex != -1) {
        std::cout << " Active DMM Index: [" << activeIndex << "]" << std::endl;
    } else {
        std::cout << " Active DMM Index: None Selected" << std::endl;
    }
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << " 1. Connect New DMM" << std::endl;
    std::cout << " 2. List Connected DMMs & Switch Active Device" << std::endl;
    std::cout << " 3. Measure DC Voltage" << std::endl;
    std::cout << " 4. Measure 2-Wire Resistance" << std::endl;
    std::cout << " 5. Measure 4-Wire Resistance" << std::endl;
    std::cout << " 6. Send Custom SCPI Command (NI MAX style)" << std::endl;
    std::cout << " 7. Read Value (:READ?)" << std::endl;
    std::cout << " 8. Reset Active DMM (*RST)" << std::endl;
    std::cout << " 0. Exit Application" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "Enter Choice: ";
}

int main() {
    std::vector<DMMDevice> dmmList;
    int activeIndex = -1;
    int choice = -1;

    while (choice != 0) {
        PrintMenu(activeIndex, dmmList.size());
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1: {
                std::cout << "Enter VISA Address (e.g. TCPIP0::192.168.1.100::inst0::INSTR): ";
                std::string addr;
                std::cin >> addr;

                auto newDMM = DMMFactory::CreateDriver(DMMType::KEITHLEY_6500);
                std::cout << "Connecting..." << std::endl;

                if (newDMM->Connect(addr)) {
                    std::string idn = newDMM->GetIdentity();
                    int newIndex = dmmList.size();
                    dmmList.push_back({newIndex, addr, std::move(newDMM), idn});
                    activeIndex = newIndex;
                    std::cout << "Connected! Assigned Index: " << newIndex << "\nID: " << idn << std::endl;
                } else {
                    std::cout << "Connection Failed for " << addr << std::endl;
                }
                break;
            }
            case 2: {
                if (dmmList.empty()) {
                    std::cout << "No DMMs connected!" << std::endl;
                    break;
                }
                std::cout << "\n--- Connected DMM Devices ---" << std::endl;
                for (size_t i = 0; i < dmmList.size(); ++i) {
                    std::cout << " [" << i << "] " << dmmList[i].address << " | " << dmmList[i].identity;
                    if ((int)i == activeIndex) std::cout << "  <-- ACTIVE";
                    std::cout << std::endl;
                }
                std::cout << "Enter Index to select Active DMM: ";
                int sel;
                if (std::cin >> sel && sel >= 0 && sel < (int)dmmList.size()) {
                    activeIndex = sel;
                    std::cout << "Active DMM switched to Index " << activeIndex << std::endl;
                } else {
                    std::cout << "Invalid Index!" << std::endl;
                }
                break;
            }
            case 3: {
                if (activeIndex == -1 || activeIndex >= (int)dmmList.size()) {
                    std::cout << "No Active DMM selected!" << std::endl;
                    break;
                }
                dmmList[activeIndex].driver->ConfigureMeasurement(MeasurementMode::DC_VOLTAGE);
                double v = dmmList[activeIndex].driver->ReadValue();
                std::cout << " -> DC Voltage: " << v << " V" << std::endl;
                break;
            }
            case 4: {
                if (activeIndex == -1 || activeIndex >= (int)dmmList.size()) {
                    std::cout << "No Active DMM selected!" << std::endl;
                    break;
                }
                dmmList[activeIndex].driver->ConfigureMeasurement(MeasurementMode::TWO_WIRE_RES);
                double r = dmmList[activeIndex].driver->ReadValue();
                std::cout << " -> 2-Wire Resistance: " << r << " Ohms" << std::endl;
                break;
            }
            case 5: {
                if (activeIndex == -1 || activeIndex >= (int)dmmList.size()) {
                    std::cout << "No Active DMM selected!" << std::endl;
                    break;
                }
                dmmList[activeIndex].driver->ConfigureMeasurement(MeasurementMode::FOUR_WIRE_RES);
                double r = dmmList[activeIndex].driver->ReadValue();
                std::cout << " -> 4-Wire Resistance: " << r << " Ohms" << std::endl;
                break;
            }
            case 6: { // NI MAX Style Direct SCPI Command
                if (activeIndex == -1 || activeIndex >= (int)dmmList.size()) {
                    std::cout << "No Active DMM selected!" << std::endl;
                    break;
                }
                std::cout << "Enter SCPI Command (e.g. :SENS:VOLT:DC:NPLC 10 or *IDN?): ";
                std::string cmd;
                std::cin.ignore(10000, '\n');
                std::getline(std::cin, cmd);

                std::string response = dmmList[activeIndex].driver->ExecuteCommand(cmd);
                std::cout << " -> Response: " << response << std::endl;
                break;
            }
            case 7: {
                if (activeIndex == -1 || activeIndex >= (int)dmmList.size()) {
                    std::cout << "No Active DMM selected!" << std::endl;
                    break;
                }
                double val = dmmList[activeIndex].driver->ReadValue();
                std::cout << " -> Reading: " << val << std::endl;
                break;
            }
            case 8: {
                if (activeIndex == -1 || activeIndex >= (int)dmmList.size()) {
                    std::cout << "No Active DMM selected!" << std::endl;
                    break;
                }
                dmmList[activeIndex].driver->Reset();
                std::cout << " -> DMM Reset Done!" << std::endl;
                break;
            }
            case 0: {
                std::cout << "Disconnecting all devices..." << std::endl;
                for (auto& dev : dmmList) {
                    dev.driver->Disconnect();
                }
                std::cout << "Exiting application. Bye!" << std::endl;
                break;
            }
            default:
                std::cout << "Invalid choice!" << std::endl;
                break;
        }
    }

    return 0;
}