noinst_LTLIBRARIES = libgmframework.la

framework_dir = $(top_srcdir)/lib/engine/include/framework

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/include \
	-I$(top_srcdir)/lib/engine/include/framework \
	-I$(top_srcdir)/lib/engine/include/addressbook 

libgmframework_la_SOURCES = \
			    $(framework_dir)/services.h \
			    $(framework_dir)/form-builder.h \
			    $(framework_dir)/form-dumper.h \
			    $(framework_dir)/form.h \
			    $(framework_dir)/form-request.h \
			    $(framework_dir)/form-request-simple.h \
			    $(framework_dir)/form-visitor.h \
			    $(framework_dir)/map-key-reference-iterator.h \
			    $(framework_dir)/map-key-const-reference-iterator.h \
			    $(framework_dir)/menu-builder.h \
			    $(framework_dir)/runtime.h \
			    $(framework_dir)/ui.h \
			    menu-builder.cpp \
			    form-builder.cpp \
			    form-dumper.cpp \
			    form-request-simple.cpp \
			    runtime.cpp \
			    services.cpp

libgmframework_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
