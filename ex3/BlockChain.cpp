//
// Created by orenbm21 on 5/2/2015.
//

class BlockChain {

private:
    Block *_endOfLongest;
    Block *_genesis;
    int _chainSize;
    bool _isInited;
    bool _isClosing;

public:

    /*
     * constructor
     */
    BlockChain();

    int getChainSize() const {
        return _chainSize;
    }

    void init();

    void addBlock();


};

BlockChain::BlockChain()
{
    init();
}

void BlockChain::init() {
    _genesis = new Block(0, NULL);
    _endOfLongest = _genesis;
    _chainSize = 0;
    _isInited = true;
    _isClosing = false;
}

