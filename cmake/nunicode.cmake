macro(mbgl_nunicode_core)
    target_sources(mbgl-core
        PRIVATE nunicode/include/libnu/casemap.h
        PRIVATE nunicode/include/libnu/casemap_internal.h
        PRIVATE nunicode/include/libnu/config.h
        PRIVATE nunicode/include/libnu/defines.h
        PRIVATE nunicode/include/libnu/ducet.h
        PRIVATE nunicode/include/libnu/libnu.h
        PRIVATE nunicode/include/libnu/mph.h
        PRIVATE nunicode/include/libnu/strcoll.h
        PRIVATE nunicode/include/libnu/strcoll_internal.h
        PRIVATE nunicode/include/libnu/strings.h
        PRIVATE nunicode/include/libnu/udb.h
        PRIVATE nunicode/include/libnu/unaccent.h
        PRIVATE nunicode/include/libnu/utf8.h
        PRIVATE nunicode/include/libnu/utf8_internal.h

        PRIVATE nunicode/src/libnu/ducet.c
        PRIVATE nunicode/src/libnu/strcoll.c
        PRIVATE nunicode/src/libnu/strings.c
        PRIVATE nunicode/src/libnu/tolower.c
        PRIVATE nunicode/src/libnu/tounaccent.c
        PRIVATE nunicode/src/libnu/toupper.c
        PRIVATE nunicode/src/libnu/tofold.c
        PRIVATE nunicode/src/libnu/utf8.c
    )

    target_include_directories(mbgl-core
        PRIVATE nunicode/include
    )

    target_compile_definitions(mbgl-core
        PRIVATE "-DNU_WITH_UTF8"
        PRIVATE "-DNU_WITH_Z_COLLATION"
        PRIVATE "-DNU_WITH_CASEMAP"
        PRIVATE "-DNU_WITH_UNACCENT"
    )
endmacro()
