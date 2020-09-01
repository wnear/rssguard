// For license of this file, see <project-root-folder>/LICENSE.md.

//
// Copyright (C) 2011-2017 by Martin Rotter <rotter.martinos@gmail.com>
// Copyright (C) 2010-2014 by David Rosca <nowrep@gmail.com>
//
// RSS Guard is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RSS Guard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RSS Guard. If not, see <http://www.gnu.org/licenses/>.

#ifndef ADBLOCKURLINTERCEPTOR_H
#define ADBLOCKURLINTERCEPTOR_H

#include "network-web/urlinterceptor.h"

class AdBlockManager;

class AdBlockUrlInterceptor : public UrlInterceptor
{
    Q_OBJECT

public:
    explicit AdBlockUrlInterceptor(AdBlockManager *manager);

    void interceptRequest(QWebEngineUrlRequestInfo &info);

private:
    AdBlockManager *m_manager;
};

#endif // ADBLOCKURLINTERCEPTOR_H
