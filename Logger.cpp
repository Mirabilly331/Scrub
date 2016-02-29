#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>

#include "Logger.h"

using namespace mirabella;

constexpr int kBufferSize = 256;

constexpr char SignOff = '!';
constexpr char  SignOn = '~';

static mirabella::Logger *L; 

Logger::Logger(int id) : l_id(id)
{
}

void Logger::Log(LogMessage& message)
{
    send_message(message.str());
}

void Logger::Log(LogMessage message)
{
    send_message(message.str());
}

void Logger::send_message(std::string message)
{
    int mpi_rc = 0;
    int count = message.size();

    mpi_rc = MPI_Send(message.c_str(), count, MPI_CHAR, l_id, 0, MPI_COMM_WORLD);
    if(mpi_rc != 0) {
        throw std::runtime_error("MPI_Send error");
    }
}


LogWriter::LogWriter(std::string filename) 
    : lw_filename(filename), lw_buffer(nullptr)
{
    lw_ofstream.open(filename); 
    if(lw_ofstream.is_open() != true) {
       throw std::runtime_error("invalid filename: " + filename); 
    }
    lw_buffer = new char[kBufferSize];

    LogMessage L(LogMessage::LogLevel::LOG_INFO, "Log Writer initialized... witing for messages.");
    lw_ofstream << L.str() << std::endl;
    
    process_messages();
}


LogWriter::~LogWriter()
{
    if(lw_buffer == nullptr) delete [] lw_buffer;
    lw_ofstream.close();
}

void LogWriter::process_messages()
{
   MPI_Status status;   
   int mpi_rc = 0;

   do
   {
       // std::cout << "LogWriter::process_messages about to call MPI_Recv" << std::endl;
       mpi_rc = MPI_Recv(lw_buffer, kBufferSize, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
       lw_ofstream << lw_buffer << std::endl;   
   } while(true);
}

