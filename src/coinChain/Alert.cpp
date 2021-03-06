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

#include <coinChain/Alert.h>

#include <coin/Block.h>
#include <coinChain/BlockIndex.h>
#include <coinChain/Peer.h>

#include <coin/Key.h>

#include "boost/foreach.hpp"

using namespace std;
using namespace boost;


//////////////////////////////////////////////////////////////////////////////
//
// Alert
//

string UnsignedAlert::toString() const
{
    std::string cancels;
    BOOST_FOREACH(int n, _cancels)
    cancels += strprintf("%d ", n);
    std::string subversions;
    BOOST_FOREACH(std::string str, _subversions)
    subversions += "\"" + str + "\" ";
    return strprintf(
                     "Alert(\n"
                     "    nVersion     = %d\n"
                     "    nRelayUntil  = %"PRI64d"\n"
                     "    nExpiration  = %"PRI64d"\n"
                     "    nID          = %d\n"
                     "    nCancel      = %d\n"
                     "    setCancel    = %s\n"
                     "    nMinVer      = %d\n"
                     "    nMaxVer      = %d\n"
                     "    setSubVer    = %s\n"
                     "    nPriority    = %d\n"
                     "    strComment   = \"%s\"\n"
                     "    strStatusBar = \"%s\"\n"
                     ")\n",
                     _version,
                     _relay_until,
                     _expiration,
                     _id,
                     _cancel,
                     cancels.c_str(),
                     _min_version,
                     _max_version,
                     subversions.c_str(),
                     _priority,
                     _comment.c_str(),
                     _status_bar.c_str());
}


map<uint256, Alert> mapAlerts;
/*
string getWarnings(string strFor)
{
    int nPriority = 0;
    string strStatusBar;
    string strRPC;
    if (GetBoolArg("-testsafemode"))
        strRPC = "test";

    // Misc warnings like out of disk space and clock is wrong
    if (strMiscWarning != "")
    {
        nPriority = 1000;
        strStatusBar = strMiscWarning;
    }

    // Longer invalid proof-of-work chain
    if (_bestIndex && _bestInvalidWork > _bestChainWork + _bestIndex->GetBlockWork() * 6)
    {
        nPriority = 2000;
        strStatusBar = strRPC = "WARNING: Displayed transactions may not be correct!  You may need to upgrade, or other nodes may need to upgrade.";
    }

    // Alerts
    CRITICAL_BLOCK(cs_mapAlerts)
    {
        BOOST_FOREACH(PAIRTYPE(const uint256, Alert)& item, mapAlerts)
        {
            const Alert& alert = item.second;
            if (alert.appliesToMe() && alert.getPriority() > nPriority)
            {
                nPriority = alert.getPriority();
                strStatusBar = alert.getStatusBar();
            }
        }
    }

    if (strFor == "statusbar")
        return strStatusBar;
    else if (strFor == "rpc")
        return strRPC;
    assert(!"GetWarnings() : invalid parameter");
    return "error";
}

bool Alert::relayTo(Peer* peer) const
{
    if (!isInEffect())
        return false;
    // returns true if wasn't already contained in the set
    if (peer->setKnown.insert(getHash()).second) {
        if (appliesTo(peer->nVersion, peer->strSubVer) ||
            appliesToMe() ||
            GetAdjustedTime() < _relay_until) {
            peer->PushMessage("alert", *this);
            return true;
            }
        }
    return false;
}
*/
bool Alert::checkSignature()
{
    CKey key;
    if (!key.SetPubKey(ParseHex("04fc9702847840aaf195de8442ebecedf5b095cdbb9bc716bda9110971b28a49e0ead8564ff0db22209e0374782c093bb899692d524e9d6a6956e7c5ecbcd68284")))
        return error("Alert::CheckSignature() : SetPubKey failed");
    if (!key.Verify(Hash(_message.begin(), _message.end()), _signature))
        return error("Alert::CheckSignature() : verify signature failed");
    
    // Now unserialize the data
    CDataStream s(_message);
    s >> *(UnsignedAlert*)this;
    return true;
}

bool Alert::processAlert()
{
    if (!checkSignature())
        return false;
    if (!isInEffect())
        return false;

    // Cancel previous alerts
    for (map<uint256, Alert>::iterator mi = mapAlerts.begin(); mi != mapAlerts.end();)
        {
        const Alert& alert = (*mi).second;
        if (cancels(alert))
            {
            printf("cancelling alert %d\n", alert._id);
            mapAlerts.erase(mi++);
            }
        else if (!alert.isInEffect())
            {
            printf("expiring alert %d\n", alert._id);
            mapAlerts.erase(mi++);
            }
        else
            mi++;
        }
    
    // Check if this alert has been cancelled
    BOOST_FOREACH(PAIRTYPE(const uint256, Alert)& item, mapAlerts)
    {
    const Alert& alert = item.second;
    if (alert.cancels(*this))
        {
        printf("alert already cancelled by %d\n", alert._id);
        return false;
        }
    }
    
    // Add to mapAlerts
    mapAlerts.insert(make_pair(getHash(), *this));

    printf("accepted alert %d\n", _id);
    return true;
}
