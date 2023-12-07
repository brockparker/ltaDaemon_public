#ifndef _dataConversion_h_
#define _dataConversion_h_

#include <string.h>
#include <vector>
#include <deque>
#include <map>
#include <fstream>

#include "fitsio.h"

#ifdef USEROOT
    #include "TFile.h"
    #include "TTree.h"
    #include "TBranch.h"
#endif

#include "smartSequencerHandler.h"

#define INTERLEAVING_NUM_INTEG_PER_PIXEL 3
#define SMART_FLAG_NOT_REAL_PIXELS -1
#define ADC_NUM_BITS 18
#define LTA_NUMBER_OF_CHANNELS 4

//number of samples in a "raw" readout
#define SMART_BUFFER_SIZE 65536

// max number of 64-bit words in a packet - this corresponds to LTA_SOFT_DATABUFF_SIZE=2000
// a typical packet has 182 words (this is set in the firmware)
#define PACKETBUFF_SIZE 250

using namespace std;

#ifdef USEROOT
    struct dataVars  {
    //this struct holds variables that can be saved to a ROOT TTree

    Short_t cntr;
    Short_t hdr;
    Short_t id;
    Int_t data;
    };
#endif

class dataFileReader {
    public:
        dataFileReader(const vector<string> * inFileNames, int gVerbosityTranslate);
        ~dataFileReader();
        bool getWord(uint64_t &data);
        long long getDataCount() {return dataCount_;};
        long long getMissingPackets() {return missingPackets;};
        long long getDataGaps() {return dataGaps;};
        void setupTextDump(const string outFileName);
        void deleteDatFiles();

        static uint8_t decodeCounter(uint64_t word64);
        static uint8_t decodeID(uint64_t word64);
        static int32_t decodeDataValueSigned(uint64_t word64);//for standard readout (non-raw)
        static uint8_t decodeHeader(uint64_t word64);
        static int32_t decodeADCValueSigned(uint64_t word64, int firstBit);

    private:
        void openFile(uint i);

        const vector<string> * inFileNames_;
        deque<uint64_t> dataBuffer_;
        FILE *fin;
        long long dataCount_; //number of 64-bit data words read from files
        long long packetCount_; //number of UDP packets read from files
        long long missingPackets; //number of packet index mismatches
        long long dataGaps; //number of data index mismatches
        int dataIndexOld; //data index from previous 64-bit word
        int packPreviousIndex; //packer index from previous UDP packet
        uint iFile;
        int gVerbosityTranslate_;

        //for text file dump
        bool textFileFlag;
        long fileLength;
        ofstream myfile;

        #ifdef USEROOT
            TFile *fout;
            TTree *dataTree;
            dataVars dataEntry;
        #endif
};

struct imageVar {
    string name;
    string value;
    string comment;
};

int mean(const vector <double> data, double &result);
int covariance(const vector <double> data1, const vector <double> data2, double &result);

class FitsWriter
{
    public:
        FitsWriter(const int nHdu, const long nCols, const long nRows, const string outFString, const bool isInteger, const bool isCompressed);
        ~FitsWriter();
        void add_extra_var(const string extraVar);
        int create_img(const vector<imageVar> vars, bool writeTimestamps=false);
        int close();
        int write_vars(const vector<imageVar> vars, int hdunum=-1);
        bool save_counts(int hdunum, int chID, int runNum);
        template <typename T> int dump_to_fits(int hdunum, long * fpixel, vector<T> &pixels, int npix);
        template <typename T> int dump_to_fits(int hdunum, vector<T> &pixels, bool clearVector=true, int npix=-1);
        long getPixelCount(int hdunum) {return pixelCounts[hdunum];};
        long getPixExpected() {return pixExpected;};
    private:
        string outFString_;
        fitsfile* outfptr_;
        map<int, long> pixelCounts;
        int nHdu_;
        long nCols_;
        long nRows_;
        bool isInteger_;
        long pixExpected;
        bool isCompressed_;
        bool writeTimestamps_;
        bool fileOpen;
        vector<string> extraVars_;
};

int compressFits(const char* infits, const char* outfits);

class DataConversion
{
    public:
        DataConversion();
        void setWriteTimestamps(int writeTimestamps){writeTimestamps_ = writeTimestamps;};
        int getWriteTimestamps(){return writeTimestamps_;};
        void setDeleteDats(int deleteDats){deleteDats_ = deleteDats;};
        int getDeleteDats(){return deleteDats_;};
        void setKeepTemp(int keepTemp){keepTemp_ = keepTemp;};
        int getKeepTemp(){return keepTemp_;};

        void setInFileNames(const vector <string> * inFileNames){inFileNames_=inFileNames;};

        int bin_to_fits(const string outFileName, const string tempFileName, const uint nCols, const uint nRows, const uint runNum, const vector<imageVar> vars);
        int bin_to_fits_smart(const string outFileName, const smartImageDimensions_t dims, const vector<imageVar> vars);
        int bin_to_fits_interleaving(const string outFileName, const uint nCols, const uint nRows, const vector<imageVar> vars);
        int bin_to_raw(const string outFileName);
        int bin_to_calibrate(const string outFileName, const unsigned int regMinCol, const unsigned int regMaxCol, const unsigned int regMinRow, const unsigned int regMaxRow, const unsigned int imageNCols, const vector<imageVar> vars);

        void print_interleaving();
        string get_last_filename() {return lastFilename_;};
    private:
        int textFileFlag;
        int gVerbosityTranslate;
        //gain matrix for interleaving
        vector <vector <double>> gainMatrix;
        int writeTimestamps_;
        int deleteDats_;
        int keepTemp_;
        const vector<string> * inFileNames_;
        string lastFilename_;

        template <typename T> int compute_pixel_value_smart(FitsWriter &outfAll, FitsWriter &outfAve, const smartImageDimensions_t dims, const vector<vector <T>> &pixelsVector, vector <uint> &rowInd, vector <uint> &colInd, vector<vector <T>> &fRowImageAll);
        int compute_pixel_value(FitsWriter &outf, vector<vector <float>> &pixelsVector);
};

#endif
