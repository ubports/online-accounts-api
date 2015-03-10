'''online-accounts mock template

This creates the expected methods and properties of the main
com.ubuntu.OnlineAccounts.Manager object. By default, all actions are rejected.  You
can call AllowUnknown() and SetAllowed() on the mock D-BUS interface to control
which actions are allowed.
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

BUS_NAME = 'com.ubuntu.OnlineAccounts.Manager'
MAIN_OBJ = '/com/ubuntu/OnlineAccounts/Manager'
MAIN_IFACE = 'com.ubuntu.OnlineAccounts.Manager'
SYSTEM_BUS = False

ERROR_PREFIX = 'com.ubuntu.OnlineAccounts.Error.'
ERROR_NO_ACCOUNT = ERROR_PREFIX + 'NoAccount'
ERROR_CANCELED = ERROR_PREFIX + 'UserCanceled'
ERROR_PERMISSION_DENIED = ERROR_PREFIX + 'PermissionDenied'
ERROR_INTERACTION_REQUIRED = ERROR_PREFIX + 'InteractionRequired'

CHANGE_TYPE_ENABLED = 0
CHANGE_TYPE_DISABLED = 1
CHANGE_TYPE_UPDATED = 2

def load(mock, parameters):
    mock.AddMethod(MAIN_IFACE,
                   'GetAccounts',
                   'a{sv}',
                   'a(ua{sv})',
                   '''ret = self.accounts.items()''')

    mock.AddMethod(MAIN_IFACE,
                   'RequestAccess',
                   'sa{sv}',
                   '(ua{sv})a{sv}',
                   'ret = self.request_access(args[0], args[1])')

    mock.AddMethod(MAIN_IFACE,
                   'Authenticate',
                   'usbba{sv}',
                   'a{sv}',
                   'ret = self.authenticate(args[0], args[1], args[2], args[3], args[4])')

    mock.accounts = {}
    mock.authentication_reply = {}
    mock.authentication_error = None
    mock.last_account_id = 0
    mock.new_account_details = {}


def request_access(self, service_id, params):
    if len(self.new_account_details) == 0:
        raise dbus.exceptions.DBusException('Permission denied',
                                            name=ERROR_PERMISSION_DENIED)
    self.AddAccount(self.new_account_details)
    authentication_reply = self.authenticate(self.last_account_id, service_id,
                                             True, False, params)
    return ((self.last_account_id, self.new_account_details), authentication_reply)


def authenticate(self, account_id, service_id, interactive, invalidate, params):
    if account_id not in self.accounts:
        raise dbus.exceptions.DBusException('No account with id %s' % (account_id,),
                                            name=ERROR_NO_ACCOUNT)
    if self.authentication_error:
        raise dbus.exceptions.DBusException('Authentication error',
                                            name=self.authentication_error)
    return self.authentication_reply


@dbus.service.method(MOCK_IFACE, in_signature='a{sv}', out_signature='u')
def AddAccount(self, account_details):
    '''Add a new account to the DB

    Returns the ID of the new account
    '''
    self.last_account_id += 1
    self.accounts[self.last_account_id] = account_details
    service = account_details.get('serviceId', '')
    signal_data = account_details.copy()
    signal_data['changeType'] = CHANGE_TYPE_ENABLED
    self.EmitSignal(MAIN_IFACE, 'AccountChanged', 's(ua{sv})',
                    [service, (self.last_account_id, signal_data)])
    return self.last_account_id


@dbus.service.method(MOCK_IFACE, in_signature='u', out_signature='')
def RemoveAccount(self, account_id):
    '''Remove an existing account'''

    account_details = self.accounts[account_id]
    service = account_details.get('serviceId', '')
    signal_data = account_details.copy()
    signal_data['changeType'] = CHANGE_TYPE_DISABLED
    self.EmitSignal(MAIN_IFACE, 'AccountChanged', 's(ua{sv})',
                    [service, (account_id, signal_data)])
    del self.accounts[account_id]


@dbus.service.method(MOCK_IFACE, in_signature='a{sv}', out_signature='')
def SetRequestAccessReply(self, account_details):
    '''Prepares the reply for the next RequestAccess call'''

    self.new_account_details = account_details


@dbus.service.method(MOCK_IFACE, in_signature='sa{sv}', out_signature='')
def SetAuthenticationReply(self, authentication_reply, error_name):
    '''Prepares the reply for the next Authenticate call'''

    self.authentication_reply = authentication_reply
    self.authentication_error = error_name

