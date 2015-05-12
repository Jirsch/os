//
// Created by orenbm21 on 5/2/2015.
//

#include "Block.h"


Block::Block(int blockNum, Block *predecessor)
{
    _blockNum = blockNum;
    _predecessor = predecessor;
    _hashedData = NULL;
    _successors = list<Block *>();
    _toLongest = false;
    _chainLength = 0;
}

void Block::setPredecessor(Block *predecessor)
{
    _predecessor = predecessor;

    if (isGenesis())
    {
        _chainLength = 0;
    }

    else
    {
        _chainLength = predecessor->getChainLength() + 1;
    }
}

bool Block::isGenesis()
{
    return _predecessor == NULL;
}

Block *Block::getPredecessor()
{
    return _predecessor;
}

Block::~Block()
{
    if (_hashedData != NULL)
    {
        delete _hashedData;
    }
}

void Block::addSuccessor(Block *toAdd)
{
    this->_successors.push_back(toAdd);
}
