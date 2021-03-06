/*
 * This file is part of OnlineAccountsDaemon
 *
 * Copyright (C) 2016 Canonical Ltd.
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

#define NO_TR_OVERRIDE
#include "i18n.h"

#include <libintl.h>

namespace OnlineAccountsDaemon {

QString translate(const QString &text, const QString &domain)
{
    return QString::fromUtf8(dgettext(domain.toUtf8().constData(),
                                      text.toUtf8().constData()));
}

}  // namespace
