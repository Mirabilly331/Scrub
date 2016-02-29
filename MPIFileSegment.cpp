#include <sstream>

#include "Logger.h"
#include "MPIFileSegment.h"

using namespace mirabella;


/**
 MPIFileSegment constructor:
 
 arguments:

**/
MPIFileSegment::MPIFileSegment(const std::string& filename,
                         const int number_of_segments, 
                         const int segment_index, 
                         const int mode,
                         const MPI_Offset size)
    : fs_filename(filename),
      fs_number_of_segments(number_of_segments),
      fs_segment_index(segment_index),
      fs_mode(mode)
{
    int mpi_rc = 0;

    if(size < 0) {
        throw std::invalid_argument("size");
    }

    Log_DEBUG_SHALLOW( (LogMessage::LogLevel::LOG_DEBUG_SHALLOW, 
                     "MPIFileSegment::MPIFileSegment entered filename[", fs_filename, 
                     "] fs_number_of_segments=<", fs_number_of_segments, ">") );
    Log_DEBUG_SHALLOW( ("MPIFileSegment::MPIFileSegment segment_index=<", segment_index, "> mode=<", fs_mode) );

    mpi_rc = MPI_File_open(MPI_COMM_SELF, 
                           fs_filename.c_str(),  
                           fs_mode,
                           MPI_INFO_NULL, 
                           &fs_handle );
    if(mpi_rc != 0) {
        throw std::invalid_argument("file name");
    }

    mpi_rc = MPI_File_get_size(fs_handle, &fs_file_size);
    if(mpi_rc != 0)
    {
        MPI_File_close(&fs_handle);
        throw std::invalid_argument("file size");
    }

    Log_DEBUG_DEEP( ("MPIFileSegment::MPIFileSegment fs_file_size=", fs_file_size, " size=", size) );
    if( (size > 0) && (fs_file_size != size) ) 
    {
        mpi_rc = MPI_File_set_size(fs_handle, size);
        if(mpi_rc != 0)
        {
            MPI_File_close(&fs_handle);
            throw std::invalid_argument("file size");
        }
        fs_file_size = size;
    }

    // Now that we have the size and we can do some fancy math to determine the 
    // begining and end of the segment we are interested in
    // Correcting to give the last segment the extra bytes lost in division

    fs_segment_length = fs_file_size / fs_number_of_segments;
    fs_segment_start = (fs_segment_index) * fs_segment_length;
    if(fs_segment_index == fs_number_of_segments - 1) {
        fs_segment_length = fs_file_size -  fs_segment_start;
    }
    fs_segment_end = fs_segment_start + fs_segment_length;
    Log_DEBUG_SHALLOW( ("MPIFileSegment::MPIFileSegment fs_segment_start=<", 
                     fs_segment_start, "> fs_segment_end=<", fs_segment_end, "> fs_segment_length=<", fs_segment_length) );
}

MPIFileSegment::~MPIFileSegment()
{
    MPI_File_close(&fs_handle);
}

/**
  MPIFileSegment::ReadBlockAt

  reads in a block of data from the segment
  returns the number of bytes read
  returns 0 on error or if read to or past the end of segment
**/
int MPIFileSegment::ReadBlockAt(const  MPI_Offset count, char* buffer, const MPI_Offset start)
{
    //std::cout << "MPIFileSegment::ReadBlockAt entered count=<" << count << "> start=<" << start << ">" << std::endl;
    return IO_switch(count, buffer, start,  MPIFileSegment_IOType::MPIFileSegment_IOType_READ);
}

int MPIFileSegment::WriteBlockAt(const  MPI_Offset count, char* buffer, const MPI_Offset start)
{
    //std::cout << "MPIFileSegment::WriteBlockAt entered count=<" << count << "> start=<" << start << ">" << std::endl;
    return IO_switch(count, buffer, start,  MPIFileSegment_IOType::MPIFileSegment_IOType_WRITE);
}

int MPIFileSegment::IO_switch(const  MPI_Offset count, char* buffer, const MPI_Offset start, MPIFileSegment_IOType type)
{
    // std::cout << "MPIFileSegment::IO_switch entered count=<" << count << "> start=<" << start << ">" << std::endl;
    MPI_Status mpi_status;
    MPI_Offset bytes_to_read = 0;

    MPI_Offset start_in_segment = fs_segment_start + start;
    if(start_in_segment >= fs_segment_end) {
        return bytes_to_read;
    }
    
    if(start_in_segment + count > fs_segment_end) {
        bytes_to_read = fs_segment_end - start_in_segment;
    }
    else {
        bytes_to_read = count;
    } 

    switch(type) {
    case MPIFileSegment_IOType::MPIFileSegment_IOType_READ:
        // std::cout << "MPIFileSegment::IO_switch before read start_in_segment=<" << start_in_segment << ">" << std::endl;
        MPI_File_read_at(fs_handle, 
                         start_in_segment, 
                         static_cast<void *>(buffer), 
                         bytes_to_read, 
                         MPI_CHAR, 
                         &mpi_status);
        break;

    case MPIFileSegment_IOType::MPIFileSegment_IOType_WRITE:
        // std::cout << "MPIFileSegment::IO_switch before write start_in_segment=<" << start_in_segment << ">" << std::endl;
        MPI_File_write_at(fs_handle, 
                          start_in_segment, 
                          static_cast<void *>(buffer), 
                          bytes_to_read, 
                          MPI_CHAR, 
                          &mpi_status);
        break;

    default:
        throw std::invalid_argument("MPIFileSegment_IOType");
    };
    

    // std::cout << "MPIFileSegment::IOSwitch  leaving bytes read=<" << bytes_to_read << ">" << std::endl;
    return bytes_to_read;
}

off_t MPIFileSegment::GetFileSize() const { return fs_file_size; }
