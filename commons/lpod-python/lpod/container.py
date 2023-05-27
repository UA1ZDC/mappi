# -*- coding: UTF-8 -*-
#
# Copyright (c) 2009-2010 Ars Aperta, Itaapy, Pierlis, Talend.
#
# Authors: David Versmisse <david.versmisse@itaapy.com>
#          Hervé Cauwelier <herve@itaapy.com>
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

# Import from the Standard Library
import os
import sys
import shutil
from copy import deepcopy
from cStringIO import StringIO
from zipfile import ZIP_DEFLATED, ZIP_STORED, ZipFile, BadZipfile

# Import from lpod
from const import ODF_MIMETYPES, ODF_PARTS, ODF_TYPES, ODF_MANIFEST
from const import ODF_CONTENT, ODF_META, ODF_SETTINGS, ODF_STYLES
from const import ODF_EXTENSIONS
from manifest import odf_manifest
from utils import _get_abspath  #, obsolete
from scriptutils import printwarn


class odf_container(object):
    """Representation of the ODF file.
    """
    # The archive file
    __zipfile = None
    # Using zip archive
    __packaging = None  # None, 'zip', 'flat', 'folder'


    def __init__(self, path_or_file):
        want_folder = False
        if isinstance(path_or_file, basestring):
            # Path
            self.path = path = path_or_file
            if os.path.isdir(path): # opening a folder
                want_folder = True
            else:
                file = open(path, 'rb')
        else:
            # File-like assumed
            self.path = None
            file = path_or_file
        if want_folder:
            self.__data = data = path
            self.__packaging = 'folder'
            try:
                mimetype, timestamp = self.__get_folder_part('mimetype')
            except:
                printwarn ("corrupted or not an OpenDocument folder (missing mimetype)")
                mimetype = ''
                timestamp = None
            if mimetype not in ODF_MIMETYPES:
                message = 'Document of unknown type "%s", trying with ODF_TEXT.' % mimetype
                printwarn(message)
                mimetype = ODF_EXTENSIONS['odt']
            self.__parts = {'mimetype': mimetype}
            self.__parts_ts = {'mimetype': timestamp}
        else:
            self.__data = data = file.read()
            zip_expected = data[:4] == 'PK\x03\x04'
            # Most probably zipped document
            try:
                mimetype = self.__get_zip_part('mimetype')
                self.__packaging = 'zip'
            except BadZipfile:
                if zip_expected:
                    raise ValueError("corrupted or not an OpenDocument archive")
                # Maybe XML document
                try:
                    mimetype = self.__get_xml_part('mimetype')
                except ValueError:
                    raise ValueError("bad OpenDocument format")
                self.__packaging = 'flat'
            if mimetype not in ODF_MIMETYPES:
                message = 'Document of unknown type "%s"' % mimetype
                raise ValueError(message)
            self.__parts = {'mimetype': mimetype}


    #
    # Private API (internal helpers)
    #

    def __get_data(self):
        """Return bytes of the ODF in memory.
        """
        return self.__data


    # XML implementation

    def __get_xml_parts(self):
        """Get the list of members in the XML-only ODF.
        """
        raise NotImplementedError


    def __get_xml_part(self, name):
        """Get bytes of a part from the XML-only ODF. No cache.
        """
        if name not in ODF_PARTS and name != 'mimetype':
            raise ValueError("Third-party parts are not supported "
                               "in an XML-only ODF document")
        data = self.__get_data()
        if name == 'mimetype':
            start_attr = 'office:mimetype="'
            start = data.index(start_attr) + len(start_attr)
            end = data.index('"', start)
            part = data[start:end]
        else:
            start_tag = '<office:document-%s>' % name
            start = data.index(start_tag)
            end_tag = '</office:document-%s>' % name
            end = data.index(end_tag) + len(end_tag)
            part = data[start:end]
        return part


    def __save_xml(self, file):
        """Save an XML-only ODF from the available parts.
        """
        raise NotImplementedError


    # Zip implementation

    def __get_zipfile(self):
        """Open a Zip object on the Zip ODF.
        """
        if self.__zipfile is None:
            data = self.__get_data()
            # StringIO will not duplicate the string, how big it is
            filelike = StringIO(data)
            self.__zipfile = ZipFile(filelike)
        return self.__zipfile


    def __get_zip_parts(self):
        """Get the list of members in the Zip ODF.
        """
        zipfile = self.__get_zipfile()
        return [part.filename for part in zipfile.infolist()]


    def __get_zip_part(self, path):
        """Get bytes of a part from the Zip ODF. No cache.
        """
        zipfile = self.__get_zipfile()

        return zipfile.read(path)


    def __save_zip(self, file):
        """Save a Zip ODF from the available parts.
        """
        # Parts were loaded by "save"
        parts = self.__parts
        compression = ZIP_DEFLATED
        try:
            filezip = ZipFile(file, 'w', compression=compression)
        except RuntimeError:
            # No zlib module
            compression = ZIP_STORED
            filezip = ZipFile(file, 'w', compression=compression)
        # Parts to save, except manifest at the end
        part_names = parts.keys()
        try:
            part_names.remove(ODF_MANIFEST)
        except KeyError:
            printwarn("missing '%s'" % ODF_MANIFEST)
        # "Pretty-save" parts in some order
        # mimetype requires to be first and uncompressed
        filezip.compression = ZIP_STORED
        try:
            filezip.writestr('mimetype', parts['mimetype'])
            filezip.compression = compression
            part_names.remove('mimetype')
        except:
            printwarn("missing 'mimetype'")
        # XML parts
        for path in ODF_CONTENT, ODF_META, ODF_SETTINGS, ODF_STYLES:
            if path not in parts:
                printwarn("missing '%s'" % path)
                continue
            filezip.writestr(path, parts[path])
            part_names.remove(path)
        # Everything else
        for path in part_names:
            data = parts[path]
            if data is None:
                # Deleted
                continue
            filezip.writestr(path, data)
        # Manifest
        filezip.writestr(ODF_MANIFEST, parts[ODF_MANIFEST])
        filezip.close()


    def __get_folder_parts(self):
        """Get the list of members in the ODF folder.
        """
        def parse_folder(folder):
            parts = []
            file_names = os.listdir(os.path.join(self.__data, folder))
            for f in file_names:
                if f.startswith('.'):   # no hidden files
                    continue
                if os.path.isfile(os.path.join(self.__data, folder, f)):
                    part_name = os.path.join(folder, f)
                    parts.append(part_name)
                if os.path.isdir(os.path.join(self.__data, folder, f)):
                    sub_folder = os.path.join(folder, f)
                    sub_parts = parse_folder(sub_folder)
                    if len(sub_parts) > 0:
                        parts.extend(sub_parts)
                    else:
                        # store leaf directories
                        parts.append(os.path.join(sub_folder)+'/')
            return parts
        return parse_folder('')


    def __get_folder_part(self, path):
        """Get bytes of a part from the ODF folder, with timestamp.
        """
        name = os.path.join(self.__data, path)
        if os.path.isdir(name):
            return ('', os.stat(name)[8])
        timestamp = os.stat(name)[8]
        return (open(name).read() , timestamp)


    def __get_folder_part_timestamp(self, path):
        name = os.path.join(self.__data, path)
        try:
            timestamp = os.stat(name)[8]
        except:
            timestamp = -1
        return timestamp


    def __save_folder(self, folder):
        """Save a folder ODF from the available parts.
        """
        encoding = sys.getfilesystemencoding()

        def dump(path, content):
            try:
                path = path.decode(encoding)
            except:
                pass
            file_name = os.path.join(folder, path)
            dir_name = os.path.dirname(file_name)
            if not os.path.exists(dir_name):
                os.makedirs(dir_name, mode=0755)
            if path.endswith(u'/') : # folder
                if not os.path.isdir(file_name):
                    os.makedirs(file_name.encode(encoding), mode=0777)
            else:
                open(file_name.encode(encoding), 'wb', 0666).write(content)

        if isinstance(folder, basestring) and not isinstance(folder, unicode):
            folder = folder.decode(encoding)
        # Parts were loaded by "save"
        parts = self.__parts
        # Parts to save, except manifest at the end
        part_names = parts.keys()
        try:
            part_names.remove(ODF_MANIFEST)
        except KeyError:
            printwarn(u"missing '%s'" % ODF_MANIFEST)
        # "Pretty-save" parts in some order
        # mimetype requires to be first and uncompressed
        try:
            dump('mimetype', parts['mimetype'])
            part_names.remove('mimetype')
        except:
            printwarn("missing 'mimetype'")
        # XML parts
        for path in ODF_CONTENT, ODF_META, ODF_SETTINGS, ODF_STYLES:
            if path not in parts:
                printwarn("missing '%s'" % path)
                continue
            dump(path, parts[path])
            part_names.remove(path)
        # Everything else
        for path in part_names:
            data = parts[path]
            if data is None:
                # Deleted
                continue
            dump(path, data)
        # Manifest
        dump(ODF_MANIFEST, parts[ODF_MANIFEST])


    #
    # Public API
    #

    def get_parts(self):
        """Get the list of members.
        """
        if self.__packaging == 'zip':
            return self.__get_zip_parts()
        elif self.__packaging == 'folder':
            return self.__get_folder_parts()
        else:
            return self.__get_xml_parts()


    def get_part(self, path):
        """Get the bytes of a part of the ODF.
        """
        loaded_parts = self.__parts
        if path in loaded_parts:
            part = loaded_parts[path]
            if part is None:
                raise ValueError('part "%s" is deleted' % path)
            if self.__packaging == 'folder':
                cache_ts = self.__parts_ts.get(path, -1)
                current_ts = self.__get_folder_part_timestamp(path)
                if current_ts != cache_ts:
                    part, timestamp = self.__get_folder_part(path)
                    loaded_parts[path] = part
                    self.__parts_ts[path] = timestamp
            return part
        if self.__packaging == 'zip':
            part = self.__get_zip_part(path)
        elif self.__packaging == 'folder':
            part, timestamp = self.__get_folder_part(path)
            self.__parts_ts[path] = timestamp
        else:
            part = self.__get_xml_part(path)
        loaded_parts[path] = part
        return part


    def set_part(self, path, data):
        """Replace or add a new part.
        """
        self.__parts[path] = data


    def del_part(self, path):
        """Mark a part for deletion.
        """
        self.__parts[path] = None


    def clone(self):
        """Make a copy of this container with no path.
        """
        # FIXME must load parts before?
        clone = object.__new__(self.__class__)
        for name in self.__dict__:
            # "__zipfile" is not safe to copy
            # but can be recreated from "__data"
            if name in ('path', '_odf_container__zipfile'):
                setattr(clone, name, None)
            else:
                value = getattr(self, name)
                value = deepcopy(value)
                setattr(clone, name, value)
        return clone


    def _do_backup(self, target):
        parts = target.split('.', 1)
        if len(parts) == 1:
            back_file = target + '.backup'
        else:
            back_file = parts[0] + '.backup.' + parts[1]
        if os.path.exists(target):
            if os.path.isdir(back_file):
                try:
                    shutil.rmtree(back_file)
                except Exception as e:
                    printwarn(str(e))
            try:
                shutil.move(target, back_file)
            except Exception as e:
                printwarn(str(e))


    def save(self, target=None, packaging=None, backup=False):
        """Save the container to the given target, a path or a file-like
        object.

        Package the output document in the same format than this document,
        unless "packaging" is different.

        Arguments:

            target -- str or file-like

            packaging -- 'zip' or 'flat', or for debugging purpose 'folder'

            backup -- boolean
        """
        if isinstance(target, basestring) and not isinstance(target, unicode):
            encoding = sys.getfilesystemencoding()
            target = target.decode(encoding)
        parts = self.__parts
        # Packaging
        if packaging is None:
            if self.__packaging:
                packaging = self.__packaging
            else:
                packaging = 'zip' # default
        packaging = packaging.strip().lower()
        if packaging not in ('zip', 'flat', 'folder'):
            raise ValueError('packaging type "%s" not supported' % packaging)
        # Load parts else they will be considered deleted
        for path in self.get_parts():
            if path not in parts:
                self.get_part(path)
        # Open output file
        close_after = False
        if target is None:
            target = self.path
        if isinstance(target, basestring):
            while target.endswith(os.sep):
                target = target[:-1]
            while target.endswith('.folder'):
                target = target.split('.folder', 1)[0]
        if packaging in ('zip', 'flat'):
            if isinstance(target, basestring):
                if backup:
                    self._do_backup(target)
                dest_file = open(target, 'wb')
                close_after = True
            else:
                dest_file = target
        if packaging == 'folder':
            if not isinstance(target, basestring):
                raise ValueError("Saving in folder format requires a folder "
                                 "name, not %s." % target)
            if not target.endswith('.folder'):
                target = target + '.folder'
            if backup:
                self._do_backup(target)
            else:
                if os.path.exists(target):
                    try:
                        shutil.rmtree(target)
                    except Exception as e:
                        printwarn(str(e))

            os.mkdir(target, 0777)
            dest_file = target
        # Serialize
        if packaging == 'zip':
            self.__save_zip(dest_file)
        elif packaging == 'flat':
            self.__save_xml(dest_file)
        else: # folder
            self.__save_folder(dest_file)
        # Close files we opened ourselves
        if close_after:
            dest_file.close()



def odf_get_container(path_or_file):
    """Return an odf_container instance of the ODF document stored at the
    given local path or in the given (open) file-like object.
    """
    return odf_container(path_or_file)



def odf_new_container(path_or_file):
    """Return an odf_container instance based on the given template.
    """
    if path_or_file in ODF_TYPES:
        path_or_file = _get_abspath(ODF_TYPES[path_or_file])
    template_container = odf_get_container(path_or_file)
    # Return a copy of the template container
    clone = template_container.clone()
    # Change type from template to regular
    mimetype = clone.get_part('mimetype').replace('-template', '')
    clone.set_part('mimetype', mimetype)
    # Update the manifest
    manifest = odf_manifest(ODF_MANIFEST, clone)
    manifest.set_media_type('/', mimetype)
    clone.set_part(ODF_MANIFEST, manifest.serialize())
    return clone

#odf_new_document_from_template = obsolete('odf_new_document_from_template',
#        odf_new_container)
#odf_new_document_from_type = obsolete('odf_new_document_from_template',
#        odf_new_container)
