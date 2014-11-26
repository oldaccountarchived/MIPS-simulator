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

int main( int argc, char *argv[] ) {
    int r[32]; // 32 register locations.
    for( int i = 0; i != 32; ++i ) {
        r[i] = 0;
    }
    std::ifstream infile( argv[1] );
    std::ofstream dis_stream;
    dis_stream.open("dissasembly.txt");
    std::ofstream sim_stream;
    sim_stream.open("simulation.txt");
    std::string str;
    std::vector<std::string> bins;
    int data_index = 0;
    while( getline( infile, str ) ) {
        bins.push_back( str.erase(str.find_last_not_of(" \n\r\t") + 1 ) );
        if ( !str.substr(0,6).compare("000101") ) {
            data_index = bins.size();
        }
    }
    if (data_index == 0)
        data_index = bins.size();
    std::vector<std::string> dsm(bins);
    std::stringstream dsm_out;
    for (int i = data_index; i < dsm.size(); i++) {
        dsm_out << bins[i] << "\t" << (128 + i * 4) << "\t" << btoi(bins[i]);
        dsm[i] = dsm_out.str();
        dsm_out.str(std::string());
    }
    int cycle = 0;
    for ( int i = 0; i < data_index; ++i ) {
        int org_index = i;
        cycle++;
        int opcode;
        int category = std::stoi( bins[i].substr(0, 2) );
        switch ( category ) {
        case 0: {
            opcode = std::stoi( bins[i].substr(2,4) );
            switch ( opcode ) {
            case 0: { // J Instruction
                int imd = ( btoi( bins[i].substr(6, 26) ) * 4 );
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tJ #" << imd;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                i = (( imd - 128 ) / 4) - 1; // Go to that index
                break;
            }
            case 10: { // BEQ
                int rs = btoi( bins[i].substr(6, 5) );
                int rt = btoi( bins[i].substr(11, 5) );
                int imd = ( btoi( bins[i].substr(16, 16) ) );
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tBEQ R" << rs << ", R" << rt << ", #" << (imd * 4);
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                if ( r[rs] == r[rt] ) {
                    i += imd;
                }
                break;
            }
            case 100: { //BGTZ
                int rs = btoi( bins[i].substr(6, 5) );
                int imd = ( btoi( bins[i].substr(16, 16) ) );
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tBGTZ R" << rs << ", #" << (imd * 4);
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                if ( r[rs] > 0 ) {
                    i += imd;
                }
                break;
            }
            case 101: { //BREAK
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tBREAK";
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                i = bins.size() - 1; // Stops execution next cycle.
                break;
            }
            case 110: { //SW
                int rs = btoi( bins[i].substr(6, 5) );
                int rt = btoi( bins[i].substr(11, 5) );
                int imd = ( btoi( bins[i].substr(16, 16) ) );
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tSW R" << rt << ", " << imd << "(R" << rs << ")";
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                bins[( ( r[rs] + imd - 128 ) / 4 )] = itob( r[rt] ).to_string();
                break;
            }
            case 111: { //LW
                int rs = btoi( bins[i].substr(6, 5) );
                int rt = btoi( bins[i].substr(11, 5) );
                int imd = ( btoi( bins[i].substr(16, 16) ) );
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tLW R" << rt << ", " << imd << "(R" << rs << ")";
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
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
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tADD R" << rd << ", R" << rs << ", R" << rt;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                ADD( rs, rt, rd, r );
                break;
            case 1: // SUB
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tSUB R" << rd << ", R" << rs <<  ", R" << rt;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                SUB( rs, rt, rd, r );
                break;
            case 10: // MUL
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tMUL R" << rd << ", R" << rs  << ", R" << rt;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                MUL( rs, rt, rd, r );
                break;
            case 11: // AND
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tAND R" << rd << ", R" << rs <<  ", R" << rt;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                AND( rs, rt, rd, r );
                break;
            case 100: // OR
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tOR R" << rd << ", R" << rs << ", R" << rt;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                OR( rs, rt, rd, r );
                break;
            case 101: // XOR
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tXOR R" << rd << ", R" << rs <<  ", R" << rt;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                XOR( rs, rt, rd, r );
                break;
            case 110: // NOR
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tNOR R" << rd << ", R" << rs << ", R" << rt;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
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
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tADDI R" << rt << ", R" << rs << ", #" << imd;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                ADDI( rs, rt, imd, r );
                break;
            case 1: // ANDI
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tANDI R" << rt << ", R" << rs << ", #" << imd;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                ANDI( rs, rt, imd, r );
                break;
            case 10: // ORI
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tORI R" << rt << ", R" << rs << ", #" << imd;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                ORI( rs, rt, imd, r );
                break;
            case 11: // XORI
                dsm_out << bins[i] << "\t" << (128 + i * 4) << "\tXORI R" << rt << ", R" << rs << ", #" << imd;
                dsm[i] = dsm_out.str();
                dsm_out.str(std::string());
                XORI( rs, rt, imd, r );
                break;
            }
            break;
        }
        }
        sim_stream << std::endl << std::endl;
        sim_stream << "--------------------" << std::endl;
        sim_stream << "Cycle:" << cycle << " " << (128 + org_index * 4) << "\t"  << dsm[org_index].substr(37, std::string::npos) << std::endl << std::endl;
        sim_stream << "Registers" << std::endl;
        sim_stream << "R00:\t";
        for ( int i = 0; i != 8; ++i )
            sim_stream << r[i] << "\t";
        sim_stream << std::endl;
        sim_stream << "R08:\t";
        for ( int i = 8; i != 16; ++i )
            sim_stream << r[i] << "\t";
        sim_stream << std::endl;
        sim_stream << "R16:\t";
        for ( int i = 16; i != 24; ++i )
            sim_stream << r[i] << "\t";
        sim_stream << std::endl;
        sim_stream << "R24:\t";
        for ( int i = 24; i != 32; ++i )
            sim_stream << r[i] << "\t";
        sim_stream << std::endl;
        sim_stream << std::endl;
        if (data_index != bins.size())
            sim_stream << "Data" << std::endl;
        for (int i = data_index; i < bins.size(); ++i) {
            if (i == data_index) {
                sim_stream << (128 + (i * 4)) << ":\t";
            }
            if ((i - data_index) % 8 == 0 && i != data_index) {
                sim_stream << std::endl;
                sim_stream << (128 + (i * 4)) << ":\t";
            }
            sim_stream << btoi(bins[i]) << "\t";
        }
    }
    sim_stream << std::endl;
    for (int i = 0; i < dsm.size(); ++i) {
        dis_stream << dsm[i] << std::endl;
    }
    dis_stream << std::endl;
}
