#ifndef __QT__ // Qt doesn't expose SQLite VFS

#include <mbgl/test/sqlite3_test_fs.hpp>

#include <sqlite3.h>

#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <cassert>

static bool sqlite3_test_fs_debug = false;
static bool sqlite3_test_fs_io = true;
static bool sqlite3_test_fs_file_open = true;
static bool sqlite3_test_fs_file_create = true;
static int64_t sqlite3_test_fs_read_limit = -1;
static int64_t sqlite3_test_fs_write_limit = -1;

static sqlite3_vfs* unix_fs;

static const sqlite3_io_methods* unix_fs_methods;

static int sqlite3_test_fs_close(sqlite3_file* file) {
    if (sqlite3_test_fs_debug) {
        fprintf(stderr, "SQLite3: close(%p)\n", file);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs_methods->xClose(file);
}

static int sqlite3_test_fs_read(sqlite3_file* file, void* ptr, int iAmt, sqlite3_int64 iOfst) {
    if (sqlite3_test_fs_debug) {
        fprintf(stderr, "SQLite3: read(%p, amount=%d, offset=%lld)\n", file, iAmt, iOfst);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    if (sqlite3_test_fs_read_limit >= 0) {
        if (iAmt > sqlite3_test_fs_read_limit) {
            iAmt = 0;
            return SQLITE_IOERR;
        }
        sqlite3_test_fs_read_limit -= iAmt;
    }
    return unix_fs_methods->xRead(file, ptr, iAmt, iOfst);
}

static int sqlite3_test_fs_write(sqlite3_file* file, const void* ptr, int iAmt, sqlite3_int64 iOfst) {
    if (sqlite3_test_fs_debug) {
        fprintf(stderr, "SQLite3: write(%p, amount=%d, offset=%lld)\n", file, iAmt, iOfst);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    if (sqlite3_test_fs_write_limit >= 0) {
        if (iAmt > sqlite3_test_fs_write_limit) {
            iAmt = 0;
            return SQLITE_FULL;
        }
        sqlite3_test_fs_write_limit -= iAmt;
    }
    return unix_fs_methods->xWrite(file, ptr, iAmt, iOfst);
}

static int sqlite3_test_fs_truncate(sqlite3_file* file, sqlite3_int64 size) {
    if (sqlite3_test_fs_debug) {
        // fprintf(stderr, "SQLite3: truncate(%p, size=%lld)\n", file, size);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs_methods->xTruncate(file, size);
}

static int sqlite3_test_fs_sync(sqlite3_file* file, int flags) {
    if (sqlite3_test_fs_debug) {
        // fprintf(stderr, "SQLite3: sync(%p, flags=%d)\n", file, flags);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs_methods->xSync(file, flags);
}

static int sqlite3_test_fs_file_size(sqlite3_file* file, sqlite3_int64* pSize) {
    if (sqlite3_test_fs_debug) {
        // fprintf(stderr, "SQLite3: file_size(%p)\n", file);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs_methods->xFileSize(file, pSize);
}

static int sqlite3_test_fs_lock(sqlite3_file* file, int lockType) {
    if (sqlite3_test_fs_debug) {
        // fprintf(stderr, "SQLite3: lock(%p, type=%d)\n", file, lockType);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs_methods->xLock(file, lockType);
}

static int sqlite3_test_fs_unlock(sqlite3_file* file, int lockType) {
    if (sqlite3_test_fs_debug) {
        // fprintf(stderr, "SQLite3: unlock(%p, type=%d)\n", file, lockType);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs_methods->xUnlock(file, lockType);
}

static int sqlite3_test_fs_check_reserved_lock(sqlite3_file* file, int* pResOut) {
    if (sqlite3_test_fs_debug) {
        // fprintf(stderr, "SQLite3: check_reserved_lock(%p)\n", file);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs_methods->xCheckReservedLock(file, pResOut);
}

static int sqlite3_test_fs_file_control(sqlite3_file* file, int op, void* pArg) {
    if (sqlite3_test_fs_debug) {
        // fprintf(stderr, "SQLite3: file_control(%p, op=%d)\n", file, op);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs_methods->xFileControl(file, op, pArg);
}

static int sqlite3_test_fs_sector_size(sqlite3_file* file) {
    if (sqlite3_test_fs_debug) {
        // fprintf(stderr, "SQLite3: sector_size(%p)\n", file);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs_methods->xSectorSize(file);
}

static int sqlite3_test_fs_device_characteristics(sqlite3_file* file) {
    if (sqlite3_test_fs_debug) {
        // fprintf(stderr, "SQLite3: device_characteristics(%p)\n", file);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs_methods->xDeviceCharacteristics(file);
}

static sqlite3_vfs test_fs;

static sqlite3_io_methods sqlite3_test_fs_methods = {
    1,
    sqlite3_test_fs_close,
    sqlite3_test_fs_read,
    sqlite3_test_fs_write,
    sqlite3_test_fs_truncate,
    sqlite3_test_fs_sync,
    sqlite3_test_fs_file_size,
    sqlite3_test_fs_lock,
    sqlite3_test_fs_unlock,
    sqlite3_test_fs_check_reserved_lock,
    sqlite3_test_fs_file_control,
    sqlite3_test_fs_sector_size,
    sqlite3_test_fs_device_characteristics,
    0,
    0,
    0,
    0,
    0,
    0
};

static int sqlite3_test_fs_open(sqlite3_vfs* vfs, const char* zName, sqlite3_file* file, int flags, int* pOutFlags) {
    if (sqlite3_test_fs_debug) {
        fprintf(stderr, "SQLite3: open(name=%s, flags=%d) -> %p\n", zName, flags, file);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    if (!sqlite3_test_fs_file_open) {
        return SQLITE_CANTOPEN;
    }
    if (!sqlite3_test_fs_file_create) {
        int res;
        const int result = unix_fs->xAccess(vfs, zName, SQLITE_ACCESS_EXISTS, &res);
        if (result != SQLITE_OK) {
            return result;
        }
        if (res != 1) {
            return SQLITE_CANTOPEN;
        }
    }
    const int status = unix_fs->xOpen(vfs, zName, file, flags, pOutFlags);
    if (status == SQLITE_OK) {
        unix_fs_methods = file->pMethods;
        file->pMethods = &sqlite3_test_fs_methods;
    }
    return status;
}

#include <stdio.h>

static int sqlite3_test_fs_delete(sqlite3_vfs* vfs, const char *zName, int syncDir) {
    if (sqlite3_test_fs_debug) {
        fprintf(stderr, "SQLite3: delete(name=%s, sync=%d)\n", zName, syncDir);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs->xDelete(vfs, zName, syncDir);
}

static int sqlite3_test_fs_access(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut) {
    if (sqlite3_test_fs_debug) {
        fprintf(stderr, "SQLite3: access(name=%s, flags=%d)\n", zName, flags);
    }
    if (!sqlite3_test_fs_io) {
        return SQLITE_AUTH;
    }
    return unix_fs->xAccess(vfs, zName, flags, pResOut);
}

static bool sqlite3_test_fs_initialized = false;

namespace mbgl {
namespace test {

SQLite3TestFS::SQLite3TestFS() {
    if (sqlite3_test_fs_initialized) {
        throw std::runtime_error("SQLite3 Test FS is already initialized");
    }

    unix_fs = sqlite3_vfs_find("unix");
    if (!unix_fs) {
        abort();
    }

    test_fs.iVersion = unix_fs->iVersion;
    test_fs.szOsFile = unix_fs->szOsFile;
    test_fs.mxPathname = unix_fs->mxPathname;
    test_fs.zName = "test_fs";
    test_fs.pAppData = unix_fs->pAppData;

    test_fs.xOpen = sqlite3_test_fs_open;
    test_fs.xDelete = sqlite3_test_fs_delete;
    test_fs.xAccess = sqlite3_test_fs_access;
    test_fs.xFullPathname = unix_fs->xFullPathname;
    test_fs.xDlOpen = unix_fs->xDlOpen;
    test_fs.xDlError = unix_fs->xDlError;
    test_fs.xDlSym = unix_fs->xDlSym;
    test_fs.xDlClose = unix_fs->xDlClose;
    test_fs.xRandomness = unix_fs->xRandomness;
    test_fs.xSleep = unix_fs->xSleep;
    test_fs.xCurrentTime = unix_fs->xCurrentTime;
    test_fs.xGetLastError = unix_fs->xGetLastError;
    test_fs.xCurrentTimeInt64 = unix_fs->xCurrentTimeInt64;
    test_fs.xSetSystemCall = unix_fs->xSetSystemCall;
    test_fs.xGetSystemCall = unix_fs->xGetSystemCall;
    test_fs.xNextSystemCall = unix_fs->xNextSystemCall;

    sqlite3_vfs_register(&test_fs, 0);

    sqlite3_test_fs_initialized = true;
}

SQLite3TestFS::~SQLite3TestFS() {
    assert(sqlite3_test_fs_initialized);
    reset();
    sqlite3_vfs_unregister(&test_fs);
    sqlite3_test_fs_initialized = false;
}

void SQLite3TestFS::setDebug(bool value) {
    sqlite3_test_fs_debug = value;
}

void SQLite3TestFS::allowIO(bool value) {
    sqlite3_test_fs_io = value;
}

void SQLite3TestFS::allowFileOpen(bool value) {
    sqlite3_test_fs_file_open = value;
}

void SQLite3TestFS::allowFileCreate(bool value) {
    sqlite3_test_fs_file_create = value;
}

void SQLite3TestFS::setReadLimit(int64_t value) {
    sqlite3_test_fs_read_limit = value;
}

void SQLite3TestFS::setWriteLimit(int64_t value) {
    sqlite3_test_fs_write_limit = value;
}

void SQLite3TestFS::reset() {
    setDebug(false);
    allowIO(true);
    allowFileOpen(true);
    allowFileCreate(true);
    setReadLimit(-1);
    setWriteLimit(-1);
}

} // namespace test
} // namespace mbgl

#endif // __QT__
