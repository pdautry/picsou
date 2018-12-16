/*
 *  Picsou | Keep track of your expenses !
 *  Copyright (C) 2018  koromodako
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "picsoumodelobj.h"

PicsouModelObj::~PicsouModelObj()
{

}

PicsouModelObj::PicsouModelObj(bool valid, PicsouModelObj *parent) :
    _id(QUuid::createUuid()),
    _valid(valid),
    _parent(parent)
{
    if(parent!=nullptr) {
        connect(this, &PicsouModelObj::modified, parent, &PicsouModelObj::modified);
    }
}

