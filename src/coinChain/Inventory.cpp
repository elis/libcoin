/* -*-c++-*- libcoin - Copyright (C) 2012 Michael Gronager
 *
 * libcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * libcoin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libcoin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <coinChain/Inventory.h>
#include <coin/util.h>

static const char* ppszTypeName[] =
{
    "ERROR",
    "tx",
    "block",
};

Inventory::Inventory()
{
    _type = 0;
    _hash = 0;
}

Inventory::Inventory(int type, const uint256& hash)
{
    _type = type;
    _hash = hash;
}

Inventory::Inventory(const std::string& type_name, const uint256& hash)
{
    int i;
    for (i = 1; i < ARRAYLEN(ppszTypeName); i++)
    {
        if (type_name == ppszTypeName[i])
        {
            _type = i;
            break;
        }
    }
    if (i == ARRAYLEN(ppszTypeName))
        throw std::out_of_range(strprintf("Inventory::Inventory(string, uint256) : unknown type '%s'", type_name.c_str()));
    _hash = hash;
}

bool operator<(const Inventory& a, const Inventory& b)
{
    return (a._type < b._type || (a._type == b._type && a._hash < b._hash));
}

bool Inventory::isKnownType() const
{
    return (_type >= 1 && _type < ARRAYLEN(ppszTypeName));
}

const char* Inventory::getCommand() const
{
    if (!isKnownType())
        throw std::out_of_range(strprintf("Inventory::GetCommand() : type=%d unknown type", _type));
    return ppszTypeName[_type];
}

std::string Inventory::toString() const
{
    return strprintf("%s %s", getCommand(), _hash.toString().substr(0,20).c_str());
}

void Inventory::print() const
{
    printf("Inventory(%s)\n", toString().c_str());
}
