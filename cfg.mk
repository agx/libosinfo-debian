# Customize Makefile.maint.                           -*- makefile -*-
# Copyright (C) 2008-2012 Red Hat, Inc.
# Copyright (C) 2003-2008 Free Software Foundation, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Tests not to run as part of "make distcheck".
local-checks-to-skip =			\
  changelog-check			\
  makefile-check			\
  makefile_path_separator_check		\
  patch-check				\
  sc_GPL_version			\
  sc_always_defined_macros		\
  sc_cast_of_alloca_return_value	\
  sc_cross_check_PATH_usage_in_tests	\
  sc_dd_max_sym_length			\
  sc_error_exit_success			\
  sc_file_system			\
  sc_immutable_NEWS			\
  sc_prohibit_always_true_header_tests  \
  sc_two_space_separator_in_usage	\
  sc_error_message_uppercase		\
  sc_program_name			\
  sc_makefile_check

# Files that should never cause syntax check failures.
VC_LIST_ALWAYS_EXCLUDE_REGEX = \
  (^HACKING|\.po|maint.mk)$$

# Tweak how some of the syntax check rules work
_makefile_at_at_check_exceptions = ' && !/INTLTOOL_XML_RULE/'
_gl_translatable_string_re = \
	(\b(N?_|gettext *)\([^)"]*("|$$))|(<(_[a-zA-Z]*).*/\5>)

# Functions like free() that are no-ops on NULL arguments.
useless_free_options =				\
  --name=g_free					\
  --name=xmlBufferFree				\
  --name=xmlFree				\
  --name=xmlFreeDoc				\
  --name=xmlXPathFreeContext			\
  --name=xmlXPathFreeObject

# Ensure that no C source file, docs, or rng schema uses TABs for
# indentation.  Also match *.h.in files, to get libvirt.h.in.  Exclude
# files in gnulib, since they're imported.
space_indent_files=(\.(rng|s?[ch](\.in)?|html.in|py)|(daemon|tools)/.*\.in)
sc_TAB_in_indentation:
	@prohibit='^ *	'						\
	in_vc_files='$(space_indent_files)$$'				\
	halt='indent with space, not TAB, in C, sh, html, py, and RNG schemas' \
	  $(_sc_search_regexp)

# G_GNUC_UNUSED should only be applied in implementations, not
# header declarations
sc_avoid_attribute_unused_in_header:
	@prohibit='^[^#]*G_GNUC_UNUSED([^:]|$$)'			\
	in_vc_files='\.h$$'						\
	halt='use G_GNUC_UNUSED in .c rather than .h files'		\
	  $(_sc_search_regexp)

# Enforce recommended preprocessor indentation style.
sc_preprocessor_indentation:
	@if cppi --version >/dev/null 2>&1; then			\
	  $(VC_LIST_EXCEPT) | grep '\.[ch]$$' | xargs cppi -a -c	\
	    || { echo '$(ME): incorrect preprocessor indentation' 1>&2;	\
		exit 1; };						\
	else								\
	  echo '$(ME): skipping test $@: cppi not installed' 1>&2;	\
	fi

sc_copyright_format:
	@require='Copyright .*Red 'Hat', Inc\.'				\
	containing='Copyright .*Red 'Hat				\
	halt='Red Hat copyright is missing Inc.'			\
	  $(_sc_search_regexp)
	@prohibit='Copyright [^(].*Red 'Hat				\
	halt='consistently use (C) in Red Hat copyright'		\
	  $(_sc_search_regexp)
	@prohibit='\<Red''Hat\>'					\
	halt='spell Red Hat as two words'				\
	  $(_sc_search_regexp)

sc_bracket_spacing_check:
	$(AM_V_GEN)files=`$(VC_LIST) | grep '\.c$$'`; \
	$(PERL) $(top_srcdir)/build-aux/bracket-spacing.pl $$files || \
	  { echo '$(ME): incorrect whitespace' 1>&2; \
	    exit 1; }

# We don't use this feature of maint.mk.
prev_version_file = /dev/null

exclude_file_name_regexp--sc_bindtextdomain = ^(libvirt-gconfig/tests|examples)/

exclude_file_name_regexp--sc_preprocessor_indentation = ^*/*.[ch]

exclude_file_name_regexp--sc_require_config_h = python/libvirt-glib.c
exclude_file_name_regexp--sc_require_config_h_first = python/libvirt-glib.c

exclude_file_name_regexp--sc_trailing_blank = ^tests/isodata/.*|docs/object-model.fig

exclude_file_name_regexp--sc_bindtextdomain = ^tools/.*.c|tests/.*.c

exclude_file_name_regexp--sc_space_tab = ^tests/isodata/.*

exclude_file_name_regexp--sc_prohibit_atoi_atof = ^tests/.*\.c
