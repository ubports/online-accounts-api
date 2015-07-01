'''online-accounts-service mock template

This creates the expected methods and properties of the
com.ubuntu.OnlineAccountsUi service. By default, all actions are rejected.
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

BUS_NAME = 'com.ubuntu.OnlineAccountsUi'
MAIN_OBJ = '/'
MAIN_IFACE = 'com.ubuntu.OnlineAccountsUi'
SYSTEM_BUS = False

ERROR_PREFIX = 'com.ubuntu.OnlineAccountsUi.'
ERROR_USER_CANCELED = ERROR_PREFIX + 'UserCanceled'
ERROR_INVALID_PARAMETERS = ERROR_PREFIX + 'InvalidParameters'
ERROR_INVALID_APPLICATION= ERROR_PREFIX + 'InvalidApplication'

def load(mock, parameters):
    mock.AddMethod(MAIN_IFACE,
                   'requestAccess',
                   'a{sv}',
                   'a{sv}',
                   'ret = self.request_access(args[0])')

    mock.access_reply = {}
    mock.access_reply_error = {}

    def request_access(self, params):
        if 'errorName' in self.access_reply:
            raise dbus.exceptions.DBusException(self.access_reply.errorMessage,
                                                name=self.access_reply.errorName)
        return self.access_reply

    setattr(mock.__class__, "request_access", request_access)

@dbus.service.method(MOCK_IFACE, in_signature='a{sv}', out_signature='')
def SetRequestAccessReply(self, reply):
    '''Prepares the reply for the next RequestAccess call'''
    self.access_reply = reply

