package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;

final class DataSetManager
{
    interface ImportOperation
    {
        void run(File temporaryRoot) throws IOException;
    }

    static final class DataSet
    {
        final String id;
        final String name;
        final boolean active;

        DataSet(String id, String name, boolean active)
        {
            this.id = id;
            this.name = name;
            this.active = active;
        }

        @Override
        public String toString()
        {
            return active ? name + " (active)" : name;
        }
    }

    private static final String STORAGE_DIR = ".data-sets";

    private final File root;
    private final File storage;
    private final SharedPreferences preferences;

    DataSetManager(Context context, File root) throws IOException
    {
        this.root = root;
        storage = new File(root, STORAGE_DIR);
        if (!storage.isDirectory() && !storage.mkdirs())
            throw new IOException("Could not create data-set storage");
        preferences = GamePreferences.get(context);
        registerExistingActiveData();
    }

    List<DataSet> list()
    {
        JSONObject names = readNames();
        String activeId = activeId();
        ArrayList<DataSet> result = new ArrayList<>();
        Iterator<String> ids = names.keys();
        while (ids.hasNext())
        {
            String id = ids.next();
            boolean active = id.equals(activeId);
            File location = active ? root : new File(storage, id);
            if (DataImporter.isUserDataPresent(location))
                result.add(new DataSet(id, names.optString(id, id), active));
        }
        result.sort((a, b) -> {
            if (a.active != b.active)
                return a.active ? -1 : 1;
            return a.name.compareToIgnoreCase(b.name);
        });
        return result;
    }

    DataSet active()
    {
        for (DataSet dataSet : list())
        {
            if (dataSet.active)
                return dataSet;
        }
        return null;
    }

    String importDataSet(String requestedName, ImportOperation operation) throws IOException
    {
        String name = cleanDisplayName(requestedName);
        String id = uniqueId(name);
        File temporary = new File(storage, "." + id + ".part");
        File destination = new File(storage, id);
        DataImporter.deleteRecursively(temporary);
        if (!temporary.mkdirs())
            throw new IOException("Could not create temporary import folder");

        try
        {
            operation.run(temporary);
            if (!DataImporter.isUserDataPresent(temporary))
                throw new IOException("imported data is missing expected files");
            Files.move(temporary.toPath(), destination.toPath(), StandardCopyOption.ATOMIC_MOVE);
        }
        finally
        {
            DataImporter.deleteRecursively(temporary);
        }

        JSONObject names = readNames();
        try
        {
            names.put(id, name);
        }
        catch (JSONException e)
        {
            throw new IOException("Could not save data-set name", e);
        }
        writeNames(names);
        if (activeId().isEmpty())
            activate(id);
        return id;
    }

    void activate(String id) throws IOException
    {
        String oldId = activeId();
        if (id.equals(oldId))
            return;

        File selected = new File(storage, id);
        if (!DataImporter.isUserDataPresent(selected))
            throw new IOException("The selected data set is incomplete");

        File parked = null;
        if (!oldId.isEmpty() && DataImporter.isUserDataPresent(root))
        {
            File parkedDestination = new File(storage, oldId);
            if (parkedDestination.exists())
                throw new IOException("Stored data set already exists: " + parkedDestination);
            parked = new File(storage, "." + oldId + ".part");
            DataImporter.deleteRecursively(parked);
            if (!parked.mkdirs())
                throw new IOException("Could not prepare the active data set for switching");
            DataImporter.moveUserData(root, parked);
        }

        try
        {
            DataImporter.moveUserData(selected, root);
            if (parked != null)
            {
                Files.move(parked.toPath(), new File(storage, oldId).toPath(),
                    StandardCopyOption.ATOMIC_MOVE);
            }
            preferences.edit().putString(GamePreferences.ACTIVE_DATA_SET_ID, id).apply();
            if (selected.exists() && !selected.delete())
                selected.deleteOnExit();
        }
        catch (IOException e)
        {
            if (!DataImporter.isUserDataPresent(root) && parked != null)
                DataImporter.moveUserData(parked, root);
            throw e;
        }
    }

    void delete(String id) throws IOException
    {
        if (id.equals(activeId()))
        {
            DataImporter.deleteUserData(root);
            preferences.edit().remove(GamePreferences.ACTIVE_DATA_SET_ID).apply();
        }
        else
        {
            DataImporter.deleteRecursively(new File(storage, id));
        }

        JSONObject names = readNames();
        names.remove(id);
        writeNames(names);

        List<DataSet> remaining = list();
        if (activeId().isEmpty() && !remaining.isEmpty())
            activate(remaining.get(0).id);
    }

    private void registerExistingActiveData() throws IOException
    {
        if (!DataImporter.isUserDataPresent(root) || !activeId().isEmpty())
            return;
        JSONObject names = readNames();
        String id = uniqueId("Imported data");
        try
        {
            names.put(id, "Imported data");
        }
        catch (JSONException e)
        {
            throw new IOException("Could not register existing game data", e);
        }
        preferences.edit()
            .putString(GamePreferences.DATA_SET_NAMES, names.toString())
            .putString(GamePreferences.ACTIVE_DATA_SET_ID, id)
            .apply();
    }

    private String activeId()
    {
        return preferences.getString(GamePreferences.ACTIVE_DATA_SET_ID, "");
    }

    private JSONObject readNames()
    {
        try
        {
            return new JSONObject(preferences.getString(GamePreferences.DATA_SET_NAMES, "{}"));
        }
        catch (JSONException e)
        {
            return new JSONObject();
        }
    }

    private void writeNames(JSONObject names)
    {
        preferences.edit().putString(GamePreferences.DATA_SET_NAMES, names.toString()).apply();
    }

    private String uniqueId(String name)
    {
        String base = name.toLowerCase(Locale.ROOT).replaceAll("[^a-z0-9]+", "-")
            .replaceAll("^-|-$", "");
        if (base.isEmpty())
            base = "data";
        String id = base;
        int suffix = 2;
        JSONObject names = readNames();
        while (names.has(id) || new File(storage, id).exists())
            id = base + "-" + suffix++;
        return id;
    }

    private static String cleanDisplayName(String name)
    {
        if (name == null)
            return "Imported data";
        String result = name.trim();
        if (result.toLowerCase(Locale.ROOT).endsWith(".zip"))
            result = result.substring(0, result.length() - 4).trim();
        return result.isEmpty() ? "Imported data" : result;
    }
}
