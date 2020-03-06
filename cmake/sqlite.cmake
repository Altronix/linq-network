function(append_sqlite_compiler_flags dst)
  list(APPEND definitions "-DSQLITE_CORE")
  list(APPEND definitions "-DSQLITE_DEFAULT_FORIEGN_KEYS=1")
  list(APPEND definitions "-DSQLITE_ENABLE_API_ARMOR")
  list(APPEND definitions "-DSQLITE_ENABLE_COLUMN_METADATA")
  list(APPEND definitions "-DSQLITE_ENABLE_DBSTAT_VTAB")
  list(APPEND definitions "-DSQLITE_ENABLE_FTS3")
  list(APPEND definitions "-DSQLITE_ENABLE_FTS3_PARENTHESIS")
  list(APPEND definitions "-DSQLITE_ENABLE_FTS5")
  list(APPEND definitions "-DSQLITE_ENABLE_JSON1")
  list(APPEND definitions "-DSQLITE_ENABLE_LOAD_EXTENSION")
  list(APPEND definitions "-DSQLITE_ENABLE_MEMORY_MANAGEMENT")
  list(APPEND definitions "-DSQLITE_ENABLE_RTREE")
  list(APPEND definitions "-DSQLITE_ENABLE_STAT2")
  list(APPEND definitions "-DSQLITE_ENABLE_STAT4")
  list(APPEND definitions "-DSQLITE_SOUNDEX")
  list(APPEND definitions "-DSQLITE_THREADSAFE=1")
  list(APPEND definitions "-DSQLITE_USE_URI")
  list(APPEND definitions "-DHAVE_USLEEP=1")
  set(${dst} ${definitions} PARENT_SCOPE)
endfunction()
