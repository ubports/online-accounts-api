#! /usr/bin/python3
# coding: UTF-8

import argparse

import dbus

import dbus.mainloop.glib
from gi.repository import GLib

import json
import sys
import traceback


BUS_NAME = 'com.ubuntu.OnlineAccounts.Manager'
MAIN_OBJ = '/com/ubuntu/OnlineAccounts/Manager'
MAIN_IFACE = 'com.ubuntu.OnlineAccounts.Manager'


class TestProcess:
    def __init__(self, bus):
        self.main_loop = GLib.MainLoop()
        self.manager = bus.get_object(BUS_NAME, MAIN_OBJ)

    def get_accounts(self, args):
        filters = dbus.Dictionary(signature='sv')
        if args.filters:
            filters.update(json.loads(args.filters))
        print('%s' % json.dumps(self.manager.GetAccounts(filters), sort_keys=True))


def create_parser(app):
    parser = argparse.ArgumentParser(description='Test process')
    subparsers = parser.add_subparsers()

    parser_accounts = subparsers.add_parser('GetAccounts')
    parser_accounts.add_argument('-f', '--filters')
    parser_accounts.set_defaults(func=app.get_accounts)

    return parser


dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

bus = dbus.SessionBus()
print('%s' % bus.get_unique_name())

try:
    app = TestProcess(bus)
except dbus.DBusException:
    traceback.print_exc()
    sys.exit(1)

parser = create_parser(app)

while True:
    command = input()
    if not command:
        sys.exit(0)
    args = parser.parse_args(command.split())
    args.func(args)
