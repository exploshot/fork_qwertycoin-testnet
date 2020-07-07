// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2020, The Qwertycoin Group.
//
// This file is part of Qwertycoin.
//
// Qwertycoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Qwertycoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Qwertycoin.  If not, see <http://www.gnu.org/licenses/>.

#include <boost/utility/value_init.hpp>
#include <CryptoNoteCore/BlockIndex.h>
#include <CryptoNoteCore/CryptoNoteSerialization.h>
#include <Serialization/SerializationOverloads.h>

namespace CryptoNote {

Crypto::Hash BlockIndex::getBlockId(uint32_t height) const
{
    assert(height < m_container.size());

    return m_container[static_cast<size_t>(height)];
}

std::vector<Crypto::Hash> BlockIndex::getBlockIds(uint32_t startBlockIndex, uint32_t maxCount) const
{
    std::vector<Crypto::Hash> result;
    if (startBlockIndex >= m_container.size()) {
        return result;
    }

    size_t count = std::min(
        static_cast<size_t>(maxCount),
        m_container.size() - static_cast<size_t>(startBlockIndex)
    );
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.push_back(m_container[startBlockIndex + i]);
    }

    return result;
}

std::vector<Crypto::Hash> BlockIndex::getBlockIds(uint32_t startBlockIndex,
                                                  uint32_t maxCount,
                                                  BlockchainDB &mDb) const
{
    std::vector<Crypto::Hash> result;
    if (startBlockIndex >= m_container.size()) {
        return result;
    }

    size_t count = std::min(
            static_cast<size_t>(maxCount),
            m_container.size() - static_cast<size_t>(startBlockIndex)
    );
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.push_back(m_container[startBlockIndex + i]);
    }

    return result;
}

bool BlockIndex::findSupplement(const std::vector<Crypto::Hash> &ids, uint32_t &offset) const
{
    for (const auto &id : ids) {
        if (getBlockHeight(id, offset)) {
            return true;
        }
    }

    return false;
}

bool BlockIndex::findSupplement(const std::vector<Crypto::Hash> &ids,
                    uint32_t &offset,
                    BlockchainDB &mDb) const
{
    // TODO: Check if this should return a vector by reference for offset
    for (const auto &id : ids) {
        try {
            offset = mDb.getBlockHeight(id);
        } catch (...) {
            std::exception e;
            throw e;

            return false;
        }
        return true;
    }
    return false;
}

std::vector<Crypto::Hash> BlockIndex::buildSparseChain(const Crypto::Hash &startBlockId) const
{
    assert(m_index.count(startBlockId) > 0);

    uint32_t startBlockHeight;
    getBlockHeight(startBlockId, startBlockHeight);

    std::vector<Crypto::Hash> result;
    auto sparseChainEnd = static_cast<size_t>(startBlockHeight + 1);
    for (size_t i = 1; i <= sparseChainEnd; i *= 2) {
        result.emplace_back(m_container[sparseChainEnd - i]);
    }

    if (result.back() != m_container[0]) {
        result.emplace_back(m_container[0]);
    }

    return result;
}

std::vector<Crypto::Hash> BlockIndex::buildSparseChain(const Crypto::Hash& startBlockId,
                                           BlockchainDB& mDb) const
{
    uint32_t startBlockHeight = mDb.getBlockHeight(startBlockId);

    std::vector<Crypto::Hash> result;
    size_t sparseChainEnd = static_cast<size_t>(startBlockHeight + 1);
    for (size_t i = 1; i <= sparseChainEnd; i *= 2) {
        result.emplace_back(mDb.getBlockHashFromHeight(sparseChainEnd - i));
    }

    if (result.back() != m_container[0]) {
        result.emplace_back(m_container[0]);
    }

    return result;
}

Crypto::Hash BlockIndex::getTailId() const
{
    assert(!m_container.empty());
    return m_container.back();
}

void BlockIndex::serialize(ISerializer &s)
{
    if (s.type() == ISerializer::INPUT) {
        readSequence<Crypto::Hash>(std::back_inserter(m_container), "index", s);
    } else {
        writeSequence<Crypto::Hash>(m_container.begin(), m_container.end(), "index", s);
    }
}

} // namespace CryptoNote
