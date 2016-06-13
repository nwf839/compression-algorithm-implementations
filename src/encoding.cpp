#include <istream>

#include "filelib.h"
#include "pqueue.h"
#include "map.h"
#include "encoding.h"

Map<int, int> buildFrequencyTable(istream& input) {
    Map<int, int> freqTable;
    char ch;
    while (input.get(ch)) freqTable[int(ch)]++;
    freqTable[PSEUDO_EOF] = 1;
    return freqTable;
}

HuffmanNode* buildEncodingTree(const Map<int, int>& freqTable) {
    PriorityQueue<HuffmanNode *> pq;
    for (const int &key : freqTable)
    {
        HuffmanNode *leaf = new HuffmanNode;
        leaf->character = key;
        leaf->count = freqTable[key];
        leaf->zero = NULL;
        leaf->one = NULL;
        pq.enqueue(leaf, freqTable[key]);
    }
    while (pq.size() > 1)
    {
        HuffmanNode *root = new HuffmanNode;
        root->character = NOT_A_CHAR;
        root->zero = pq.dequeue();
        root->one = pq.dequeue();
        root->count = root->zero->count + root->one->count;
        pq.enqueue(root, root->count);
    }
    return pq.dequeue();
}

void buildEncodingMap(Map<int, std::string> &encodingMap, HuffmanNode* encodingTree, const std::string& soFar)
{
    if (encodingTree->character) encodingMap[encodingTree->character] = soFar;
    if (encodingTree->zero != NULL) buildEncodingMap(encodingMap, encodingTree->zero, soFar + '0');
    if (encodingTree->one != NULL) buildEncodingMap(encodingMap, encodingTree->one, soFar + '1');
}

Map<int, std::string> buildEncodingMap(HuffmanNode* encodingTree) {
    Map<int, string> encodingMap;
    buildEncodingMap(encodingMap, encodingTree, "");
    return encodingMap;
}

void encodeData(istream& input, const Map<int, string>& encodingMap, obitstream& output) {
    char ch;
    while (input.get(ch))
    {
        for (int i = 0; i < encodingMap[int(ch)].length(); i++)
        {
            output.writeBit(int(encodingMap[ch][i]) - 48);
        }
    }
    for (int i = 0; i < encodingMap[PSEUDO_EOF].length(); i++)
    {
        output.writeBit(int(encodingMap[PSEUDO_EOF][i]) - 48);
    }
}

void decodeData(ibitstream& input, HuffmanNode* encodingTree, ostream& output) {
    HuffmanNode *cursor = encodingTree;
    int bit = 0;
    while (bit != -1)
    {
        bit = input.readBit();
        if (cursor->character == PSEUDO_EOF)
        {
            return;
        }
        if (cursor->character != NOT_A_CHAR)
        {
            output.put(char(cursor->character));
            cursor = encodingTree;
        }

        if (bit == 0) cursor = cursor->zero;
        else cursor = cursor->one;
    }
}


void compress(istream& input, obitstream& output) {
    Map<int, int> freqTable;
    Map<int, std::string> encodingMap;
    freqTable = buildFrequencyTable(input);
    rewindStream(input);
    output << freqTable;
    HuffmanNode *encodingTree = buildEncodingTree(freqTable);
    encodingMap = buildEncodingMap(encodingTree);
    encodeData(input, encodingMap, output);
}

void decompress(ibitstream& input, ostream& output) {
    Map<int, int> freqTable;
    input >> freqTable;
    HuffmanNode *encodingTree = buildEncodingTree(freqTable);
    decodeData(input, encodingTree, output);
}

void freeTree(HuffmanNode* node) {
    if (node != NULL)
    {
        freeTree(node->zero);
        freeTree(node->one);
        delete node;
    }
}
