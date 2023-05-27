QT      -= core gui
TEMPLATE = lib
TARGET   = bson

DEFINES += BSON_COMPILATION

DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

PUB_HEADERS =                           \
    bson/bcon.h                         \
    bson/common-b64-private.h           \
    bson/bson-config.h                  \
    bson/bson-atomic.h                  \
    bson/bson-clock.h                   \
    bson/bson-compat.h                  \
    bson/bson-context-private.h         \
    bson/bson-context.h                 \
    bson/bson-decimal128.h              \
    bson/bson-endian.h                  \
    bson/bson-error.h                   \
    bson/bson-iso8601-private.h         \
    bson/bson-iter.h                    \
    bson/bson-json.h                    \
    bson/bson-keys.h                    \
    bson/bson-macros.h                  \
    bson/bson-md5.h                     \
    bson/bson-memory.h                  \
    bson/bson-oid.h                     \
    bson/bson-private.h                 \
    bson/bson-reader.h                  \
    bson/bson-string.h                  \
    bson/bson-thread-private.h          \
    bson/bson-timegm-private.h          \
    bson/bson-types.h                   \
    bson/bson-utf8.h                    \
    bson/bson-version.h                 \
    bson/bson-value.h                   \
    bson/bson-version-functions.h       \
    bson/bson-writer.h                  \
    bson/bson.h                         \
    bson/bson-fnv-private.h             \
    bson/common-thread-private.h        \
    bson/common-prelude.h               \
    bson/bson-prelude.h                 \
    jsonsl/jsonsl.h

HEADERS = $$PUB_HEADERS

SOURCES +=                              \
    bson/bcon.c                         \
    bson/common-b64.c                   \
    bson/common-md5.c                   \
    bson/bson-atomic.c                  \
    bson/bson-clock.c                   \
    bson/bson-context.c                 \
    bson/bson-decimal128.c              \
    bson/bson-error.c                   \
    bson/bson-iso8601.c                 \
    bson/bson-iter.c                    \
    bson/bson-json.c                    \
    bson/bson-keys.c                    \
    bson/bson-md5.c                     \
    bson/bson-memory.c                  \
    bson/bson-oid.c                     \
    bson/bson-reader.c                  \
    bson/bson-string.c                  \
    bson/bson-timegm.c                  \
    bson/bson-utf8.c                    \
    bson/bson-value.c                   \
    bson/bson-version-functions.c       \
    bson/bson-writer.c                  \
    bson/bson.c                         \
    bson/bson-fnv.c                     \
    jsonsl/jsonsl.c


include( $$(SRC)/include.pro )

QMAKE_CFLAGS += -Wno-missing-field-initializers
QMAKE_CFLAGS += -Wno-sign-compare
QMAKE_CFLAGS += -Wno-unused-parameter

INCLUDEPATH += ./bson
