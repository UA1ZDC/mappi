TEMPLATE = subdirs
SUBDIRS = \
    includes \
    app \
    singleton \
    debug \
    funcs

app.depends = includes
debug.depends = app