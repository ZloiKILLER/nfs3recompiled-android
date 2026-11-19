#ifndef LIB_SETTINGS_H_
#define LIB_SETTINGS_H_

#include <lib/app.h>
#include <algorithm>
#include <cctype>
#include <string>
#include <map>

namespace nfs
{

/* The port's table of the settings the game looks up: the keys and names are
 * the ones it asks for (the paths it kept under the Windows registry), the
 * answers the port's.  A key or a setting is a resource of the application,
 * as it has always been, so the handles the rest gets keep their numbers.
 * The three tops a key hangs from, at the handles they always had. */
enum class SettingsRoot : nfs::u32
{
    classes         = 0x80000000,
    currentUser     = 0x80000001,
    localMachine    = 0x80000002,
};

/* A setting: a number or a text. */
class SettingValue : public GenericResource
{
public:
    explicit SettingValue(nfs::u32 value);
    explicit SettingValue(const std::string& value);
    explicit SettingValue(const char* value) : SettingValue(std::string(value)) {}

    bool isNumber() const { return m_isNumber; }
    nfs::u32 number() const { return m_number; }
    const std::string& text() const { return m_text; }

private:
    bool        m_isNumber;
    nfs::u32    m_number;
    std::string m_text;
};

/* A key: its keys and its settings by name, whatever their case. */
class SettingsKey : public GenericResource
{
public:
    SettingsKey();
    ~SettingsKey();

    void addChild(const std::string& name, SettingsKey* key);
    void addValue(const std::string& name, SettingValue* value);
    SettingsKey* getChild(const std::string& name) const;
    SettingValue* getValue(const std::string& name) const;

private:
    struct ci_less
    {
        // case-independent (ci) compare_less binary function
        struct nocase_compare
        {
            bool operator() (const unsigned char& c1, const unsigned char& c2) const
            {
                return tolower (c1) < tolower (c2);
            }
        };

        bool operator() (const std::string& s1, const std::string& s2) const
        {
            return std::lexicographical_compare(s1.begin(), s1.end(),
                                                s2.begin(), s2.end(),
                                                nocase_compare());
        }
    };
    std::map< std::string, SettingsKey*, ci_less >      m_children;
    std::map< std::string, SettingValue*, ci_less >     m_values;
};

}

#endif
