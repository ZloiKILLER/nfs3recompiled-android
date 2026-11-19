#ifndef LIB_FILE_H_
#define LIB_FILE_H_

#include <lib/app.h>
#include <string>
#include <vector>


namespace nfs
{

/* A file found by FileEnumerator: its name, whether it is a directory, its size. */
struct FileEntry
{
    std::string name;
    bool directory = false;
    nfs::u32 size = 0;
};

class FileEnumerator : public GenericResource
{
public:
    FileEnumerator(const char* filename);
    ~FileEnumerator();

    bool findNext(FileEntry& entry);

private:
    std::string                     m_pattern;
    std::vector<FileEntry>          m_files;
    int                             m_index;
};

class File : public GenericResource
{
public:
    /* What a file is opened for. */
    enum class Mode
    {
        read,
        write,
        readWrite,
    };
    /* What an open does with a file that is there, or is not. */
    enum class Disposition
    {
        openExisting,       // opened if it is there, else not
        createNew,          // made if it is not there
        createAlways,       // made, or emptied if it is there
        openAlways,         // opened, made if it is not there
        truncateExisting,   // emptied if it is there, else not
    };

    File(int file, const char* name);
    File(const File& other);
    File(const char* path, Mode mode, Disposition disposition);
    ~File();

    nfs::u32 seek(nfs::u32 mode, nfs::s32 offset);
    nfs::u32 read(void* buffer, nfs::u32 max, nfs::u32* totalRead);
    nfs::u32 write(const void *buffer, nfs::u32 max, nfs::u32* totalWritten);
    nfs::u32 truncate();
    nfs::u32 size() const;

    static const std::string& getCurrentDirectory();
    static void setCurrentDirectory(const char* path);
    static void setDataDirectory(const char* path);
    static void setCdDirectory(const char* path);
    /* Called with the path of every file opened only to be read; a path back
     * is opened in its place, an empty string leaves the file as it is. */
    using ReadSubstitute = std::string (*)(const std::string& path);
    static void setReadSubstitute(ReadSubstitute substitute);
    static nfs::u32 remove(const char* filename);
    /* Creates the leaf only, and reports failure when the directory is
     * already there, exactly as the original's CreateDirectory does. */
    static nfs::u32 createDirectory(const char* path);

    operator void*() { return reinterpret_cast<void*>(static_cast<intptr_t>(m_file)+1); }
private:
    int m_file;
    std::string m_filename;
};

}

#endif
