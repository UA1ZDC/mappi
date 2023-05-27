# -*- coding: UTF-8 -*-
#
# Copyright (c) 2009-2010 Ars Aperta, Itaapy, Pierlis, Talend.
#
# Authors: Hervé Cauwelier <herve@itaapy.com>
#          Romain Gauthier <romain@itaapy.com>
#
# This file is part of Lpod (see: http://lpod-project.net).
# Lpod is free software; you can redistribute it and/or modify it under
# the terms of either:
#
# a) the GNU General Public License as published by the Free Software
#    Foundation, either version 3 of the License, or (at your option)
#    any later version.
#    Lpod is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#    You should have received a copy of the GNU General Public License
#    along with Lpod.  If not, see <http://www.gnu.org/licenses/>.
#
# b) the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#    http://www.apache.org/licenses/LICENSE-2.0
#

# Import from lpod
from xmlpart import odf_xmlpart
from utils import _get_elements, _get_element  #, obsolete



context_mapping = {
        'paragraph': ('//office:styles', '//office:automatic-styles'),
        'text': ('//office:styles',),
        'graphic': ('//office:styles',),
        'page-layout': ('//office:automatic-styles',),
        'master-page': ('//office:master-styles',),
        'font-face': ('//office:font-face-decls',),
        'outline': ('//office:styles',),
        'date': ('//office:automatic-styles',),
        'list': ('//office:styles',),
        'presentation': ('//office:styles', '//office:automatic-styles'),
        'drawing-page': ('//office:automatic-styles',),
        'presentation-page-layout': ('//office:styles',),
        'marker': ('//office:styles',),
        'fill-image': ('//office:styles',),
        # FIXME Do they?
        'table': ('//office:automatic-styles',),
        'table-cell': ('//office:automatic-styles',),
        'table-row': ('//office:automatic-styles',),
        'table-column': ('//office:automatic-styles',),
}



class odf_styles(odf_xmlpart):

    def _get_style_contexts(self, family, automatic=False):
        if automatic is True:
            return (self.get_element('//office:automatic-styles'),)
        elif family is None:
            # All possibilities
            return (self.get_element('//office:automatic-styles'),
                    self.get_element('//office:styles'),
                    self.get_element('//office:master-styles'),
                    self.get_element('//office:font-face-decls'))
        queries = context_mapping.get(family)
        if queries is None:
            raise ValueError, "unknown family: " + family
        return [self.get_element(query) for query in queries]


    def get_styles(self, family=None, automatic=False):
        """Return the list of styles in the Content part, optionally limited
        to the given family.

        Arguments:

            family -- str

        Return: list of odf_style
        """
        result = []
        for context in self._get_style_contexts(family, automatic=automatic):
            if context is None:
                continue
            result.extend(context.get_styles(family=family))
        return result

    #get_style_list = obsolete('get_style_list', get_styles)


    def get_style(self, family, name_or_element=None, display_name=None):
        """Return the style uniquely identified by the name/family pair. If
        the argument is already a style object, it will return it.

        If the name is None, the default style is fetched.

        If the name is not the internal name but the name you gave in the
        desktop application, use display_name instead.

        Arguments:

            name_or_element -- unicode, odf_style or None

            family -- 'paragraph', 'text',  'graphic', 'table', 'list',
                      'number', 'page-layout', 'master-page'

            display_name -- unicode

        Return: odf_style or None if not found
        """
        for context in self._get_style_contexts(family):
            if context is None:
                continue
            style = context.get_style(family,
                    name_or_element=name_or_element,
                    display_name=display_name)
            if style is not None:
                return style
        return None


    def get_master_pages(self):
        return _get_elements(self, 'descendant::style:master-page')


    def get_master_page(self, position=0):
        return _get_element(self, 'descendant::style:master-page', position)
