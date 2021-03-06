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

#ifndef CHAIN_H
#define CHAIN_H

#include <coin/Block.h>

#include <coinChain/Export.h>

#include <boost/noncopyable.hpp>
#include <boost/array.hpp>

class CBlockIndex;
class Transaction;

/// Pure virtual baseclass Chain defining a chain. Using this template you can define any bitcoin based cryptocurrency.

typedef boost::array<char, 4> MessageStart;

class COINCHAIN_EXPORT Chain : private boost::noncopyable
{
public:
    enum ChangeIdentifier {
        BIP0016 = 16,
        BIP0030 = 30
    };
    virtual const Block& genesisBlock() const = 0;
    virtual const uint256& genesisHash() const = 0;
    virtual const int64 subsidy(unsigned int height) const = 0;
    virtual bool isStandard(const Transaction& tx) const = 0;
    virtual unsigned int nextWorkRequired(const CBlockIndex* pindexLast) const = 0;
    virtual const CBigNum proofOfWorkLimit() const = 0;
    
    //    virtual const bool checkProofOfWork(uint256 hash, unsigned int nBits) const = 0;
    virtual bool checkPoints(const unsigned int height, const uint256& hash) const { return true; } // optional
    virtual unsigned int totalBlocksEstimate() const { return 0; } // optional
    
    virtual const std::string dataDirSuffix() const = 0;
    
    virtual unsigned int timeStamp(ChangeIdentifier id) const {
        switch(id) {
            case BIP0016: return 1333238400; // April 1st                
            case BIP0030: return 1331769600; // 03/15/2012 12:00am GMT.
        }
    }
    
    //    virtual char networkId() const = 0; // 0x00, 0x6f, 0x34 (bitcoin, testnet, namecoin)
    virtual ChainAddress getAddress(PubKeyHash hash) const = 0;
    virtual ChainAddress getAddress(ScriptHash hash) const = 0;
    virtual ChainAddress getAddress(std::string str) const = 0;
    
    /// signedMessageMagic is for using the wallet keys to sign messages - you can overload "Bitcoin" in other chains or leave it as is.
    virtual std::string signedMessageMagic() const { return "Bitcoin Signed Message:\n"; }
    
    virtual const MessageStart& messageStart() const = 0;
    virtual short defaultPort() const = 0;

    virtual std::string ircChannel() const = 0;
    virtual unsigned int ircChannels() const = 0; // number of groups to try (100 for bitcoin, 2 for litecoin)
};

class COINCHAIN_EXPORT BitcoinChain : public Chain
{
public:
    BitcoinChain();
    virtual const Block& genesisBlock() const ;
    virtual const uint256& genesisHash() const { return _genesis; }
    virtual const int64 subsidy(unsigned int height) const ;
    virtual bool isStandard(const Transaction& tx) const ;
    virtual const CBigNum proofOfWorkLimit() const { return CBigNum(~uint256(0) >> 32); }
    virtual unsigned int nextWorkRequired(const CBlockIndex* pindexLast) const ;
    //    virtual const bool checkProofOfWork(uint256 hash, unsigned int nBits) const;
    virtual bool checkPoints(const unsigned int height, const uint256& hash) const ;
    virtual unsigned int totalBlocksEstimate() const;

    virtual const std::string dataDirSuffix() const { return "bitcoin"; }
    
    //    virtual char networkId() const { return 0x00; } // 0x00, 0x6f, 0x34 (bitcoin, testnet, namecoin)
    virtual ChainAddress getAddress(PubKeyHash hash) const { return ChainAddress(0x00, hash); }
    virtual ChainAddress getAddress(ScriptHash hash) const { return ChainAddress(0x05, hash); }
    virtual ChainAddress getAddress(std::string str) const {
        ChainAddress addr(str);
        if(addr.version() == 0x00)
            addr.setType(ChainAddress::PUBKEYHASH);
        else if (addr.version() == 0x05)
            addr.setType(ChainAddress::SCRIPTHASH);
        return addr;
    }
    
    virtual const MessageStart& messageStart() const { return _messageStart; };
    virtual short defaultPort() const { return 8333; }
    
    virtual std::string ircChannel() const { return "bitcoin"; }
    virtual unsigned int ircChannels() const { return 100; } // number of groups to try (100 for bitcoin, 2 for litecoin)
    
private:
    Block _genesisBlock;
    uint256 _genesis;
    MessageStart _messageStart;
    typedef std::map<int, uint256> Checkpoints;
    Checkpoints _checkpoints;
};

extern const BitcoinChain bitcoin;

class COINCHAIN_EXPORT TestNetChain : public Chain
{
public:
    TestNetChain();
    virtual const Block& genesisBlock() const ;
    virtual const uint256& genesisHash() const { return _genesis; }
    virtual const int64 subsidy(unsigned int height) const ;
    virtual bool isStandard(const Transaction& tx) const ;
    virtual const CBigNum proofOfWorkLimit() const { return CBigNum(~uint256(0) >> 28); }
    virtual unsigned int nextWorkRequired(const CBlockIndex* pindexLast) const ;
    //    virtual const bool checkProofOfWork(uint256 hash, unsigned int nBits) const;
    
    virtual const std::string dataDirSuffix() const { return "bitcoin/testnet"; };
    
    virtual unsigned int timeStamp(ChangeIdentifier id) const {
        switch(id) {
            case BIP0016: return 1329264000; // Feb 15
            case BIP0030: return 1329696000; // 02/20/2012 12:00am GMT
        }
    }
    
    //    virtual char networkId() const { return 0x6f; } // 0x00, 0x6f, 0x34 (bitcoin, testnet, namecoin)
    virtual ChainAddress getAddress(PubKeyHash hash) const { return ChainAddress(0x6f, hash); }
    virtual ChainAddress getAddress(ScriptHash hash) const { return ChainAddress(0xc4, hash); }
    virtual ChainAddress getAddress(std::string str) const {
        ChainAddress addr(str);
        if(addr.version() == 0x6f)
            addr.setType(ChainAddress::PUBKEYHASH);
        else if (addr.version() == 0xc4)
            addr.setType(ChainAddress::SCRIPTHASH);
        return addr;
    }

    virtual const MessageStart& messageStart() const { return _messageStart; };
    virtual short defaultPort() const { return 18333; }
    
    virtual std::string ircChannel() const { return "bitcoinTEST"; }
    virtual unsigned int ircChannels() const { return 1; } // number of groups to try (100 for bitcoin, 2 for litecoin)
        
private:
    Block _genesisBlock;
    uint256 _genesis;
    MessageStart _messageStart;
};

extern const TestNetChain testnet;


#endif // CHAIN_H
