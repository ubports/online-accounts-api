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
        self.parser = self.create_parser()
        self.main_loop = GLib.MainLoop()
        self.manager = bus.get_object(BUS_NAME, MAIN_OBJ)
        self.manager.connect_to_signal("AccountChanged", self.on_account_changed,
                dbus_interface=MAIN_IFACE)

    def get_providers(self, args):
        filters = dbus.Dictionary(signature='sv')
        if args.filters:
            filters.update(json.loads(args.filters))
        try:
            print('%s' % json.dumps(self.manager.GetProviders(filters), sort_keys=True),
                    flush=True)
        except dbus.exceptions.DBusException as err:
            print('{ "error": "%s" }' % err.get_dbus_name(), flush=True)


    def get_accounts(self, args):
        filters = dbus.Dictionary(signature='sv')
        if args.filters:
            filters.update(json.loads(args.filters))
        print('%s' % json.dumps(self.manager.GetAccounts(filters), sort_keys=True),
                flush=True)

    def on_account_changed(self, serviceId, accountInfo):
        info = json.dumps(accountInfo, sort_keys=True)
        print('AccountChanged %s %s' % (serviceId, info), flush=True)

    def on_line_read(self, line):
        if not line:
            self.main_loop.quit()
            return
        args = self.parser.parse_args(line.split())
        args.func(args)

    def run(self):
        GLib.io_add_watch(0, GLib.IO_IN | GLib.IO_HUP, self.on_input)
        self.main_loop.run()

    def on_input(self, source, reason):
        if reason & GLib.IO_IN:
            line = sys.stdin.readline()
            self.on_line_read(line.strip())
        else:
            self.on_line_read('')
        return True

    def create_parser(self):
        parser = argparse.ArgumentParser(description='Test process')
        subparsers = parser.add_subparsers()

        parser_providers = subparsers.add_parser('GetProviders')
        parser_providers.add_argument('-f', '--filters')
        parser_providers.set_defaults(func=self.get_providers)

        parser_accounts = subparsers.add_parser('GetAccounts')
        parser_accounts.add_argument('-f', '--filters')
        parser_accounts.set_defaults(func=self.get_accounts)

        return parser


dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

bus = dbus.SessionBus()
print('%s' % bus.get_unique_name(), flush=True)

try:
    app = TestProcess(bus)
except dbus.DBusException:
    traceback.print_exc()
    sys.exit(1)

app.run()
