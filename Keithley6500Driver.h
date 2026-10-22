#pragma once
#include "IDMMDriver.h"
#include <visa.h>
#include <iostream>
#include <string>

class Keithley6500Driver : public IDMMDriver {
private:
    ViSession defaultRM = VI_NULL;
    ViSession viSession = VI_NULL;

    bool SendSCPI(const std::string& command) {
        if (viSession == VI_NULL) return false;
        std::string cmd = command + "\n";
        ViUInt32 writeCount;
        ViStatus status = viWrite(viSession, (ViBuf)cmd.c_str(), (ViUInt32)cmd.length(), &writeCount);
        return status >= VI_SUCCESS;
    }

    std::string QuerySCPI(const std::string& command) {
        if (!SendSCPI(command)) return "";
        char buffer[256] = {0};
        ViUInt32 retCount;
        viRead(viSession, (ViBuf)buffer, sizeof(buffer) - 1, &retCount);
        return std::string(buffer);
    }

public:
    Keithley6500Driver() = default;
    ~Keithley6500Driver() { Disconnect(); }

    bool Connect(const std::string& visaAddress) override {
        if (viOpenDefaultRM(&defaultRM) < VI_SUCCESS) return false;
        ViStatus status = viOpen(defaultRM, (ViRsrc)visaAddress.c_str(), VI_NULL, VI_NULL, &viSession);
        
        if (status >= VI_SUCCESS) {
            SendSCPI("*LANG SCPI");
            return true;
        }
        return false;
    }

    void Disconnect() override {
        if (viSession != VI_NULL) viClose(viSession);
        if (defaultRM != VI_NULL) viClose(defaultRM);
        viSession = VI_NULL;
        defaultRM = VI_NULL;
    }

    std::string GetIdentity() override {
        return QuerySCPI("*IDN?");
    }

    bool Reset() override {
        return SendSCPI("*RST");
    }

    bool ConfigureMeasurement(MeasurementMode mode, double range) override {
        switch (mode) {
            case MeasurementMode::DC_VOLTAGE:
                return SendSCPI(":SENS:FUNC 'VOLT:DC'");
            case MeasurementMode::AC_VOLTAGE:
                return SendSCPI(":SENS:FUNC 'VOLT:AC'");
            case MeasurementMode::DC_CURRENT:
                return SendSCPI(":SENS:FUNC 'CURR:DC'");
            case MeasurementMode::AC_CURRENT:
                return SendSCPI(":SENS:FUNC 'CURR:AC'");
            case MeasurementMode::TWO_WIRE_RES:
                return SendSCPI(":SENS:FUNC 'RES'");
            case MeasurementMode::FOUR_WIRE_RES:
                return SendSCPI(":SENS:FUNC 'FRES'");
            case MeasurementMode::TEMPERATURE:
                return SendSCPI(":SENS:FUNC 'TEMP'");
            default:
                return false;
        }
    }

    double ReadValue() override {
        std::string result = QuerySCPI(":READ?");
        try {
            return std::stod(result);
        } catch (...) {
            return 0.0;
        }
    }

    std::string ExecuteCommand(const std::string& scpiCmd) override {
        if (scpiCmd.find('?') != std::string::npos) {
            return QuerySCPI(scpiCmd);
        } else {
            bool ok = SendSCPI(scpiCmd);
            return ok ? "Command Sent Successfully" : "Command Failed";
        }
    }
};