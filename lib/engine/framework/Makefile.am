noinst_LTLIBRARIES = libgmframework.la

framework_dir = $(top_srcdir)/lib/engine/framework

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(XML_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf			\
	-I$(top_srcdir)/lib/engine/include 		\
	-I$(top_srcdir)/lib/engine/framework

libgmframework_la_SOURCES = \
			    $(framework_dir)/services.h \
			    $(framework_dir)/chain-of-responsibility.h \
			    $(framework_dir)/form-builder.h \
			    $(framework_dir)/form-dumper.h \
			    $(framework_dir)/form.h \
			    $(framework_dir)/form-request.h \
			    $(framework_dir)/form-request-simple.h \
			    $(framework_dir)/form-visitor.h \
			    $(framework_dir)/gmconf-bridge.h \
			    $(framework_dir)/gmconf-bridge.cpp \
			    $(framework_dir)/map-key-reference-iterator.h \
			    $(framework_dir)/map-key-const-reference-iterator.h \
			    $(framework_dir)/menu-builder.h \
			    $(framework_dir)/runtime.h \
			    $(framework_dir)/menu-builder.cpp \
			    $(framework_dir)/form-builder.cpp \
			    $(framework_dir)/form-dumper.cpp \
			    $(framework_dir)/form-request-simple.cpp \
			    $(framework_dir)/runtime-glib.cpp \
			    $(framework_dir)/services.cpp \
			    $(framework_dir)/trigger.h \
			    $(framework_dir)/menu-xml.h \
			    $(framework_dir)/menu-xml.cpp 

libgmframework_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS) $(XML_LIBS)
