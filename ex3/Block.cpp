//
// Created by orenbm21 on 5/2/2015.
//

#include "Block.h"

Block::Block(int blockNum, Block *predecessor) {
    _predecessor = predecessor;
    _hashedData = NULL;
    _successors = NULL;
    _blockNum = blockNum;
}

Block *Block::getPredecessor() {
    return _predecessor;
}

Block::~Block() {
    if (!_successors.empty()) {
        for (list<int>::const_iterator iterator = _successors.begin(), end = _successors.end();
             iterator != end; ++iterator) {
            delete *iterator;
        }
    }
}