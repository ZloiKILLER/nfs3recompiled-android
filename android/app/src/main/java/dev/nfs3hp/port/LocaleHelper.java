package dev.nfs3hp.port;

import android.content.Context;
import android.content.res.Configuration;
import android.os.LocaleList;

import java.util.Locale;

/** Applies the launcher's language preference to a context.
 *
 *  Every context that resolves a string has to go through this, including the
 *  application context: from API 25 on, overriding attachBaseContext of an
 *  Activity does not affect getApplicationContext(), so background work that
 *  formats a message would otherwise come back in the system language rather
 *  than the chosen one.  UI_LANGUAGE is written by the picker on the launcher's
 *  main screen; "system" leaves the context untouched. */
final class LocaleHelper
{
    private LocaleHelper() {}

    static Context wrap(Context context)
    {
        if (context == null)
            return null;
        String tag = GamePreferences.get(context)
            .getString(GamePreferences.UI_LANGUAGE, GamePreferences.UI_LANGUAGE_SYSTEM);
        if (GamePreferences.UI_LANGUAGE_SYSTEM.equals(tag))
            return context;
        Configuration config = new Configuration(context.getResources().getConfiguration());
        // setLocales, not setLocale: replacing only the primary locale leaves the
        // rest of the system list behind it, where it can still win the lookup.
        config.setLocales(new LocaleList(Locale.forLanguageTag(tag)));
        return context.createConfigurationContext(config);
    }
}
