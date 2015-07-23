'''D-Bus mock template for GetConnectionAppArmorSecurityContext
'''

# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 3 of the License, or (at your option) any
# later version.  See http://www.gnu.org/copyleft/lgpl.html for the full text
# of the license.

__author__ = 'Alberto Mardegan'
__email__ = 'alberto.mardegan@canonical.com'
__copyright__ = '(c) 2015 Canonical Ltd.'
__license__ = 'LGPL 3+'

import dbus

from dbusmock import MOCK_IFACE

BUS_NAME = 'mocked.org.freedesktop.dbus'
MAIN_OBJ = '/org/freedesktop/DBus'
MAIN_IFACE = 'org.freedesktop.DBus'
SYSTEM_BUS = False

def load(mock, parameters):
    mock.AddMethod(MAIN_IFACE,
                   'GetConnectionAppArmorSecurityContext',
                   's', 's',
                   'ret = self.contexts.get(args[0], "unconfined")')

    mock.contexts = {}

@dbus.service.method(MOCK_IFACE, in_signature='ss', out_signature='s')
def AddClient(self, client, context):
    '''Adds a client with its security context'''
    print("Adding client %s with context %s" % (client, context))
    self.contexts[client] = context
    return '%s - %s' % (client, context)

