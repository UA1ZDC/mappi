#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Import from the Standard Library
from datetime import date
from os import remove
from os.path import exists
from sys import argv, stderr, exit

# Import from lpod
from lpod.document import odf_new_document
from lpod.span import odf_create_span
from lpod.template import stl_odf


def get_namespace(homme=False):
    return {"titre": "Test de STL no 1",
            "date": date.today().strftime("%d/%m/%Y"),
            "homme": homme,
            "genre": "M." if homme else "Mme",
            "nom": "Michu",
            "enum1": {'label': "Revenu", 'value': 1234.56},
            "enum2": {'label': "Âge", 'value': 65},
            "couleur": "rouge",
            "gras": "gras comme un moine",
            "élément": odf_create_span("élément", style='T2')}


if __name__ == '__main__':
    try:
        output = argv[1]
    except IndexError:
        print("Usage: %s <output document>" % argv[0], file=stderr)
        exit(1)
    document = odf_new_document('test_template.ott')
    stl_odf(document, get_namespace())
    if exists(output):
        remove(output)
    document.save(output)
    print('Document "%s" generated.' % output)
