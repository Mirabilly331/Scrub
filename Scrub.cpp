#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <thread>
#include <chrono>

#include "LogMessage.h"
#include "Logger.h"
#include "MPIFileSegment.h"
#include "Tick.h"

#include <ctime>

#include <mpi.h>

#include <ctype.h>
#include <stdio.h>
#include <sys/sysinfo.h>


using namespace std;
using namespace std::chrono;
using namespace mirabella;

char* get_next_line(char** current_position, int& bytesLeft);
void process_section(char* inbuffer, char* outbuffer, const int size);
unsigned long whats_quarter_memory();

int blocksize = 100000;
int windowsize = 1000;

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <infile>  <outfile>" << std::endl;
        return -1;
    }
        
    std::string infile(argv[1]);
    std::string outfile(argv[2]);

    int tid, NCPUS;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &tid);
    MPI_Comm_size(MPI_COMM_WORLD, &NCPUS);

    if(tid == 0) {
        cout << LogMessage(LogMessage::LogLevel::LOG_INFO, "Reading from file: ", infile, " writing to file: ", outfile).str() << endl;; 
    }
    try
    {

            high_resolution_clock::time_point t1, t2;
            duration<double> reading_time_span = duration<double>::zero(); 
            duration<double> writing_time_span = duration<double>::zero(); 
            duration<double> processing_time_span = duration<double>::zero(); 

            MPIFileSegment IMFS(infile, NCPUS, tid);
            MPIFileSegment OMFS(outfile, NCPUS, tid, MPI_MODE_CREATE|MPI_MODE_WRONLY, IMFS.GetFileSize()); 

            unsigned long mem = whats_quarter_memory();     
            if(IMFS.GetFileSize() < mem)
                blocksize = IMFS.GetFileSize();
            else
                blocksize = mem;
            cout << LogMessage(LogMessage::LogLevel::LOG_INFO, "File Size: ", IMFS.GetFileSize(), "in chunks of", blocksize, " bytes").str() << endl;; 

            

            char* inbuffer = new char[blocksize];
            char* outbuffer = new char[blocksize];
            
            int bytesRead = 0;
            int offset = 0;
            do
            {
                t1 = high_resolution_clock::now();
                bytesRead =  IMFS.ReadBlockAt(blocksize, inbuffer, offset);
                t2 = high_resolution_clock::now();
                reading_time_span += duration_cast<duration<double>>(t2 - t1);
                

                if(bytesRead > 0)
                {
                    t1 = high_resolution_clock::now();
                    process_section(inbuffer, outbuffer,  bytesRead);
                    t2 = high_resolution_clock::now();
                    processing_time_span += duration_cast<duration<double>>(t2 - t1);

                    t1 = high_resolution_clock::now();
                    OMFS.WriteBlockAt(blocksize, outbuffer, offset);
                    t2 = high_resolution_clock::now();
                    writing_time_span += duration_cast<duration<double>>(t2 - t1);
                }
                offset += bytesRead;
            } while(bytesRead > 0);
   
            {
                cout << LogMessage(LogMessage::LogLevel::LOG_INFO, "Reading Time=" , reading_time_span.count()).str() << endl;; 
                cout << LogMessage(LogMessage::LogLevel::LOG_INFO, "Writing Time=" , writing_time_span.count()).str() << endl;; 
                cout << LogMessage(LogMessage::LogLevel::LOG_INFO, "Processing Time=" , processing_time_span.count()).str() << endl;; 
            }

            double send_data[3] = {reading_time_span.count(), writing_time_span.count(), processing_time_span.count()};
            double recv_data[3];
            MPI_Reduce(send_data, recv_data, 3, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            if(tid == 0) {
                cout << LogMessage(LogMessage::LogLevel::LOG_INFO, "Total Reading Time=" , recv_data[0]).str() << endl;
                cout << LogMessage(LogMessage::LogLevel::LOG_INFO, "Total Writing Time=" , recv_data[1]).str() << endl;
                cout << LogMessage(LogMessage::LogLevel::LOG_INFO, "Total Processing Time=" , recv_data[2]).str() << endl;
            }
     
            delete [] inbuffer;
            delete [] outbuffer;
    }
    catch(const std::invalid_argument& e)
    {
       std::cout << "MAIN: Caught an invalid argument exception: " << e.what() << std::endl;
       cout << LogMessage(LogMessage::LogLevel::LOG_FATAL,"MAIN: Caught an invalid argument exception: ", e.what()).str() << endl;; 
    }


    MPI_Finalize();
    return 0;
}

void process_section(char* buffer, char* outbuffer, const int size)
{
    vector<Tick> tickVector;

    // Populate the tick vector with data from the input buffer 
    char* p = nullptr;
    int bytes_in_buffer = size;
    char* remaining_buffer = buffer;
    while( (p = get_next_line(&remaining_buffer, bytes_in_buffer)) != nullptr) {
        Tick T(p);
        tickVector.push_back(T);
    }

    auto windowbottom = tickVector.begin();
    TickTimeLessThan cmp;
    for(auto windowtop = windowbottom + windowsize; windowtop < tickVector.end(); windowtop = windowbottom + windowsize)
    {
        std::sort(windowbottom, windowtop, cmp);  
        windowbottom += windowsize / 2;
    }

    // Now run the validators
    TickTimeValidator ttv;
    TickPriceValidator tpv;
    TickVolumeValidator tvv;
    TickValidator TV(ttv, tpv, tvv);
    for_each(tickVector.begin(), tickVector.end(), TV);    

    // Write the tick vector to the output buffer
    p = outbuffer;
    int xcnt = 0;
    for(auto i = tickVector.begin(); i < tickVector.end(); ++i)
    {
        const char* q = i->get_data();
        // Bad ticks get their leading character replaced with a #
        if(i->get_tickstate() != TickState::TICK_VALID)
        {
            *p++ = '#';
             q++;
             ++xcnt;
        }
        while(*q != '\n' && xcnt < size) {
            *p++ = *q++;
             ++xcnt;
        }
        if(xcnt < size) {
            *p++ = '\n';
            ++xcnt;
        }
    }
}

char* get_next_line(char** current_position, int& bytesLeft)
{
    if(bytesLeft == 0){
        return nullptr;
    }

   char* p = *current_position;
   char* result = *current_position;

   // Advance the pointer to the start of the next record
   while(*p++ != '\n')
   {
        --bytesLeft;
        if(bytesLeft == 0)
            break;
        if(*p == '\0')
            break;
   }
   if(bytesLeft > 0)
        --bytesLeft;

   *current_position = p;
   return result;
}

unsigned long whats_quarter_memory()
{
    struct sysinfo info;
    int rc = sysinfo(&info);
    if(rc != 0) {
        throw invalid_argument("sysinfo");
    }
    
    return info.totalram / 4;
}

