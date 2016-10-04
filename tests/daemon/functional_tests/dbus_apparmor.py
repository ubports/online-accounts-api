'''dbus mock template

This creates the expected methods and properties of the
org.freedesktop.DBus service.
'''

# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 3 of the License, or (at your option) any
# later version.  See http://www.gnu.org/copyleft/lgpl.html for the full text
# of the license.

__author__ = 'Alberto Mardegan'
__email__ = 'alberto.mardegan@canonical.com'
__copyright__ = '(c) 2016 Canonical Ltd.'
__license__ = 'LGPL 3+'

import dbus
import time

from dbusmock import MOCK_IFACE
import dbusmock

BUS_NAME = 'mocked.org.freedesktop.dbus'
MAIN_OBJ = '/org/freedesktop/DBus'
MAIN_IFACE = 'org.freedesktop.DBus'
SYSTEM_BUS = False

ERROR_PREFIX = 'org.freedesktop.DBus.Error.'
ERROR_NAME_HAS_NO_OWNER = ERROR_PREFIX + 'NameHasNoOwner'

def get_credentials(self, service):
    if service not in self.credentials:
        raise dbus.exceptions.DBusException('Service not found',
                                            name=ERROR_NAME_HAS_NO_OWNER)
    return self.credentials[service]


def load(mock, parameters):
    mock.get_credentials = get_credentials
    mock.AddMethods(MAIN_IFACE, [
        ('GetConnectionCredentials', 's', 'a{sv}', 'ret = self.get_credentials(self, args[0])'),
    ])

    mock.credentials = {}


@dbus.service.method(MOCK_IFACE, in_signature='sa{sv}', out_signature='')
def SetCredentials(self, service, credentials):
    self.credentials[service] = credentials
