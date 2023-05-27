# -*- coding: UTF-8 -*-
#
# Copyright (c) 2009-2010 Ars Aperta, Itaapy, Pierlis, Talend.
#
# Authors: Hervé Cauwelier <herve@itaapy.com>
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
from element import register_element_class, odf_element, odf_create_element
from shapes import registered_shapes
from smil import odf_create_anim_par, odf_create_anim_transitionFilter


def odf_create_draw_page(page_id=None, name=None, master_page=None,
                         presentation_page_layout=None, style=None):
    """This element is a container for content in a drawing or presentation
    document.

    Arguments:

        page_id -- str

        name -- unicode

        master_page -- str

        presentation_page_layout -- str

        style -- str

    Return: odf_element
    """
    element = odf_create_element('draw:page')
    if page_id:
        element.set_id(page_id)
    if name:
        element.set_name(name)
    if style:
        element.set_style(style)
    if master_page:
        element.set_master_page(master_page)
    if presentation_page_layout:
        element.set_presentation_page_layout(presentation_page_layout)
    return element



class odf_draw_page(odf_element):
    """Specialised element for pages of presentation and drawing.
    """
    def get_name(self):
        return self.get_attribute('draw:name')


    def set_name(self, name):
        return self.set_attribute('draw:name', name)


    def get_style(self):
        return self.get_attribute('draw:style-name')


    def set_style(self, name):
        return self.set_style_attribute('draw:style-name', name)


    def get_master_page(self):
        return self.get_attribute('draw:master-page-name')


    def set_master_page(self, name):
        return self.set_style_attribute('draw:master-page-name', name)


    def get_presentation_page_layout(self):
        return self.get_attribute('presentation:presentation-page-layout-name')


    def set_presentation_page_layout(self, name):
        attribute = 'presentation:presentation-page-layout-name'
        return self.set_style_attribute(attribute, name)


    def get_id(self):
        return self.get_attribute('draw:id')


    def set_id(self, page_id):
        self.set_attribute('draw:id', page_id)


    def set_transition(self, type, subtype=None, dur='2s'):
        # Create the new animation
        anim_page = odf_create_anim_par(presentation_node_type="timing-root")
        my_page_id = self.get_id()
        anim_begin = odf_create_anim_par(smil_begin="%s.begin" % my_page_id)
        transition = odf_create_anim_transitionFilter(smil_dur=dur,
                                                      smil_type=type,
                                                      smil_subtype=subtype)
        anim_page.append(anim_begin)
        anim_begin.append(transition)

        # Replace when already a transition:
        #   anim:seq => After the frame's transition
        #   cf page 349 of OpenDocument-v1.0-os.pdf
        #   Conclusion: We must delete the first child 'anim:par'
        existing = self.get_element('anim:par')
        if existing:
            self.delete(existing)
        self.append(anim_page)


    def get_shapes(self):
        query = ('(descendant::'
                + '|descendant::'.join(registered_shapes) + ')')
        return self.get_elements(query)


    def get_formatted_text(self, context):
        result = []
        for element in self.get_children():
            if element.get_tag() == 'presentation:notes':
                # No need for an advanced odf_notes.get_formatted_text()
                # because the text seems to be only contained in paragraphs
                # and frames, that we already handle
                for child in element.get_children():
                    result.append(child.get_formatted_text(context))
                result.append(u"\n")
            result.append(element.get_formatted_text(context))
        result.append(u"\n")
        return u"".join(result)



register_element_class('draw:page', odf_draw_page)
