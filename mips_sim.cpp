#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <bitset>
#include <unistd.h> 

std::string shift2( std::string bitstr ) { // Shift bitstrings by 2
    std::stringstream s;
    s << bitstr << "00";
    return s.str(); 
}

unsigned int pow( int num, int power ) { // For raising to powers of 2
    int val = 1;
    for( int i = 0; i != power; ++i ) {
        val *= num;
    }
    return val;
}

int btoi( std::string bitstr ) { // Convert bitstrings to ints.
    bitstr.erase(bitstr.find_last_not_of(" \n\r\t")+1);
    int val = 0;
    bool isNeg = false;
    std::stringstream s; 
    if ( bitstr.at(0) == '1' ) {
        isNeg = true;
        for ( int i = 0; i != bitstr.length(); ++i ) {
            if ( bitstr.at(i) == '1' ) {
                s << '0';
            } else if ( bitstr.at(i) == '0' ) {
                s << '1';
            }
        }
        bitstr = s.str();
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

    std::ifstream infile("sample2.txt");
    std::string str;
    std::vector<std::string> bins;
    int data_index = bins.size();
    while( getline( infile, str ) ) {
        bins.push_back( str ); // Saving for easier jumps.
        if ( str.substr(0,6).compare("000101") ) {
            data_index = bins.size();
        }
    }

    int cycle = 1;
    
    for ( int i = 0; i != data_index; ++i ) {
        std::cout << "cycle: " << cycle << std::endl;
        cycle++;
        std::cout << "ran: " << bins[i] << std::endl;
        int opcode;
        int category = std::stoi( bins[i].substr(0, 2) ); 
        switch ( category ) {
        case 0: {
            opcode = std::stoi( bins[i].substr(2,4) );
            switch ( opcode ) {
            case 0: { // J Instruction
                int imd = ( btoi( bins[i].substr(6, 26) ) * 4 );
                i = (( imd - 128 ) / 4) - 1; // Go to that index
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
            case 0: //ADDI
                ADDI( rs, rt, imd, r );
                break;
            case 1: //ANDI
                ANDI( rs, rt, imd, r );
                break;
            case 10: //ORI
                ORI( rs, rt, imd, r );
                break;
            case 11: //XORI
                XORI( rs, rt, imd, r );
                break;
            }
            break;
        }
        }
        std::cout << "REGISTERS" << std::endl;
        for ( int i = 0; i != 32; ++i )
            std::cout << r[i] << " ";
        std::cout << std::endl;
        std::cout << std::endl;
    }
}
