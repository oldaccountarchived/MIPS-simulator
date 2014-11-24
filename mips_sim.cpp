#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <bitset>

unsigned int pow( int num, int power ) { // For raising to powers of 2
    int val = 1;
    for( int i = 0; i != power; ++i ) {
        val *= num;
    }
    return val;
}

int btoi( std::string bitstr ) { // Convert bitstrings to ints.
    // Strip out those newlines.
    bitstr.erase(bitstr.find_last_not_of(" \n\r\t") + 1);
    int val = 0;
    bool isNeg = false;
    std::stringstream bitstr_out; 
    if ( bitstr.at(0) == '1' ) {
        isNeg = true;
        for ( int i = 0; i != bitstr.length(); ++i ) {
            if ( bitstr.at(i) == '1' ) {
                bitstr_out << '0';
            } else if ( bitstr.at(i) == '0' ) {
                bitstr_out << '1';
            }
        }
        bitstr = bitstr_out.str();
    }
    for ( int i = bitstr.length() - 1; i >= 0; --i ) {
        if ( bitstr.at(i) == '1' ) {
            val += pow( 2, ( bitstr.length() - 1 - i ) );
        }
    }
    if ( isNeg ) {
        return (val * -1) - 1;
    } else {
        return val;
    }
}

std::bitset<32> itob( int dec ) {
    if ( dec < 1 ) {
        std::bitset<32> tempset(dec);
        tempset = tempset.flip();
        std::bitset<32> finalset( tempset.to_ulong() + 1 );
        return finalset;
    } else { return std::bitset<32>(dec); }
}

void ADD(int rs, int rt, int rd, int* r) {
    r[rd] = r[rs] + r[rt];
}

void SUB(int rs, int rt, int rd, int* r) {
    r[rd] = r[rs] - r[rt];
}

void MUL(int rs, int rt, int rd, int* r) {
    r[rd] = r[rs] * r[rt];
}

void AND(int rs, int rt, int rd, int* r) {
    r[rd] = r[rs] & r[rt];
}

void OR(int rs, int rt, int rd, int* r) {
    r[rd] = r[rs] | r[rt];
}

void XOR(int rs, int rt, int rd, int* r) {
    r[rd] = r[rs] ^ r[rt];
}

void NOR(int rs, int rt, int rd, int* r) {
    r[rd] = ~(r[rs] | r[rt]);
}

void ADDI(int rs, int rt, int imd, int* r) {
    r[rt] = r[rs] + imd;
}

void ANDI(int rs, int rt, int imd, int* r) {
    r[rt] = r[rs] & imd;
}

void ORI(int rs, int rt, int imd, int* r) {
    r[rt] = r[rs] | imd;
}

void XORI(int rs, int rt, int imd, int* r) {
    r[rt] = r[rs] ^ imd;
}

int main() {
    int r[32]; // 32 register locations.
    for( int i = 0; i != 32; ++i ) {
        r[i] = 0;
    }
    std::ifstream infile("sample.txt");
    std::string str;
    std::vector<std::string> bins;
    int data_index = 0;
    while( getline( infile, str ) ) {
        bins.push_back( str ); // Saving for easier jumps.
        if ( !str.substr(0,6).compare("000101") ) {
            data_index = bins.size();
        }
    }
    if (data_index == 0)
        data_index = bins.size();
    std::vector<std::string> dsm(bins);
    std::stringstream dsm_out;
    int cycle = 0;
    for ( int i = 0; i < data_index; ++i ) {
        cycle++;
        int opcode;
        int category = std::stoi( bins[i].substr(0, 2) );
        switch ( category ) {
        case 0: {
            opcode = std::stoi( bins[i].substr(2,4) );
            switch ( opcode ) {
            case 0: { // J Instruction
                int imd = ( btoi( bins[i].substr(6, 26) ) * 4 );
                i = (( imd - 128 ) / 4) - 1; // Go to that index
                dsm_out << bins[i] << "/t" << "J #" << imd;
                dsm[i] = dsm_out.str();
                dsm_out.clear();
                break;
            }
            case 10: { // BEQ
                int rs = btoi( bins[i].substr(6, 5) );
                int rt = btoi( bins[i].substr(11, 5) );
                if ( r[rs] == r[rt] ) {
                    int imd = ( btoi( bins[i].substr(16, 16) ) );
                    i += imd;
                }
                break;
            }
            case 100: { //BGTZ
                int rs = btoi( bins[i].substr(6, 5) );
                if ( r[rs] > 0 ) {
                    int imd = ( btoi( bins[i].substr(16, 16) ) );
                    i += imd;
                }
                break;
            }
            case 101: { //BREAK
                i = bins.size() - 1; // Stops execution next cycle.
                break;
            }
            case 110: { //SW
                int rs = btoi( bins[i].substr(6, 5) );
                int rt = btoi( bins[i].substr(11, 5) );
                int imd = ( btoi( bins[i].substr(16, 16) ) );
                bins[( ( r[rs] + imd - 128 ) / 4 )] = itob( r[rt] ).to_string();
                break;
            }
            case 111: { //LW
                int rs = btoi( bins[i].substr(6, 5) );
                int rt = btoi( bins[i].substr(11, 5) );
                int imd = ( btoi( bins[i].substr(16, 16) ) );
                r[rt] = btoi( bins[( ( r[rs] + imd - 128 ) / 4 )] );
                break;
            }
            }
            break;
        }
        case 1: {
            opcode = std::stoi( bins[i].substr(12,4) );
            int rs = btoi( bins[i].substr(2,5) );
            int rt = btoi( bins[i].substr(7,5) );
            int rd = btoi( bins[i].substr(16,5) );
            switch ( opcode ) {
            case 0: // ADD
                ADD( rs, rt, rd, r );
                break;
            case 1: // SUB
                SUB( rs, rt, rd, r );
                break;
            case 10: // MUL
                MUL( rs, rt, rd, r );
                break;
            case 11: // AND
                AND( rs, rt, rd, r );
                break;
            case 100: // OR
                OR( rs, rt, rd, r );
                break;
            case 101: // XOR
                XOR( rs, rt, rd, r );
                break;
            case 110: // NOR
                NOR( rs, rt, rd, r );
                break;
            }
            break;
        }
        case 10: {
            opcode = std::stoi( bins[i].substr(12,4) );
            int rs = btoi( bins[i].substr(2,5) );
            int rt = btoi( bins[i].substr(7,5) );
            int imd = btoi( bins[i].substr(16, 16) );
            switch ( opcode ) {
            case 0: // ADDI
                ADDI( rs, rt, imd, r );
                break;
            case 1: // ANDI
                ANDI( rs, rt, imd, r );
                break;
            case 10: // ORI
                ORI( rs, rt, imd, r );
                break;
            case 11: // XORI
                XORI( rs, rt, imd, r );
                break;
            }
            break;
        }
        }
        std::cout << data_index << std::endl;
        std::cout << "--------------------" << std::endl;
        std::cout << "Cycle:" << cycle << std::endl << std::endl;
        std::cout << "Registers" << std::endl;
        std::cout << "R00:\t";
        for ( int i = 0; i != 8; ++i )
            std::cout << r[i] << "\t";
        std::cout << std::endl;
        std::cout << "R08:\t";
        for ( int i = 8; i != 16; ++i )
            std::cout << r[i] << "\t";
        std::cout << std::endl;
        std::cout << "R16:\t";
        for ( int i = 16; i != 24; ++i )
            std::cout << r[i] << "\t";
        std::cout << std::endl;
        std::cout << "R24:\t";
        for ( int i = 24; i != 32; ++i )
            std::cout << r[i] << "\t";
        std::cout << std::endl;
        std::cout << std::endl;
        if (data_index != bins.size())
            std::cout << "Data" << std::endl;
        for (int i = data_index; i < bins.size(); ++i) {
            if (i == data_index) {
                std::cout << (128 + (i * 4)) << ":\t";
            }
            if ((i - data_index) % 8 == 0 && i != data_index) {
                std::cout << std::endl;
                std::cout << (128 + (i * 4)) << ":\t";
            }
            std::cout << btoi(bins[i]) << "\t";
        }
    }
}
