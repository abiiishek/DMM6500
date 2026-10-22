#pragma once
#include "Keithley6500Driver.h"
#include <memory>

enum class DMMType {
    KEITHLEY_6500,
    KEYSIGHT_34461A
};

class DMMFactory {
public:
    static std::unique_ptr<IDMMDriver> CreateDriver(DMMType type) {
        switch (type) {
            case DMMType::KEITHLEY_6500:
                return std::make_unique<Keithley6500Driver>();
            default:
                return nullptr;
        }
    }
};