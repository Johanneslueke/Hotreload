//
// Created by johannes on 9/27/16.
//

#include "../hotreload.h"

#define ME_WARNING(type,msg,value,name,para)



ConfigSettingInt::ConfigSettingInt(const char* name, const char* synopsis, int initialValue) {
    this->AddToList();
    m_name = name;
    m_synopsis = synopsis;
    m_value = initialValue;

    m_max = 0;
    m_min = 0;
}

ConfigSettingInt::ConfigSettingInt(const char* name, const char* synopsis, int initialValue, int minValue, int maxValue) {

    this->AddToList();
    m_name = name;
    m_synopsis = synopsis;
    m_value = initialValue;

    m_max = maxValue;
    m_min = minValue;

}

ConfigSettingInt& ConfigSettingInt::operator=(int value) {
    if (value < m_min) {
        ME_WARNING("Config", "Could not set value %d for setting \"%s\" because it is smaller than the minimum. It will be set to %d.", value, m_name, m_min);
        value = m_min;
    } else if (value > m_max) {
        ME_WARNING("Config", "Could not set value %d for setting \"%s\" because it is greater than the maximum. It will be set to %d.", value, m_name, m_max);
        value = m_max;
    }

    m_value = value;
    return *this;
}

void ConfigSettingInt::AddToList(void) {
    if (head) {
        tail->m_next = this;
        tail = this;
    } else {
        head = this;
        tail = this;
    }
}

ConfigSettingInt* ConfigSettingInt::FindSetting(const char* name) {

    auto cursor = head;

    do
    {
        if(cursor->m_name == name)
            return cursor;
        else
            return cursor = cursor->m_next;
    }while(cursor != tail);

    return nullptr;

}

ConfigSettingInt::operator int() const{
    return m_value;
}

ConfigSettingString::ConfigSettingString(const char* name, const char* synopsis, const std::string& initialValue) {
    this->AddToList();
    m_name = name;
    m_synopsis = synopsis;
    m_value = initialValue;

}

void ConfigSettingString::AddToList() {
    if (head_s) {
        tail_s->m_next = this;
        tail_s = this;
    } else {
        head_s = this;
        tail_s = this;
    }

}

ConfigSettingString* ConfigSettingString::FindSetting(const char* name) {

    auto cursor = head_s;

    do {
        if (cursor != nullptr) {
            //DONE(hannes): if name is not found it still tries to access the next element. This is wrong
            if (cursor->m_name == name)
                return cursor;
            else
                return cursor = cursor->m_next;
        }
    }
    while (cursor != tail_s);

    return nullptr;
}

ConfigSettingString& ConfigSettingString::operator=(const std::string& value) {
    m_value = value;

    return *this;
}

ConfigSettingString::operator char*() const {
    return (char*)m_value.c_str();
}