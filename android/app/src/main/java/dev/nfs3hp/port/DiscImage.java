package dev.nfs3hp.port;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/** A CD image read in place: the ISO 9660 file system of its data track, so the
 *  launcher can copy the game's folders out of it.  An .iso holds the 2048
 *  bytes of each sector's data alone; a .bin (or .img) of a CUE sheet holds
 *  whole 2352-byte sectors -- sync, header, data, error correction -- of a
 *  MODE1 track (data at 16) or a MODE2 form 1 one (data at 24).  Which it is
 *  comes from where the volume descriptor turns up, so the CUE sheet itself is
 *  not needed.  Only the primary volume's names are read: the game's are 8.3
 *  anyway, and the copies are written in lower case, as the game expects. */
final class DiscImage
{
    static final int DATA_SIZE = 2048;

    /** The image stops before a sector the file system points at: a copy cut
     *  short.  DataImporter says so in the player's language. */
    static final class TruncatedException extends IOException
    {
        TruncatedException()
        {
            super("The disc image ends early");
        }
    }

    /** One name in a directory: a file or a folder, its extents in order. */
    static final class Entry
    {
        final String name;
        final boolean directory;
        final List<long[]> extents = new ArrayList<>();  // { first sector, bytes }
        long size;

        Entry(String name, boolean directory)
        {
            this.name = name;
            this.directory = directory;
        }
    }

    /** Sector size and where the data starts in a sector, tried in turn. */
    private static final int[][] LAYOUTS = { { 2048, 0 }, { 2352, 16 }, { 2352, 24 }, { 2336, 8 } };

    private final FileChannel channel;
    private final int sectorSize;
    private final int dataOffset;
    private final Entry root;

    private DiscImage(FileChannel channel, int sectorSize, int dataOffset, Entry root)
    {
        this.channel = channel;
        this.sectorSize = sectorSize;
        this.dataOffset = dataOffset;
        this.root = root;
    }

    /** The image's file system, or null if no ISO 9660 volume is found in it. */
    static DiscImage open(FileChannel channel) throws IOException
    {
        for (int[] layout : LAYOUTS)
        {
            byte[] descriptor = new byte[DATA_SIZE];
            if (!readFully(channel, 16L * layout[0] + layout[1], descriptor, 0, DATA_SIZE))
                continue;
            if (descriptor[0] != 1 || !"CD001".equals(new String(descriptor, 1, 5, StandardCharsets.US_ASCII)))
                continue;
            Entry root = new Entry("", true);
            root.extents.add(new long[] { le32(descriptor, 156 + 2), le32(descriptor, 156 + 10) });
            root.size = le32(descriptor, 156 + 10);
            return new DiscImage(channel, layout[0], layout[1], root);
        }
        return null;
    }

    /** Whether the file starts like a CUE sheet: text naming the BIN it is for. */
    static boolean looksLikeCueSheet(FileChannel channel) throws IOException
    {
        byte[] head = new byte[512];
        int n = channel.read(ByteBuffer.wrap(head), 0);
        if (n <= 0)
            return false;
        String text = new String(head, 0, n, StandardCharsets.ISO_8859_1).toUpperCase(Locale.ROOT);
        return text.contains("FILE") && text.contains("TRACK");
    }

    Entry root()
    {
        return root;
    }

    /** The names in a folder, without "." and "..", a file in several extents
     *  as one entry. */
    List<Entry> list(Entry folder) throws IOException
    {
        List<Entry> entries = new ArrayList<>();
        byte[] data = readAll(folder);
        Entry continued = null;
        int at = 0;
        while (at < data.length)
        {
            int length = data[at] & 0xff;
            if (length == 0)
            {
                // Records never cross a sector: the rest of this one is padding.
                at = (at / DATA_SIZE + 1) * DATA_SIZE;
                continue;
            }
            if (at + 33 > data.length || at + length > data.length)
                break;
            long extent = le32(data, at + 2);
            long size = le32(data, at + 10);
            int flags = data[at + 25] & 0xff;
            int nameLength = data[at + 32] & 0xff;
            String name = recordName(data, at + 33, nameLength);
            at += length;
            if (name == null)
                continue;
            Entry entry;
            if (continued != null && continued.name.equals(name))
                entry = continued;
            else
            {
                entry = new Entry(name, (flags & 0x02) != 0);
                entries.add(entry);
            }
            entry.extents.add(new long[] { extent, size });
            entry.size += size;
            continued = (flags & 0x80) != 0 ? entry : null;
        }
        return entries;
    }

    /** A folder's entry by name, without regard to case; null if not there. */
    Entry find(Entry folder, String name) throws IOException
    {
        for (Entry entry : list(folder))
            if (entry.name.equalsIgnoreCase(name))
                return entry;
        return null;
    }

    /** A file's bytes, from its extents in order. */
    InputStream open(Entry file)
    {
        return new InputStream()
        {
            private int extent = 0;
            private long offset = 0;  // into the current extent
            private final byte[] one = new byte[1];

            @Override public int read() throws IOException
            {
                return read(one, 0, 1) < 0 ? -1 : one[0] & 0xff;
            }

            @Override public int read(byte[] buffer, int start, int count) throws IOException
            {
                while (extent < file.extents.size() && offset >= file.extents.get(extent)[1])
                {
                    ++extent;
                    offset = 0;
                }
                if (extent >= file.extents.size())
                    return -1;
                long[] current = file.extents.get(extent);
                int n = (int) Math.min(count, current[1] - offset);
                readData(current[0], offset, buffer, start, n);
                offset += n;
                return n;
            }
        };
    }

    /** All of an entry's bytes: a folder's records. */
    private byte[] readAll(Entry entry) throws IOException
    {
        if (entry.size > 16L * 1024 * 1024)
            throw new IOException("Folder record too large: " + entry.name);
        byte[] data = new byte[(int) entry.size];
        int at = 0;
        for (long[] extent : entry.extents)
        {
            readData(extent[0], 0, data, at, (int) extent[1]);
            at += (int) extent[1];
        }
        return data;
    }

    /** `count` bytes of data from `offset` into the extent starting at sector
     *  `first`, across as many sectors as they lie in. */
    private void readData(long first, long offset, byte[] buffer, int start, int count) throws IOException
    {
        if (sectorSize == DATA_SIZE)
        {
            if (!readFully(channel, first * DATA_SIZE + offset, buffer, start, count))
                throw new TruncatedException();
            return;
        }
        while (count > 0)
        {
            long sector = first + offset / DATA_SIZE;
            int within = (int) (offset % DATA_SIZE);
            int n = Math.min(count, DATA_SIZE - within);
            if (!readFully(channel, sector * sectorSize + dataOffset + within, buffer, start, n))
                throw new TruncatedException();
            offset += n;
            start += n;
            count -= n;
        }
    }

    private static boolean readFully(FileChannel channel, long position, byte[] buffer, int start, int count)
        throws IOException
    {
        ByteBuffer target = ByteBuffer.wrap(buffer, start, count);
        while (target.hasRemaining())
        {
            int n = channel.read(target, position + (target.position() - start));
            if (n < 0)
                return false;
        }
        return true;
    }

    /** A record's name as the copy is called: no ";1" version, no "." left by
     *  a name without an extension; null for "." and "..". */
    private static String recordName(byte[] data, int at, int length)
    {
        if (length == 1 && (data[at] == 0 || data[at] == 1))
            return null;
        String name = new String(data, at, length, StandardCharsets.ISO_8859_1);
        int version = name.indexOf(';');
        if (version >= 0)
            name = name.substring(0, version);
        if (name.endsWith("."))
            name = name.substring(0, name.length() - 1);
        return name.isEmpty() ? null : name;
    }

    private static long le32(byte[] data, int at)
    {
        return (data[at] & 0xffL) | (data[at + 1] & 0xffL) << 8 | (data[at + 2] & 0xffL) << 16
            | (data[at + 3] & 0xffL) << 24;
    }
}
