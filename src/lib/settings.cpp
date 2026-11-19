#include <lib/settings.h>

namespace nfs
{

SettingValue::SettingValue(nfs::u32 value)
    :   m_isNumber(true)
    ,   m_number(value)
{
}

SettingValue::SettingValue(const std::string& value)
    :   m_isNumber(false)
    ,   m_number(0)
    ,   m_text(value)
{
}

SettingsKey::SettingsKey()
{
}
SettingsKey::~SettingsKey()
{
}

void SettingsKey::addChild(const std::string& name, SettingsKey* key)
{
    m_children[name] = key;
}

SettingsKey* SettingsKey::getChild(const std::string& name) const
{
    std::map< std::string, SettingsKey*, ci_less >::const_iterator it = m_children.find(name);
    if (it != m_children.end())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

void SettingsKey::addValue(const std::string& name, SettingValue* value)
{
    m_values[name] = value;
}

SettingValue* SettingsKey::getValue(const std::string& name) const
{
    std::map< std::string, SettingValue*, ci_less >::const_iterator it = m_values.find(name);
    if (it != m_values.end())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

}
