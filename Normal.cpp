#include <iostream>

#include <string>
#include <sys/sysinfo.h>

#include "Tick.h"
#include "LogMessage.h"
#include "MPIFileSegment.h"
#include "BTime.h"
#include "RunningStats.h"

using namespace std;
using namespace std::chrono;
using namespace mirabella;

char* get_next_line(char** current_position, int& bytesLeft);
RunningStats process_section(char* inbuffer, const int size);
unsigned long whats_quarter_memory();

int blocksize = 100000;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <infile> " << std::endl;
        return -1;
    }

    std::string infile(argv[1]);

    int tid, NCPUS;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &tid);
    MPI_Comm_size(MPI_COMM_WORLD, &NCPUS);

    RunningStats rs;

     if(tid == 0) {
        cout << LogMessage(LogMessage::LogLevel::LOG_INFO, "Reading from file: ", infile).str() << endl;;
    }
    try
    {
        MPIFileSegment IMFS(infile, NCPUS, tid);

        unsigned long mem = whats_quarter_memory();
        if(IMFS.GetFileSize() < mem)
            blocksize = IMFS.GetFileSize();
        else
            blocksize = mem;
        cout << LogMessage(LogMessage::LogLevel::LOG_INFO, "File Size: ", IMFS.GetFileSize(), "in chunks of", blocksize, " bytes").str() << endl;;
        char* inbuffer = new char[blocksize];

        int bytesRead = 0;
        int offset = 0;
        do
        {
            bytesRead =  IMFS.ReadBlockAt(blocksize, inbuffer, offset);

            if(bytesRead > 0)
            {
                rs += process_section(inbuffer,  bytesRead);
            }
            offset += bytesRead;
        } while(bytesRead > 0);

        delete [] inbuffer;
    }
    catch(const std::invalid_argument& e)
    {
       std::cout << "MAIN: Caught an invalid argument exception: " << e.what() << std::endl;
       cout << LogMessage(LogMessage::LogLevel::LOG_FATAL,"MAIN: Caught an invalid argument exception: ", e.what()).str() << endl;;
    }

    cout << LogMessage(LogMessage::LogLevel::LOG_INFO, "Skewness=", rs.Skewness(), " Kurtosis=", rs.Kurtosis()).str() << endl;

    MPI_Finalize();

    return 0;
}


RunningStats process_section(char* inbuffer, const int size)
{
    RunningStats stats;

    char* p = nullptr;
    int bytes_in_buffer = size;
    char* remaining_buffer = inbuffer;
    Tick oldtick;
    Tick newtick;

    while( (p = get_next_line(&remaining_buffer, bytes_in_buffer)) != nullptr) {
        
        // Check for the "#" symbol at the begining of the line that will mark this tick as bad.
        if(*p == '#')
            continue;

        newtick = Tick(p);

        if(oldtick.get_price() == 0.0)
        {
            oldtick = newtick;
            continue;
        }

        double pricereturn = (newtick.get_price() - oldtick.get_price()) / (oldtick.get_price());
        stats.Push(pricereturn);
    
        oldtick = newtick;
    }

    return stats;
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



double get_return(const Tick& oldtick, const Tick& newtick)
{
    double _return = 0.0;

    double timedifference = newtick.get_timestamp().get_seconds() - oldtick.get_timestamp().get_seconds();
    double pricereturn = (newtick.get_price() - oldtick.get_price()) / (oldtick.get_price());
}
