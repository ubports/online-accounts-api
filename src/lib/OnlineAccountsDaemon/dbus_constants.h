/*
 * This file is part of OnlineAccountsDaemon
 *
 * Copyright (C) 2015-2016 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ONLINE_ACCOUNTS_DBUS_CONSTANTS_P_H
#define ONLINE_ACCOUNTS_DBUS_CONSTANTS_P_H

/* Manager service, object path and interface name */
#define ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME \
    "com.ubuntu.OnlineAccounts.Manager"
#define ONLINE_ACCOUNTS_MANAGER_PATH "/com/ubuntu/OnlineAccounts/Manager"
#define ONLINE_ACCOUNTS_MANAGER_INTERFACE ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME

/* Keys for the account info dictionary */
#define ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME "displayName"
#define ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID "serviceId"
#define ONLINE_ACCOUNTS_INFO_KEY_AUTH_METHOD "authMethod"
#  define ONLINE_ACCOUNTS_AUTH_METHOD_UNKNOWN 0
#  define ONLINE_ACCOUNTS_AUTH_METHOD_OAUTH1 1
#  define ONLINE_ACCOUNTS_AUTH_METHOD_OAUTH2 2
#  define ONLINE_ACCOUNTS_AUTH_METHOD_PASSWORD 3
#  define ONLINE_ACCOUNTS_AUTH_METHOD_SASL 4
#define ONLINE_ACCOUNTS_INFO_KEY_SETTINGS "settings/"
#define ONLINE_ACCOUNTS_INFO_KEY_CHANGE_TYPE "changeType"
#  define ONLINE_ACCOUNTS_INFO_CHANGE_ENABLED 0
#  define ONLINE_ACCOUNTS_INFO_CHANGE_DISABLED 1
#  define ONLINE_ACCOUNTS_INFO_CHANGE_UPDATED 2

/* Keys for the provider info dictionary */
#define ONLINE_ACCOUNTS_INFO_KEY_PROVIDER_ID "providerId"
#define ONLINE_ACCOUNTS_INFO_KEY_TRANSLATIONS "translations"

/* Error codes */
#define ONLINE_ACCOUNTS_ERROR_PREFIX "com.ubuntu.OnlineAccounts.Error."
#define ONLINE_ACCOUNTS_ERROR_NO_ACCOUNT \
    ONLINE_ACCOUNTS_ERROR_PREFIX "NoAccount"
#define ONLINE_ACCOUNTS_ERROR_USER_CANCELED \
    ONLINE_ACCOUNTS_ERROR_PREFIX "UserCanceled"
#define ONLINE_ACCOUNTS_ERROR_PERMISSION_DENIED \
    ONLINE_ACCOUNTS_ERROR_PREFIX "PermissionDenied"
#define ONLINE_ACCOUNTS_ERROR_INTERACTION_REQUIRED \
    ONLINE_ACCOUNTS_ERROR_PREFIX "InteractionRequired"

/* Keys for the authentication data dictionaries */
#define ONLINE_ACCOUNTS_AUTH_KEY_CLIENT_ID "ClientId"
#define ONLINE_ACCOUNTS_AUTH_KEY_CLIENT_SECRET "ClientSecret"
#define ONLINE_ACCOUNTS_AUTH_KEY_SCOPES "Scopes"
#define ONLINE_ACCOUNTS_AUTH_KEY_ACCESS_TOKEN "AccessToken"
#define ONLINE_ACCOUNTS_AUTH_KEY_EXPIRES_IN "ExpiresIn"
#define ONLINE_ACCOUNTS_AUTH_KEY_GRANTED_SCOPES "GrantedScopes"

#define ONLINE_ACCOUNTS_AUTH_KEY_CONSUMER_KEY "ConsumerKey"
#define ONLINE_ACCOUNTS_AUTH_KEY_CONSUMER_SECRET "ConsumerSecret"
#define ONLINE_ACCOUNTS_AUTH_KEY_TOKEN "Token"
#define ONLINE_ACCOUNTS_AUTH_KEY_TOKEN_SECRET "TokenSecret"
#define ONLINE_ACCOUNTS_AUTH_KEY_SIGNATURE_METHOD "SignatureMethod"

#define ONLINE_ACCOUNTS_AUTH_KEY_SERVICE "Service"
#define ONLINE_ACCOUNTS_AUTH_KEY_MECHANISMS "MechList"
#define ONLINE_ACCOUNTS_AUTH_KEY_FQDN "Fqdn"
#define ONLINE_ACCOUNTS_AUTH_KEY_LOCAL_IP "IpLocal"
#define ONLINE_ACCOUNTS_AUTH_KEY_REMOTE_IP "IpRemote"
#define ONLINE_ACCOUNTS_AUTH_KEY_CHALLENGE "Challenge"
#define ONLINE_ACCOUNTS_AUTH_KEY_CHOSEN_MECHANISM "ChosenMechanism"
#define ONLINE_ACCOUNTS_AUTH_KEY_RESPONSE "Response"
#define ONLINE_ACCOUNTS_AUTH_KEY_STATE "state"
#  define ONLINE_ACCOUNTS_AUTH_SASL_STATE_FINISHED 0
#  define ONLINE_ACCOUNTS_AUTH_SASL_STATE_CONTINUE 1

#define ONLINE_ACCOUNTS_AUTH_KEY_USERNAME "Username"
#define ONLINE_ACCOUNTS_AUTH_KEY_PASSWORD "Password"


#endif // ONLINE_ACCOUNTS_DBUS_CONSTANTS_P_H
